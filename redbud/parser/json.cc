// ============================================================================
// Copyright (c) 2017 Alinshans. All rights reserved.
// Licensed under the MIT License. See LICENSE for details.
// 
// Source File : redbud/parser/json.cc 
//
// This file contains the definition and implementation of JsonValue class 
// and its derived classes, and contains the implementation of Json class.
// ============================================================================

#include "json.h"

#include <cstdio>

#include <algorithm>
#include <iosfwd>

#include "json_parser.h"
#include "tokenizer.h"
#include "../exception.h"
#include "../platform.h"

namespace redbud
{
namespace parser
{
namespace json
{

#if defined(REDBUD_MSVC)
  #pragma warning(push)
  #pragma warning(disable : 6031) // return value ignored
#endif

// ============================================================================
// Macro definition.

#define EXPECT_BOOL                                     \
  REDBUD_THROW_EX_IF(type() != Json::Type::kJsonBool,   \
                  "Expecting a boolean.");

#define EXPECT_NUMBER                                   \
  REDBUD_THROW_EX_IF(type() != Json::Type::kJsonNumber, \
                  "Expecting a number.");

#define EXPECT_STRING                                   \
  REDBUD_THROW_EX_IF(type() != Json::Type::kJsonString, \
                  "Expecting a string.");

#define EXPECT_ARRAY                                    \
  REDBUD_THROW_EX_IF(type() != Json::Type::kJsonArray,  \
                  "Expecting a Json array.")

#define EXPECT_OBJECT                                   \
  REDBUD_THROW_EX_IF(type() != Json::Type::kJsonObject, \
                  "Expecting a Json object.");

// ============================================================================
// Base class : JsonValue

// anonymous namespace
namespace
{

// Distinguishs between lvalue and rvalue.
struct rvalue {};
struct lvalue {};

}

class JsonValue
{

 protected:

  friend class Json;

  // Pure virtual functions.
  virtual Json::Type  type() const = 0;
  virtual size_t      size() const = 0;
  virtual void        clear() = 0;

  // Gets Json from JsonArray.
  Json&               get_value_from_arr(size_t i);
  const Json&         get_value_from_arr(size_t i) const;

  // Gets Json from JsonObject.
  Json&               get_value_from_obj(const std::string& key, lvalue);
  const Json&         get_value_from_obj(const std::string& key, rvalue) const;

  // If the types does not match, the corresponding instance
  // will be returned.
  bool                get_bool_safe()   const;
  double              get_number_safe() const;
  const std::string&  get_string_safe() const;
  const Json::Array&  get_array_safe()  const;
  const Json::Object& get_object_safe() const;

  // STL-like access.
  template <typename T>
  void push_back(T&& e);
  void pop_back();
  void insert(const Json::ObjectValue& p);
  void insert(Json::ObjectValue&& p);
  void erase(size_t i);
  void erase(const std::string& key);

};

// ============================================================================
// Derived class, representing the specific type of JSON.

class JsonNull : public JsonValue
{

 public:

  friend class Json;
  friend class JsonValue;

  // override
  Json::Type type() const override { return Json::Type::kJsonNull; }
  size_t     size() const override { return 1; }
  void       clear()      override {}

};

class JsonBool : public JsonValue
{

 public:

  friend class Json;
  friend class JsonValue;

  JsonBool() = default;
  JsonBool(bool b) :value_(b) {}
  ~JsonBool() = default;

  // override
  Json::Type type() const override { return Json::Type::kJsonBool; }
  size_t     size() const override { return 1; }
  void       clear()      override { value_ = false; }

 private:
  bool value_;

};

class JsonNumber : public JsonValue
{

 public:

  friend class Json;
  friend class JsonValue;

  JsonNumber() = default;
  JsonNumber(int32_t n) :value_(static_cast<double>(n)) {}
  JsonNumber(uint32_t n) :value_(static_cast<double>(n)) {}
  JsonNumber(int64_t n) :value_(static_cast<double>(n)) {}
  JsonNumber(uint64_t n) :value_(static_cast<double>(n)) {}
  JsonNumber(double n) :value_(n) {}
  ~JsonNumber() = default;

  // override
  Json::Type type() const override { return Json::Type::kJsonNumber; }
  size_t     size() const override { return 1; }
  void       clear()      override { value_ = 0.0; }

 private:
  double value_;

};

class JsonString : public JsonValue
{

 public:

  friend class Json;
  friend class JsonValue;

  JsonString() = default;
  JsonString(const char* sz) :value_(sz) {}
  JsonString(const std::string& str) :value_(str) {}
  JsonString(std::string&& str) :value_(std::move(str)) {}
  ~JsonString() = default;

  // override
  Json::Type type() const override { return Json::Type::kJsonString; }
  size_t     size() const override { return 1; }
  void       clear()      override { value_.clear(); }

 private:
  std::string value_;

};

class JsonArray : public JsonValue
{

 public:

  friend class Json;
  friend class JsonValue;

  JsonArray() = default;
  JsonArray(const Json::Array& a) :value_(a) {}
  JsonArray(Json::Array&& a) :value_(std::move(a)) {}
  JsonArray(const std::initializer_list<Json>& ilist)
    :value_(ilist.begin(), ilist.end())
  {
  }
  ~JsonArray() = default;

  // override
  Json::Type type() const override { return Json::Type::kJsonArray; }
  size_t     size() const override { return value_.size(); }
  void       clear()      override { value_.clear(); }

 private:
  Json::Array value_;

};

class JsonObject : public JsonValue
{

 public:

  friend class Json;
  friend class JsonValue;
  
  JsonObject() = default;
  JsonObject(const Json::Object& o) :value_(o) {}
  JsonObject(Json::Object&& o) :value_(std::move(o)) {}
  JsonObject(const std::initializer_list<
             std::pair<std::string, Json>>& ilist)
    :value_(ilist.begin(), ilist.end())
  {
  }
  ~JsonObject() = default;

  // override
  Json::Type type() const override { return Json::Type::kJsonObject; }
  size_t     size() const override { return value_.size(); }
  void       clear() override { value_.clear(); }

 private:
  Json::Object value_;

};

// ============================================================================
// Implementation of JsonValue.

// ----------------------------------------------------------------------------
// Member functions.

Json& JsonValue::get_value_from_arr(size_t index)
{
  auto& arr = static_cast<JsonArray&>(*this).value_;
  return arr[index];
}

const Json& JsonValue::get_value_from_arr(size_t index) const
{
  const auto& arr = static_cast<const JsonArray&>(*this).value_;
  return arr[index];
}

Json& JsonValue::get_value_from_obj(const std::string& key, lvalue)
{
  auto& obj = static_cast<JsonObject&>(*this).value_;
  return obj[key];
}

const Json& JsonValue::get_value_from_obj(const std::string& key, rvalue) const
{
  const auto& obj = static_cast<const JsonObject&>(*this).value_;
  const auto& key_pos = obj.find(key);
  REDBUD_THROW_EX_IF(key_pos == obj.cend(), "Json no such key.");
  return key_pos->second;
}

bool JsonValue::get_bool_safe() const
{
  return static_cast<const JsonBool&>(*this).value_;
}

double JsonValue::get_number_safe() const
{
  return static_cast<const JsonNumber&>(*this).value_;
}

const std::string& JsonValue::get_string_safe() const
{
  return static_cast<const JsonString&>(*this).value_;
}

const Json::Array& JsonValue::get_array_safe() const
{
  return static_cast<const JsonArray&>(*this).value_;
}

const Json::Object& JsonValue::get_object_safe() const
{
  return static_cast<const JsonObject&>(*this).value_;
}

template <typename T>
void JsonValue::push_back(T&& e)
{
  auto& arr = static_cast<JsonArray&>(*this).value_;
  arr.push_back(std::forward<T>(e));
}

void JsonValue::pop_back()
{
  auto& arr = static_cast<JsonArray&>(*this).value_;
  arr.pop_back();
}

void JsonValue::insert(const Json::ObjectValue& p)
{
  auto& obj = static_cast<JsonObject&>(*this).value_;
  obj[p.first] = p.second;
}

void JsonValue::insert(Json::ObjectValue&& p)
{
  auto& obj = static_cast<JsonObject&>(*this).value_;
  obj[p.first] = std::move(p.second);
}

void JsonValue::erase(size_t i)
{
  auto& arr = static_cast<JsonArray&>(*this).value_;
  if (arr.size() <= i)
  {
    return;
  }
  arr.erase(arr.begin() + i);
}

void JsonValue::erase(const std::string& key)
{
  auto& obj = static_cast<JsonObject&>(*this).value_;
  obj.erase(key);
}

// ============================================================================
// Implementation of Json class.

// ----------------------------------------------------------------------------
// Static functions.

Json Json::parse(const std::string& json_text)
{
  return JsonParser::parse(json_text);
}

Json Json::parse(std::string&& json_text)
{
  return JsonParser::parse(std::move(json_text));
}

// ----------------------------------------------------------------------------
// Constructor / Copy constructor / Move constructor / Destructor

Json::Json()
{
}

Json::Json(std::nullptr_t)
  :node_(std::make_shared<JsonNull>())
{
}

Json::Json(bool b)
  :node_(std::make_shared<JsonBool>(b))
{
}

Json::Json(int32_t n)
  :node_(std::make_shared<JsonNumber>(n))
{
}

Json::Json(uint32_t n)
  :node_(std::make_shared<JsonNumber>(n))
{
}

Json::Json(int64_t n)
  :node_(std::make_shared<JsonNumber>(n))
{
}

Json::Json(uint64_t n)
  :node_(std::make_shared<JsonNumber>(n))
{
}

Json::Json(double d)
  :node_(std::make_shared<JsonNumber>(d))
{
}

Json::Json(const char* sz)
  :node_(std::make_shared<JsonString>(sz))
{
}

Json::Json(const std::string& str)
  :node_(std::make_shared<JsonString>(str))
{
}

Json::Json(std::string&& str)
  :node_(std::make_shared<JsonString>(std::move(str)))
{
}

Json::Json(const Array& a)
  :node_(std::make_shared<JsonArray>(a))
{
}

Json::Json(Array&& a)
  :node_(std::make_shared<JsonArray>(std::move(a)))
{
}

Json::Json(const Object& o)
  :node_(std::make_shared<JsonObject>(o))
{
}

Json::Json(Object&& o)
  :node_(std::make_shared<JsonObject>(std::move(o)))
{
}

Json::Json(const Json& j)
  :node_(j.node_)
{
}

Json::Json(Json&& j)
  :node_(std::move(j.node_))
{
}

// ----------------------------------------------------------------------------
// Copy assignment operator / Move assignment operator

Json& Json::operator=(const Json& j)
{
  node_ = j.node_;
  return *this;
}

Json& Json::operator=(Json&& j)
{
  node_ = std::move(j.node_);
  return *this;
}

// ----------------------------------------------------------------------------
// initializer_list

Json::Json(std::initializer_list<Json> ilist)
{
  bool maybe_object =
    std::all_of(ilist.begin(), ilist.end(), [&ilist](const Json& v)
  {
    return v.is_array() && v.size() == 2 && v[0].is_string();
  });

  if (maybe_object)
  {
    node_ = std::make_shared<JsonObject>();
    std::for_each(ilist.begin(), ilist.end(), [this](const Json& v)
    {
      insert({ v[0].as_string(),v[1] });
    });
  }
  else // not maybe_object
  {
    node_ = std::make_shared<JsonArray>(ilist);
  }
}

Json& Json::operator=(std::initializer_list<Json> ilist)
{
  bool maybe_object =
    std::all_of(ilist.begin(), ilist.end(), [&ilist](const Json& v)
  {
    return v.is_array() && v.size() == 2 && v[0].is_string();
  });

  if (maybe_object)
  {
    node_ = std::make_shared<JsonObject>();
    std::for_each(ilist.begin(), ilist.end(), [this](const Json& v)
    {
      insert({ v[0].as_string(),v[1] });
    });
  }
  else // not maybe_object
  {
    node_ = std::make_shared<JsonArray>(ilist);
  }
  return *this;
}

// ----------------------------------------------------------------------------
// Type interface.

Json::Type Json::type() const
{
  if (node_.use_count() == 0)
  {
    return Type::kNull;
  }
  return node_.get()->type();
}

bool Json::is_null()   const { return type() == Type::kJsonNull; }
bool Json::is_bool()   const { return type() == Type::kJsonBool; }
bool Json::is_number() const { return type() == Type::kJsonNumber; }
bool Json::is_string() const { return type() == Type::kJsonString; }
bool Json::is_array()  const { return type() == Type::kJsonArray; }
bool Json::is_object() const { return type() == Type::kJsonObject; }

// ----------------------------------------------------------------------------
// Data interface.

bool Json::as_bool() const
{
  EXPECT_BOOL;
  return node_.get()->get_bool_safe();
}

double Json::as_number() const
{
  EXPECT_NUMBER;
  return node_.get()->get_number_safe();
}

const std::string& Json::as_string() const
{
  EXPECT_STRING;
  return node_.get()->get_string_safe();
}

const Json::Array& Json::as_array() const
{
  EXPECT_ARRAY;
  return node_.get()->get_array_safe();
}

const Json::Object& Json::as_object() const
{
  EXPECT_OBJECT;
  return node_.get()->get_object_safe();
}

// ----------------------------------------------------------------------------
// Accesses / modifies data via operator[].

Json& Json::operator[](size_t index)
{
  EXPECT_ARRAY;
  REDBUD_THROW_EX_IF(size() <= index, "Json index out of range.");
  return node_.get()->get_value_from_arr(index);
}

const Json& Json::operator[](size_t index) const
{
  EXPECT_ARRAY;
  REDBUD_THROW_EX_IF(size() <= index, "Json index out of range.");
  return node_.get()->get_value_from_arr(index);
}

#if JSON_OBJECT_USE_PROXY

Json::JsonProxy Json::operator[](const std::string& key)
{
  if (type() == Type::kNull)
  {
    *this = Object{};
  }
  EXPECT_OBJECT;
  return JsonProxy(*this, key);
}

const Json::JsonProxy Json::operator[](const std::string& key) const
{
  EXPECT_OBJECT;
  return JsonProxy(const_cast<Json&>(*this), key);
}

#else

Json& Json::operator[](const std::string& key)
{
  if (type() == Type::kNull)
  {
    *this = Object{};
    return node_.get()->get_value_from_obj(key, lvalue{});
  }
  EXPECT_OBJECT;
  return node_.get()->get_value_from_obj(key, lvalue{});
}

const Json& Json::operator[](const std::string& key) const
{
  EXPECT_OBJECT;
  return node_.get()->get_value_from_obj(key, rvalue{});
}

#endif // JSON_OBJECT_USE_PROXY

// ----------------------------------------------------------------------------
// STL-like access.

size_t Json::size() const
{
  if (type() == Type::kNull)
  {
    return 0;
  }
  return node_.get()->size();
}

bool Json::empty() const
{
  return size() == 0;
}

void Json::push_back(const ArrayValue& element)
{
  if (type() == Type::kNull)
  {
    *this = Array{};
    node_.get()->push_back(element);
    return;
  }
  EXPECT_ARRAY;
  node_.get()->push_back(element);
}

void Json::push_back(ArrayValue&& element)
{
  if (type() == Type::kNull)
  {
    *this = Array{};
    node_.get()->push_back(std::move(element));
    return;
  }
  EXPECT_ARRAY;
  node_.get()->push_back(std::move(element));
}

void Json::pop_back()
{
  EXPECT_ARRAY;
  REDBUD_THROW_EX_IF(size() == 0, "Json has no value before pop.");
  node_.get()->pop_back();
}

void Json::insert(const ObjectValue& pair)
{
  if (type() == Type::kNull)
  {
    *this = Object{};
    node_.get()->insert(pair);
    return;
  }
  EXPECT_OBJECT;
  node_.get()->insert(pair);
}

void Json::insert(ObjectValue&& pair)
{
  if (type() == Type::kNull)
  {
    *this = Object{};
    node_.get()->insert(std::move(pair));
    return;
  }
  EXPECT_OBJECT;
  node_.get()->insert(std::move(pair));
}

void Json::erase(size_t i)
{
  EXPECT_ARRAY;
  node_.get()->erase(i);
}

void Json::erase(const std::string& key)
{
  EXPECT_OBJECT;
  node_.get()->erase(key);
}

void Json::clear()
{
  if (type() == Type::kJsonNull || type() == Type::kNull)
  {
    return;
  }
  node_.get()->clear();
}

// ----------------------------------------------------------------------------
// Serialization / Deserialization

void Json::dumps(std::string& str) const
{
  str.clear();
  str.reserve(size() << 6);
  _dumps_from(*this, str);
}

std::string Json::dumps() const
{
  std::string str;
  dumps(str);
  return str;
}

void Json::loads(const std::string & str)
{
  *this = JsonParser::parse(str);
}

void Json::loads(std::string&& str)
{
  *this = JsonParser::parse(std::move(str));
}

// ----------------------------------------------------------------------------
// Output.

void Json::print(PrintType t, size_t ind) const
{
  switch (type())
  {
    case Type::kJsonNull:
      std::printf("null");
      break;
    case Type::kJsonBool:
      std::printf("%s", as_bool() ? "true" : "false");
      break;
    case Type::kJsonNumber:
      std::printf("%.17g", as_number());
      break;
    case Type::kJsonString:
      std::printf("%s", as_string().c_str());
      break;
    case Type::kJsonArray:
      _print_array(t, ind, 0);
      break;
    case Type::kJsonObject:
      _print_object(t, ind, 0);
      break;
    
    // This Json has no value.
    default:
      break;
  }
}

// ============================================================================
// Helper functions.

// Serialization.

void Json::_dumps_from(const Json& j, std::string& str) const
{
  switch (j.type())
  {
    case Type::kJsonNull:
      str.append("null");
      break;
    case Type::kJsonBool:
      str.append(j.as_bool() ? "true" : "false");
      break;
    case Type::kJsonNumber:
      char buf[22]; // (-) + (17 digits) + (.) + (e) + (+)
      std::snprintf(buf, sizeof(buf), "%.17g", j.as_number());
      str.append(buf);
      break;
    case Type::kJsonString:
      _dumps_string(j.as_string(), str);
      break;
    case Type::kJsonArray:
      _dumps_array(j.as_array(), str);
      break;
    case Type::kJsonObject:
      _dumps_object(j.as_object(), str);
      break;
    
    // This Json has no value.
    default:
      break;
  }
}

#define GETC(ch)       static_cast<uint8_t>(ch)
#define PUTC(ch)       str.push_back(static_cast<char>(ch))
#define CODE(bin, pre) (static_cast<uint8_t>(bin) & pre)

void Json::_dumps_string(const std::string& s, std::string& str) const
{
  PUTC('\"');
  for (size_t i = 0; i < s.size(); ++i)
  {
    /*if (Token::escape(s[i]))
    {
      PUTC(s[i]);
    }
    else*/ if (GETC(s[i]) <= 0x7F)
    { // One byte : 0xxxxxxx  
      // Preserves ASCII characters.
      PUTC(s[i]);
    }
    else if (GETC(s[i]) <= 0xDF)
    { // Two bytes : 110xxxxx  10xxxxxx
      uint32_t u = (CODE(s[i], 0x1F) << 6) |
                   (CODE(s[i + 1], 0x3F));
      char buf[7];
      std::snprintf(buf, sizeof(buf), "\\u%04X", u);
      str.append(buf);
      i += 1;
    }
    else if (GETC(s[i]) <= 0xEF)
    { // Three bytes : 1110xxxx  10xxxxxx  10xxxxxx
      uint32_t u = (CODE(s[i], 0xF) << 12) |
                   (CODE(s[i + 1], 0x3F) << 6) |
                   (CODE(s[i + 2], 0x3F));
      char buf[7];
      std::snprintf(buf, sizeof(buf), "\\u%04X", u);
      str.append(buf);
      i += 2;
    }
    else if (GETC(s[i]) <= 0xF7)
    { // Four bytes : 11110xxx 	10xxxxxx 	10xxxxxx 	10xxxxxx
      // Deal with surrogate pair.
      uint32_t u = (CODE(s[i], 0x7) << 18) |
                   (CODE(s[i + 1], 0x3F) << 12) |
                   (CODE(s[i + 2], 0x3F) << 6) |
                   (CODE(s[i + 3], 0x3F));
      u -= 0x10000;
      char buf[7];
      std::snprintf(buf, sizeof(buf), "\\u%04X", (u >> 10) + 0xD800);
      str.append(buf);
      std::snprintf(buf, sizeof(buf), "\\u%04X", (u & 0x3FF) + 0xDC00);
      str.append(buf);
      i += 3;
    }
  }
  PUTC('\"');
}

void Json::_dumps_array(const Array& a, std::string& str) const
{
  bool begin = true;
  PUTC('[');
  for (const auto& j : a)
  {
    if (!begin)
    {
      PUTC(',');
    }
    _dumps_from(j, str);
    begin = false;
  }
  PUTC(']');
}

void Json::_dumps_object(const Object& o, std::string& str) const
{
  bool begin = true;
  PUTC('{');
  for (const auto& p : o)
  {
    if (!begin)
    {
      PUTC(',');
    }
    _dumps_string(p.first, str);
    PUTC(':');
    _dumps_from(p.second, str);
    begin = false;
  }
  PUTC('}');
}

#undef GETC
#undef PUTC
#undef CODE

// Output.

void Json::_print_array(PrintType t, size_t ind, size_t dep) const
{
  if (size() == 0)
  {
    std::printf("[]");
    return;
  }
  std::printf("[");
  auto last = as_array().cend();
  --last;
  for (auto it = as_array().cbegin(); it != as_array().cend(); ++it)
  {
    _indentation(t, ind, dep + 1);
    switch (it->type())
    {
      case Type::kJsonNull:
        std::printf("null");
        break;
      case Type::kJsonBool:
        std::printf("%s", it->as_bool() ? "true" : "false");
        break;
      case Type::kJsonNumber:
        std::printf("%.17g", it->as_number());
        break;
      case Type::kJsonString:
        std::printf("\"%s\"", it->as_string().c_str());
        break;
      case Type::kJsonArray:
        it->_print_array(t, ind, dep + 1);
        break;
      case Type::kJsonObject:
        it->_print_object(t, ind, dep + 1);
        break;
    }
    if (it != last && it->type() != Type::kNull)
    {
      std::printf(",");
    }
  }
  _indentation(t, ind, dep);
  std::printf("]");
}

void Json::_print_object(PrintType t, size_t ind, size_t dep) const
{
  if (size() == 0)
  {
    std::printf("{}");
    return;
  }
  std::printf("{");
  auto last = as_object().cend();
  --last;
  for (auto it = as_object().cbegin(); it != as_object().cend(); ++it)
  {
    _indentation(t, ind, dep + 1);
    t == PrintType::Compact ? std::printf("\"%s\":", it->first.c_str())
      : std::printf("\"%s\" : ", it->first.c_str());
    switch (it->second.type())
    {
      case Type::kJsonNull:
        std::printf("null");
        break;
      case Type::kJsonBool:
        std::printf("%s", it->second.as_bool() ? "true" : "false");
        break;
      case Type::kJsonNumber:
        std::printf("%.17g", it->second.as_number());
        break;
      case Type::kJsonString:
        std::printf("\"%s\"", it->second.as_string().c_str());
        break;
      case Type::kJsonArray:
        it->second._print_array(t, ind, dep + 1);
        break;
      case Type::kJsonObject:
        it->second._print_object(t, ind, dep + 1);
        break;
    }
    if (it != last)
    {
      std::printf(",");
    }
  }
  _indentation(t, ind, dep);
  std::printf("}");
}

// Controls indentation.
void Json::_indentation(PrintType t, size_t ind, size_t dep) const
{
  // Only PrintType::Pretty has indentation.
  if (t == PrintType::Pretty)
  {
    std::printf("\n");
    for (size_t i = 0; i < ind * dep; ++i)
    {
      std::printf(" ");
    }
  }
}

// ============================================================================
// Overloads standard input / output stream.

std::ostream& operator<<(std::ostream& os, const Json& j)
{
  j.print(Json::PrintType::Compact);
  return os;
}

std::istream& operator >> (std::istream& is, Json& j)
{
  std::string json_buf;
  std::getline(is, json_buf, '\n');
  j = Json::parse(json_buf);
  return is;
}

#if JSON_OBJECT_USE_PROXY

// ============================================================================
// Proxy class.

Json::JsonProxy::JsonProxy(Json& n, const std::string& key)
  :j_(n), key_(key)
{
}

Json::JsonProxy& Json::JsonProxy::operator=(const Json& rhs)
{
  j_.node_.get()->get_value_from_obj(key_, lvalue{}) = rhs;
  return *this;
}

Json::JsonProxy& Json::JsonProxy::operator=(Json&& rhs)
{
  j_.node_.get()->get_value_from_obj(key_, lvalue{}) = rhs;
  return *this;
}

Json::JsonProxy::operator Json() const
{
  return j_.node_.get()->get_value_from_obj(key_, rvalue{});
}

Json::JsonProxy Json::JsonProxy::operator[](const std::string& key)
{
  return j_.node_.get()->get_value_from_obj(key_, lvalue{})[key];
}

Json* Json::JsonProxy::operator&()
{
  return &(j_.node_.get()->get_value_from_obj(key_, lvalue{}));
}

const Json* Json::JsonProxy::operator&() const
{
  return &(j_.node_.get()->get_value_from_obj(key_, rvalue{}));
}

#endif // JSON_OBJECT_USE_PROXY


#undef EXPECT_BOOL
#undef EXPECT_NUMBER
#undef EXPECT_STRING
#undef EXPECT_ARRAY
#undef EXPECT_OBJECT

#if defined(REDBUD_MSVC)
  #pragma warning(pop)
#endif

} // namespace json
} // namespace parser
} // namespace redbud

