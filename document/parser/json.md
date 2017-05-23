# redbud::parser::json::Json

***

## Catalog

* [Introduction](#introduction)
* [Conversion table](#conversion-table)
* [Getting start](#getting-start)
  * [Simple example](#simple-example) 
  * [Parse](#parse)
  * [Encode / Decode](#encode--decode)
  * [Serialization / Deserialization](#serialization--deserialization)
  * [STL-like access](#stl-like-access)
  * [Input / Output](#input--output)
* [Notes](#notes)
  * [initializer_list](#initializer_list)
  * [`operator[]` with a `JsonObject`](#operator-with-a-jsonobject)
  * [Output format](#output-format)
  * [Other](#other)

## Introduction

The `Json` class is a convenient tool to encode/decode for JSON, it provides a series of interfaces to parse, generate, modify and output a JSON value. This class is compliance with [RFC 7159](https://tools.ietf.org/html/rfc7159) (which obsoletes RFC 4627) and [ECMA-404](http://www.ecma-international.org/publications/files/ECMA-ST/ECMA-404.pdf).

## Conversion table

The type conversion table for JSON and C++ is as follow:

|JSON | C++ |
|:---:|:---:|
| null | nullptr |
| true | true |
| false | false |
| number | int32_t/uint32_t/int64_t/uint64_t/double |
| string | string |
| array | vector |
| object | map |

## Getting start

### Simple example

There are a simple example for using `Json`:

```c++
#include <iostream>

#include "redbud/parser/json.h"

using namespace redbud::parser::json;

int main()
{
  Json json = Json::parse("{\"name\":\"alinshans\",\"year\":20}");
  std::cout << json;

  json = {
    {"username","alinshans" },
    {"password","123456"},
    {"birthday",{
      {"year",2000},
      {"month",1},
      {"day",1}
    }},
    {"yes",true},
    {"null",nullptr}
  };

  auto name = json["username"].as_string();
  auto day = json["birthday"]["day"].as_int32();
  json.print();
}
```

### Parse

Use the following static function to parse a Json from a string:
```c++
  std::string jsonstr = "{\"name\":\"alinshans\",\"year\":20}";
  Json json = Json::parse(jsonstr);
```

### Encode / Decode

You can encode easily with the `initializer_list`:
```c++
Json j = {
    { "null", nullptr },
    { "bool",true },
    { "number", 1 },
    { "string", "str" },
    { "array",{ 0, 1, 2 } },
    { "object",{
      { "key1", 3.3 },
      { "key2", 2.3 }
    } },
  };
```
or in this way:
```c++
  Json j;
  j["project"] = "redbud";
  j["file"] = 10;
  j["finished"] = false;
  j["list"] = Json::array_t{ "json.h","json.cc" };
  j["author"] = Json::object_t{ {"Name","Alinshans"},{"age",18} };
```
and you can get the value of the Json with `as_xxx`:
```c++
  auto b = j2["bool"].as_bool();
  auto n = j2["number"].as_int32();
  auto s = j2["string"].as_string();
```

### Serialization / Deserialization

to string / from string
```c++
  std::string json = "{\"num\":1,\"arr\":[],\"null\":null,"
    "\"str\":\"hello\",\"Unicode\":\"\\u0041\",\"obj\":{},"
    "\"level1\":{\"level2\":{\"level3\":{}}}}";
  Json j = Json::parse(json);

  j.loads("[0,1.1,true,\"new\",[],{}]");
  auto str = j.dumps();
```

### STL-like access
```c++
  Json j;
  j.push_back(0);
  j.push_back(true);
  j.push_back("haha");
  j.push_back(Json::object_t{ { "obj",true } });
  j.erase(0);

  j.type();         // Json::Type::kJsonArray
  j.size();         // 3
  j.empty();        // false
  j[0].is_bool();   // true
  j[1].as_string(); // "haha"
  j.clear();
```

### Input / Output
```c++
  Json j;
  std::cin >> j;          // input:  {"input":"something","item":2}
  std::cout << j << "\n"; // output: {"input":"something","item":2}
  j.print(Json::PrintType::Pretty);
  // output:
  // {
  //     "input" : "something",
  //     "item" : 2
  // }
```

## Notes

There are some places in this class to note:

* The old version of JSON specified by the obsolete RFC 4627 required that the top-level value of a JSON text must be either a JSON object or array, and could not be a JSON null, boolean, number, or string value. **RFC 7159 removed that restriction**.
* **Only supports UTF-8 for unicode**, please refer to the corresponding following instructions below for specific serialization / deserialization.
* Not supports `NaN`, `Infinity` and `-Infinity` for number.
* The RFC specifies that the keys within a JSON object should be unique, for repeated keys, the last value will override the previous value.
* The JSON object will be sorted according to the keys.

### initializer_list

It can be use easily with `std::initializer_list`, as mentioned above. There are some places where you should pay attention to. You can create a Json with `std::initalizer_list`, and the type of this Json and its member, may be an array or an object. It is worth noting that the Json will be a `Json object` as much as possible. Here are a few cases:

1. Uses braces without elements does not call the `std::initializer_list` constructor but call the default constructor, so if you writes code like these:
  ```c++
    Json j1{};
    Json j2 = {}; 
    Json j3 = {{"null",{}}};
  ``` 
  a pair of braces that exist alone will call the default constructor, i.e., it will have a null value.
  ```c++
    std::cout << j1;  // null
    std::cout << j2;  // null
    std::cout << j3;  // {"null":null}
  ```

2. Uses braces without elements more than one times, the innermost one
        will be treated as default constructor and other will be treated as
        a JSON array(see point 3), e.g.:
 ```c++
   Json j{{{}}}; 
   std::cout << j;  // [[null]]
 ```

3. Things go different when you write a code like this:
  ```c++
    Json j({});
    std::cout << j;  // {}
  ```
  the braces in here will really call the `std::initializer_list` constructor and luckily, it may be an object so it really becomes an object. So the grammar of `Json j({})` is to construct a Json with another Json which is an object, so the result is what you see.

4. The format of constructing a JSON array like this:
  ```
    {JsonValue[,JsonValue]}
  ```
  The contents of square brackets can occur zero or more times. A JSON array is composed by several JsonValue wrapped in a pair of braces, e.g.:
  ```c++
    Json j = {"this is an","array"};
    std::cout << j; // ["this is an","array"]
  ```
  But if you writes:
  ```c++
    Json j = {{"this is an","array"}};
  ```
  what you might want is an array in an array, but it does not handle it like you want(see point 5,6).

5. The format of constructing a JSON object like this:
  ```
    {{"string",JsonValue}[,{"string",JsonValue}]}
  ```
  The outermost braces indicate that this is an object, some braces in the inner layor indicate that is a key-value pair, the first element is a string, the second element is a JsonValue. Here is an example for constructing a JSON object:
  ```c++
    Json j = {{"key",1},{"empty",false},{"obj",{{"null",nullptr}}}};
    std::cout << j; // {"empty":false,"key":1,"obj":{"null":null}}
  ```
  Notes that, it will give priority to match an object if possible, so the code like 
  ```c++
    Json j = {{"this is an","array"}}
  ```
  conforms to the format of a JSON object, and it will be treated as an object.

6. You can explicitly declare the type, i.e., use `Json::array_t` or `Json::object_t`, to specify the type of JSON. This rule will cover the preceding rules. e.g.:
  ```c++
    Json j = Json::array_t{{"this is an","array"}};
  ```
  `j` will be an array, and it has one element which is also an array, so if you
  output it you will see: `[["this is an","array"]]`, That is what you want to see in the point 4.
  

The above is the rule of using braces(std::initializer_list), and they can be used in combination. The following is a complete example:
```c++
  Json j = {
    { "null",nullptr },
    { "bool",true },
    { "number",1.1 },
    { "string","hello" },
    { "array",{ 1,2,3 } },
    { "object",{
      { "key1",1 },
      { "key2",2 }
    } },
    { "not obj",Json::array_t{
      { "array","in array" }
  } }
  };
```

### `operator[]` with a `JsonObject`

When you use the `operator[]` on a `JsonObject`, you should note the following situation:
```c++
  Json j = {
    { "x",1 },
  };
  std::cout << j["y"];  // null
  std::cout << j;       // {"x":1,"y":null}
```
If the key is not existed, use `operator[]` will insert a key-value pair to the object(std::map), so the `j` will be changed. If you do not want that, please make a judgment before using it:
```c++
  Json j = {
    { "x",1 },
  };
  if (j.has_key("y"))
  {
    // do something
  }
  else
  {
    // or not do
  }
  std::cout << j;  // {"x":1}
```

### Output format

Uses the `print` function for a output which you like, e.g.:
```c++
  Json j = Json::array_t{
    { { "I say","hello" } },
    { { "you says","world" } },
    { "list",nullptr,true,1 }
  };
  j.print();
```
you will see:
```
[{"I say":"hello"},{"you says":"world"},["list",null,true,1]]
```
you can specify the first argument for format-output:
```c++
  j.print(Json::PrintType::Pretty);
```
you will see:
```
[
    {
        "I say" : "hello"
    },
    {
        "you says" : "world"
    },
    [
        "list",
        null
        true,
        1
    ]
]
```
and you can specify the second argument for indentation:
```c++
  j.print(Json::PrintType::Pretty, 2);
```
you will see:
```
[
  {
    "I say" : "hello"
  },
  {
    "you says" : "world"
  },
  [
    "list",
    null
    true,
    1
  ]
]
```

### Other

When you use the function which is not applicable to all types, such as `as_xxx()`, `push_back()`, `insert()` and so on, make sure the type is correct, otherwise, it will yield an exception.


