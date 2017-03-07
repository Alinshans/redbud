// ============================================================================
// Copyright (c) 2017 Alinshans. All rights reserved.
// Licensed under the MIT License. See LICENSE for details.
// 
// Header File : redbud/parser/json_parser.h
//
// This file contains a JsonParser class, which is used to parse a JSON text.
// ============================================================================

#ifndef ALINSHANS_REDBUD_PARSER_JSON_PARSER_H_
#define ALINSHANS_REDBUD_PARSER_JSON_PARSER_H_

#include "json.h"
#include "reader.h"

namespace redbud
{
namespace parser
{
namespace json
{

// ============================================================================
// Json Parser class
class JsonParser
{

  // --------------------------------------------------------------------------
  // Static function.
 public:
  static Json parse(const std::string& s);

  // --------------------------------------------------------------------------
  // Copy constructor.
 public:
  JsonParser(const std::string& s);
  JsonParser(std::string&& s);

  // --------------------------------------------------------------------------
  // Helper functions.
 private:

  // Parses the corresponding JSON type.
  Json        parse_json();
  Json        parse_literal(const char* s, const Json& j);
  Json        parse_number();
  std::string parse_string();
  Json        parse_array();
  Json        parse_object();

  // Parses unicode.
  void        parse_hex4(uint32_t& u);
  std::string parse_utf8();

  // --------------------------------------------------------------------------
  // Private member data.
 private:
  Reader r;
};

} // namespace json
} // namespace parser
} // namespace redbud
#endif // !ALINSHANS_REDBUD_PARSER_JSON_PARSER_H_

