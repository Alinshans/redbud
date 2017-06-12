// ============================================================================
// Copyright (c) 2017 Alinshans. All rights reserved.
// Licensed under the MIT License. See LICENSE for details.
//
// Header File : redbud/parser/json.h
//
// This file is used to deal with JSON (JavaScript Object Notation), specified
// by RFC 7159 (which obsoletes RFC 4627) and by ECMA-404, including JSON
// encoder and decoder. see more on https://tools.ietf.org/html/rfc7159 and
// http://www.ecma-international.org/publications/files/ECMA-ST/ECMA-404.pdf .
// ============================================================================

#ifndef ALINSHANS_REDBUD_PARSER_JSON_H_
#define ALINSHANS_REDBUD_PARSER_JSON_H_

#include <cstdint>

#include <algorithm>
#include <map>               // map
#include <string>            // string
#include <vector>            // vector
#include <memory>            // shared_ptr
#include <utility>           // pair, move, forward
#include <initializer_list>  // initializer_list
#include <type_traits>

#include "../platform.h"

namespace redbud
{
namespace parser
{
namespace json
{

using std::size_t;
using std::int32_t;
using std::uint32_t;
using std::int64_t;
using std::uint64_t;

// ============================================================================
// Forward declaration

class JsonValue;

// ============================================================================
// Json class
//
// This class is a JSON encoder/decoder, provides a series of interfaces
// to parse, generate, modify and output a JSON. The type conversion table for
// JSON and C++ is as follow:
// -------------------------
// [  JSON  ]    [   C++   ]
// -------------------------
// | object |    |   map   |
// | array  |    | vector  |
// | string |    | string  |
// | number |    | double  |
// |  true  |    |  true   |
// | false  |    |  false  |
// |  null  |    | nullptr |
// -------------------------
// 
// For more information please read this documents:
// https://github.com/Alinshans/redbud/blob/master/document/parser/json.md
class Json
{

  // --------------------------------------------------------------------------
  // Enum constants, friend class and using declarations.
 public:

  // Type of JSON value.
  enum class Type
  {
    kJsonNull   = 0,
    kJsonBool   = 1,
    kJsonNumber = 2,
    kJsonString = 3,
    kJsonArray  = 4,
    kJsonObject = 5
  };

  // Type of Output format.
  enum class PrintType
  {
    Compact = 0,
    Pretty = 1
  };

  // Alias declarations.
  using string_t       = std::string;
  using object_t       = std::map<string_t, Json>;
  using array_t        = std::vector<Json>;
  using array_value_t  = array_t::value_type;
  using object_value_t = object_t::value_type;

  friend class JsonValue;

  // --------------------------------------------------------------------------
  // Static functions.
 public:

  // Decodes from a string, follows the rules of RFC 7159 and ECMA-404.
  // if parses failed, it will yield an exception.

  static Json parse(const string_t& json);
  static Json parse(string_t&& json);

  // Serializes any object that can be converted to Json to Json.

  template <typename T, typename std::enable_if_t<
    std::is_constructible_v<Json, T>, int> = 0>
  static Json to_json(T&& value) { return value; }

  // for native array
  template <typename T, size_t N1>
  static Json to_json(T(&v)[N1])
  {
    static_assert(std::is_constructible_v<Json, T>, 
                  "the type can not be converted to Json");
    Json json = array_t{};
    std::for_each(std::begin(v), std::end(v), [&json](const T& value) {
      json.push_back(value);
    });
    return json;
  }

  template <typename T, size_t N1, size_t N2>
  static Json to_json(T(&v)[N1][N2])
  {
    static_assert(std::is_constructible_v<Json, T>,
                  "the type can not be converted to Json");
    Json json = array_t{};
    std::for_each(std::begin(v), std::end(v), [&json](T(&v2)[N2]) {
      json.push_back(to_json(v2));
    });
    return json;
  }

  template <typename T, size_t N1, size_t N2, size_t N3>
  static Json to_json(T(&v)[N1][N2][N3])
  {
    static_assert(std::is_constructible_v<Json, T>,
                  "the type can not be converted to Json");
    Json json = array_t{};
    std::for_each(std::begin(v), std::end(v), [&json](T(&v2)[N2][N3]) {
      json.push_back(to_json(v2));
    });
    return json;
  }

  template <typename T, size_t N1, size_t N2, size_t N3, size_t N4>
  static Json to_json(T(&v)[N1][N2][N3][N4])
  {
    static_assert(std::is_constructible_v<Json, T>,
                  "the type can not be converted to Json");
    Json json = array_t{};
    std::for_each(std::begin(v), std::end(v), [&json](T(&v2)[N2][N3][N4]) {
      json.push_back(to_json(v2));
    });
    return json;
  }

  template <typename T, size_t N1, size_t N2, size_t N3, size_t N4, size_t N5>
  static Json to_json(T(&v)[N1][N2][N3][N4][N5])
  {
    static_assert(std::is_constructible_v<Json, T>,
                  "the type can not be converted to Json");
    Json json = array_t{};
    std::for_each(std::begin(v), std::end(v), [&json](T(&v2)[N2][N3][N4][N5]) {
      json.push_back(to_json(v2));
    });
    return json;
  }

  // for initializer_list
  static Json to_json(std::initializer_list<Json> ilist);

  // --------------------------------------------------------------------------
  // Constructor / Copy constructor / Move constructor / Destructor
 public:

  // Default constructor, makes this Json have a null value.
  Json();

  // Constructs a Json with the corresponding JsonValue in C++.

  Json(std::nullptr_t);   // null
  Json(bool);             // bool
  Json(int32_t);          // number
  Json(uint32_t);         // number
  Json(int64_t);          // number
  Json(uint64_t);         // number
  Json(double);           // number
  Json(char*);            // string
  Json(const char*);      // string
  Json(const string_t&);  // string
  Json(string_t&&);       // string
  Json(const array_t&);   // array
  Json(array_t&&);        // array
  Json(const object_t&);  // object
  Json(object_t&&);       // object

  // Constructs form object-like container like std::map, std::unordered_map.
  template <typename M, typename std::enable_if_t<
    std::is_constructible_v<Json, typename M::key_type>
    && std::is_constructible_v<Json, typename M::mapped_type>, int> = 0>
  Json(const M& value) :Json(object_t(value.begin(), value.end())) {}

  // Constructs form array-like container like std::vector, std::list.
  template <typename A, typename std::enable_if_t<
    std::is_constructible_v<Json, typename A::value_type>, int> = 0>
  Json(const A& value) :Json(array_t(value.begin(), value.end())) {}

  Json(const Json&);
  Json(Json&&);

  // Deletes all constructors with a raw pointer. Because the JsonValue
  // is stored in a std::shared_ptr, and passes a raw pointer to a
  // std::shared_ptr constructor is no a good idea.
  template <typename T>
  Json(T*) = delete;

  ~Json() = default;

  // --------------------------------------------------------------------------
  // Copy assignment operator / Move assignment operator

  Json& operator=(const Json&);
  Json& operator=(Json&&);

  // --------------------------------------------------------------------------
  // initializer_list

  // Some places should be noted and please see:
  // https://github.com/Alinshans/redbud/blob/master/document/parser/json.md#initializer_list
  
  Json(std::initializer_list<Json> ilist);
  Json& operator=(std::initializer_list<Json> ilist);

  // --------------------------------------------------------------------------

 public:

  // Returns one of the Json::Type.
  Type type() const;

  // True if this type is the corresponding Json::Type.
  bool is_null()   const;
  bool is_bool()   const;
  bool is_number() const;
  bool is_string() const;
  bool is_array()  const;
  bool is_object() const;

  // Converts a JSON value to a corresponding value.
  // If the types do not match, it will yield an exception.
  bool            as_bool()   const;
  int32_t         as_int32()  const;
  uint32_t        as_uint32() const;
  int64_t         as_int64()  const;
  uint64_t        as_uint64() const;
  double          as_double() const;
  const string_t& as_string() const;
  const array_t&  as_array()  const;
  const object_t& as_object() const;

  // Gets or sets JsonValue of this Json, this type must be a JSON array,
  // otherwise, an exception will be thrown.
  Json&       operator[](size_t index);
  const Json& operator[](size_t index) const;

  // Gets or sets JsonValue of this Json, this type must be a JSON object,
  // otherwise, an exception will be thrown. the keys of the JSON object
  // should be unique, so the repeated keys will be overwritten.
  //
  // Some places should be noted and please see:
  // https://github.com/Alinshans/redbud/blob/master/document/parser/json.md#operator-with-a-jsonobject
  Json&       operator[](const string_t& key);
  const Json& operator[](const string_t& key) const;

  // Return value correspondence table:
  // Json type      return value
  // null     --->        0
  // bool     --->        1
  // number   --->        1
  // string   --->        1
  // array    --->  size of array
  // object   --->  size of object
  size_t size() const;

  // True if size() == 0.
  bool empty() const;

  // True if the Json has the key, only for JSON object.
  bool has_key(const string_t& key) const;

  // Inserts a Json value, only for JSON array.
  void push_back(const array_value_t& element);
  void push_back(array_value_t&& element);

  // Pops up the last JsonValue in this Json, only for JSON array.
  void pop_back();

  // Inserts a key-value pair, only for JSON object.
  void insert(const object_value_t& pair);
  void insert(object_value_t&& pair);

  // Deletes the Json value at subscript index of the JSON array.
  void erase(size_t index);

  // Deletes the key-value pair of the JSON object.
  void erase(const string_t& key);

  // Resets this Json to null.
  void clear();

  // Merges two Json into one, the other Json will be set to null
  // after this operation.
  //
  // Some places should be noted and please see:
  // https://github.com/Alinshans/redbud/blob/master/document/parser/json.md#merge-rule
  Json& merge(Json& other);
  Json& merge(Json&& other);

  // Serializes this JSON and saves the result in str.
  void dumps(string_t& str) const;

  // Likes the previous one, returns a string as the result.
  string_t dumps() const;

  // Passes in a string, and saves the parsed result in this Json.
  void loads(const string_t& str);
  void loads(string_t&& str);

  // Output this Json text, the first parameter can be set to the
  // output format(the default is PrintType::Compact), and the second
  // parameter can be set to the _indentation(the default is 4).
  // If the PrintType is Compact, the second parameter will be ignored.
  //
  // Some places should be noted and please see:
  // https://github.com/Alinshans/redbud/blob/master/document/parser/json.md#output-format
  void print(PrintType t = PrintType::Compact, size_t ind = 4) const;

 public:
  // --------------------------------------------------------------------------
  // Overloads standard input / output.
  // operator<< actually calls print(PrintType::Compact).
  // operator>> will parse the input string first, so if the input string
  // is a invalid JSON value, it will yield an exception.

  friend std::ostream& operator<<(std::ostream& os, const Json& j);
  friend std::istream& operator>>(std::istream& is, Json& j);

  // Overloads comparation operator.

  friend bool operator==(const Json& lhs, const Json& rhs);
  friend bool operator!=(const Json& lhs, const Json& rhs);

  // --------------------------------------------------------------------------
  // Private member data and member functions.

 private:

  // Helper functions for merge.
  Json& _merge_array(Json&& other);
  Json& _merge_object(Json&& other);

  // The following functions are designed for serialization.
  void _dumps_from(const Json& j, string_t& str) const;
  void _dumps_string(const string_t& s, string_t& str) const;
  void _dumps_array(const array_t& a, string_t& str) const;
  void _dumps_object(const object_t& o, string_t& str) const;

  // The following three functions are designed for output.
  void _print_array(PrintType t, size_t ind, size_t dep) const;
  void _print_object(PrintType t, size_t ind, size_t dep) const;
  void _indentation(PrintType t, size_t ind, size_t dep) const;

  // JsonValue node.
  std::shared_ptr<JsonValue> node_;

};

} // namespace json
} // namespace parser
} // namespace redbud
#endif // !ALINSHANS_REDBUD_PARSER_JSON_H_
