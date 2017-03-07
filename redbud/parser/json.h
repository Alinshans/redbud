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

#include <stdint.h>

#include <map>               // map
#include <string>            // string
#include <vector>            // vector
#include <memory>            // shared_ptr
#include <utility>           // pair, move
#include <initializer_list>  // initializer_list

namespace redbud
{
namespace parser
{
namespace json
{

// ============================================================================
// Forward declaration

class JsonValue;

// ============================================================================
// Json class
// 
// This class is a JSON encoder and decoder, provides a series of interfaces
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
// Notes:
//   1. The old version of JSON specified by the obsolete RFC 4627 required
//      that the top-level value of a JSON text must be either a JSON object
//      or array, and could not be a JSON null, boolean, number, or string 
//      value. RFC 7159 removed that restriction.
//   2. Only supports UTF-8, please refer to the corresponding following 
//      instructions below for specific serialization / deserialization.
//   3. Not supports `NaN`, `Infinity` and `-Infinity` for number.
//   4. The RFC specifies that the keys within a JSON object should be unique,
//      for repeated keys, the last value will override the previous value.
//   5. The JSON object will be sorted according to the keys.
//
// Example:
//
//  // Encode / Decode
//  Json j;
//  j["project"] = "redbud";
//  j["file"] = 10;
//  j["finished"] = false;
//  j["list"] = Json::Array{ "json.h","json.cc" };
//  j["author"] = Json::Object{ {"Name","Alinshans"},{"age",18} };
//
//  Json j2 = {
//    { "null", nullptr },
//    { "bool",true },
//    { "number", 1 },
//    { "string", "str" },
//    { "array",{ 0, 1, 2 } },
//    { "object",{
//      { "key1", 3.3 },
//      { "key2", 2.3 }
//    } },
//  };
//  
//  auto b = j2["bool"].as_bool();
//  auto n = j2["number"].as_number();
//  auto s = j2["string"].as_string();
//
//  // Serialization / Deserialization
//  std::string json = "{\"num\":1,\"arr\":[],\"null\":null,"
//    "\"str\":\"hello\",\"Unicode\":\"\\u0041\",\"obj\":{},"
//    "\"level1\":{\"level2\":{\"level3\":{}}}}";
//  Json j = Json::parse(json);
// 
//  j.loads("[0,1.1,true,\"new\",[],{}]");
//  auto str = j.dumps();
//
//  // STL-like access
//  Json j;
//  j.push_back(0);
//  j.push_back(true);
//  j.push_back("haha");
//  j.push_back(Json::Object{ { "obj",true } });
//  j.erase(0);
//
//  j.type();         // Json::Type::kJsonArray
//  j.size();         // 3
//  j.empty();        // false
//  j[0].is_bool();   // true
//  j[1].as_string(); // "haha"
//  j.clear();
//
//  // Input / Output
//  Json j;
//  std::cin >> j;          // input:  {"input":"something","item":2}
//  std::cout << j << "\n"; // output: {"input":"something","item":2}
//  j.print(Json::PrintType::Pretty);
//  // output:
//  // {
//  //     "input" : "something",
//  //     "item" : 2
//  // }
class Json
{

  // --------------------------------------------------------------------------
  // Enum constants, friend class and using declarations.
 public:

  // Type of JSON value.
  // kJsonNull means the Json has a value which is null,
  // kNull means the Json is empty.
  enum class Type
  {
    kJsonNull = 0,
    kJsonBool,
    kJsonNumber,
    kJsonString,
    kJsonArray,
    kJsonObject,
    kNull
  };

  // Type of Output format.
  enum class PrintType
  {
    Compact = 0,
    Pretty
  };

  // Alias declarations.
  using Object = std::map<std::string, Json>;
  using Array  = std::vector<Json>;

  friend class JsonValue;

  // --------------------------------------------------------------------------
  // Static data member, for empty JSON value, empty JSON array and empty
  // JSON object, you can initialize a JSON value like:
  //   Json j1 = Json::null_array;
  //   Json j2 = Json::null_object;
  // Notes that, if a JSON is a null_json, it can be converted to any JSON
  // type in the next operation, e.g.:
  //   Json j;
  //   j.push_back(0);  // push_back operation just for JSON array,
  //                    // so the j become a JSON array.
  //   j["key"] = 1;    // error! j is a JSON array.
  static Json null_json;
  static Json null_array;
  static Json null_object;

  // Decodes from a string, follows the rules of RFC 7159 and ECMA-404.
  // This function does not throw an exception, it will return null_json
  // if parses failed, otherwise, it will return the result of parsing.
  //
  // Example:
  //   Json j = Json::parse("{\"key\":1}"); // Successful, j is an object.
  //   std::string err;
  //   Json j2 = Json::parse("{:1}", err);  // Failed, j is an null_json,
  //                                        // exception information is 
  //                                        // stored in err.
  static Json parse(const std::string& s);

  // --------------------------------------------------------------------------
  // Constructor / Copy constructor / Move constructor / Destructor
 public:

  // Default constructor, this Json type() is kNull, means this Json is empty,
  // which is different from a JSON has a null value.
  Json();

  // Constructs a Json with the corresponding JsonValue in C++. Does not
  // use `explicit` identifier, for convenient to use std::initializer_list.
  // The template parameter of std::initializer_list is Json, so you can
  // write the following code, each element can convert to Json implicitly.
  //   Json j = {
  //     { "key", 123 },
  //     { "array",{ nullptr, 0, 1.2, "haha" }},
  //     { "object",{{"null",true}}}
  //   };

  Json(std::nullptr_t);      // null
  Json(bool);                // bool
  Json(int32_t);             // number
  Json(uint32_t);            // number
  Json(int64_t);             // number
  Json(uint64_t);            // number
  Json(double);              // number
  Json(const char*);         // string
  Json(const std::string&);  // string
  Json(std::string&&);       // string
  Json(const Array&);        // array
  Json(Array&&);             // array
  Json(const Object&);       // object
  Json(Object&&);            // object

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

  // It can be use easily with std::initializer_list, as mentioned above.
  // There are some places where you should pay attention to. You can
  // create a Json with std::initalizer_list, and the type of this Json
  // and its member, may be an array or an object. Here are a few cases
  // (assignment operation is same as this):
  //   1. Uses braces without elements does not call this constructor but
  //      call the default constructor, so if you writes code like these:
  //      `Json j1{}`, `Json j2 = {}`, `Json j3 = {{"empty",{}}}`, a pair of
  //      braces that exist alone will call the default constructor, i.e.,
  //      it does not have a JSON value(of course not belonging to JsonNull)
  //      nor does it have any output. so if you calls j3.print(), you will
  //      see `{"empty":}`, which is not expected.
  //   2. Uses braces without elements more than one times, the innermost one
  //      will be treated as default constructor and other will be treated as
  //      a JSON array(see point 3), e.g., `Json j{{{}}}; j.print();`, you
  //      will see `[[]]`.
  //   3. The format of constructing a JSON array like this:
  //      {JsonValue[,JsonValue]}
  //      The contents of square brackets can occur zero or more times.
  //      A JSON array is composed by several JsonValue wrapped in a pair of
  //      braces, e.g., `Json j = {"this is an","array"};`, after you output
  //      it you will see `["this is an","array"]`. But if you writes:
  //      `Json j = {{"this is an","array"}}`, what you might want is an array
  //      in an array, but it does not handle it like you want(see point 4,5).
  //   4. The format of constructing a JSON object like this:
  //      {{"string",JsonValue}[,{"string",JsonValue}]}
  //      The outermost braces indicate that this is an object, some braces
  //      in the inner layor indicate that is a key-value pair, the first
  //      element is a string, the second element is a JsonValue. Here is
  //      an example for constructing an JSON object:
  //      `Json j = {{"key",1},{"empty",false},{"obj",{{"null",nullptr}}}};`
  //      If calls j.print() : {"empty":false,"key":1,"obj":{"null":null}}
  //      Notes that, it will give priority to match an object if possible,
  //      so the code like `Json j = {{"this is an","array"}}` conforms to
  //      the format of a JSON object, and it will be treated as an object.
  //   5. You can explicitly declare the type, i.e., use Json::Array or
  //      Json::Object, to specify the type of JSON. This rule will cover the
  //      preceding rules. e.g., `Json j = Json::Array{{"obj",false}};`,
  //      j is an array, it has one element which is also an array, so if you
  //      calls j.printf(), you will see: `[["obj",false]]`. That is what you
  //      want to see in the point 3.
  //
  //  The above is the rule of using braces(std::initializer_list), and they
  //  can be used in combination. The following is a complete example:
  //    Json j = {
  //      {"null",nullptr},
  //      {"bool",true},
  //      {"number",1.1},
  //      {"string","hello"},
  //      {"array",{1,2,3}},
  //      {"object",{
  //        {"key1",1},
  //        {"key2",2}
  //      }},
  //      {"not obj",Json::Array{
  //        {"array","in array"}
  //      }}
  //    };
  Json(const std::initializer_list<Json>&);
  
  // The same as the previous one.
  Json& operator=(const std::initializer_list<Json>&);
  
  // --------------------------------------------------------------------------

 public:

  // Returns one of the Json::Type.
  Type type() const;

  // True if this type is the corresponding Json::Type.
  // Notes that, is_null() means that the jsonValue is null, 
  // which is different from a null JSON.
  bool is_null()   const;
  bool is_bool()   const;
  bool is_number() const;
  bool is_string() const;
  bool is_array()  const;
  bool is_object() const;

  // Converts a JSON value to a corresponding value.
  // If the types do not match, a default value is returned.
  bool               as_bool()   const;
  double             as_number() const;
  const std::string& as_string() const;
  const Array&       as_array()  const;
  const Object&      as_object() const;

  // Gets or sets JsonValue of this Json, this type must be a JSON array,
  // otherwise, an exception will be thrown.
  Json& operator[](size_t i);
  const Json& operator[](size_t i) const;

  // Gets or sets JsonValue of this Json, this type must be a JSON object,
  // otherwise, an exception will be thrown. the keys of the JSON object
  // should be unique, so the repeated keys will be overwritten.
  Json& operator[](const std::string& key);
  const Json& operator[](const std::string& key) const;

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
  bool   empty() const;

  // Inserts a Json value, only for Json array.
  void push_back(const Json& e);

  // Inserts a key-value pair, only for Json object.
  void insert(const std::pair<std::string, Json>& p);

  // Deletes the Json value at subscript i of the Json array.
  void erase(size_t i);

  // Deletes the key-value pair of the Json object.
  void erase(const std::string& key);
  
  // After calling this function, this Json will become:
  // Json type       after clear
  // null     --->      null
  // bool     --->      false
  // number   --->       0
  // string   --->       ""
  // array    --->  empty array
  // object   --->  empty object
  void clear();

  // Serializes this JSON and saves the result in str. Notes that for
  // those characters in the range of ASCII(i.e, value from 0 to 127)
  // will be saved as ASCII code, the other will be saved as Unicode
  // (especially UTF-8), and, the hex character is in uppercase form.
  //
  // Example:
  //   Json j = Json::parse("[\"\\u0041\\u4e2d\\uD834\\uDD1E\"]");
  //   std::string json_string;
  //   j.dumps(json_string);
  //   std::cout << json_string;  // output: ["A\u4E2D\uD834\uDD1E"]
  void dumps(std::string& str) const;

  // Likes the previous one, returns a std::string as the result.
  std::string dumps() const;

  // Passes in a string, and saves the parsed result in this object.
  void loads(const std::string& str);

  // Output this Json text, the first parameter can be set to the 
  // output format(the default is PrintType::Compact), and the second
  // parameter can be set to the _indentation(the default is 4).
  // If the PrintType is Compact, the second parameter will be ignored.
  //
  // Example:
  //   Json j = Json::Array{
  //     {{ "I say","hello" }},
  //     {{ "you says","world" }},
  //     { "list",nullptr,true,1 }
  //   };
  //   j.print();  
  //   //[{"I say":"hello"},{"you says":"world"},["list",null,true,1]]
  //   j.print(Json::PrintType::Pretty, 2);
  //   //[
  //   //  {
  //   //    "I say" : "hello"
  //   //  },
  //   //  {
  //   //    "you says" : "world"
  //   //  },
  //   //  [
  //   //    "list",
  //   //    null,
  //   //    true,
  //   //    1
  //   //  ]
  //   //]
  void print(PrintType t = PrintType::Compact, size_t ind = 4) const;

  // --------------------------------------------------------------------------
  // Overloads standard input / output.
  // operator<< actually calls print(PrintType::Compact).
  // operator>> will parse the input string first, so if the input string
  // is a invalid JSON value, a null_json will get.

 public:
  friend std::ostream& operator<<(std::ostream& os, const Json& j);
  friend std::istream& operator>>(std::istream& is, Json& j);

  // --------------------------------------------------------------------------
  // Private member data and member functions.

 private:

  // The following functions are designed for serialization.
  void _dumps_from(const Json& j, std::string& str) const;
  void _dumps_string(const std::string& s, std::string& str) const;
  void _dumps_array(const Array& a, std::string& str) const;
  void _dumps_object(const Object& o, std::string& str) const;

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

