// ============================================================================
// Copyright (c) 2017 Alinshans. All rights reserved.
// Licensed under the MIT License. See LICENSE for details.
// 
// Source File : redbud/parser/json_parser.cc
//
// This file contains the implementation of JsonParser class.
// ============================================================================

#include "json_parser.h"

#include <cstdlib>  // strtod
#include <cerrno>   // errno, ERANGE

#include "tokenizer.h"
#include "../exception.h"

namespace redbud
{
namespace parser
{
namespace json
{

// ----------------------------------------------------------------------------
// Static function.

Json JsonParser::parse(const std::string& s)
{
  JsonParser jp(s);
  return jp.parse_json();
}

Json JsonParser::parse(std::string&& s)
{
  JsonParser jp(std::move(s));
  return jp.parse_json();
}

// ----------------------------------------------------------------------------
// Copy constructor.

JsonParser::JsonParser(const std::string& s)
  :r(s)
{
}

JsonParser::JsonParser(std::string&& s)
  :r(std::move(s))
{
}

// ----------------------------------------------------------------------------
// Parses process.

Json JsonParser::parse_json()
{
  r.skipspace();
  switch (r.now())
  {
    case 'n': return parse_literal("null", nullptr);
    case 't': return parse_literal("true", true);
    case 'f': return parse_literal("false", false);
    case '\"': return parse_string();
    case '[': return parse_array();
    case '{': return parse_object();
    case '\0':
      REDBUD_THROW_PEX_IF(r.now() == '\0', "Valid end of JSON.", "", r.getp());

    default:
      return parse_number();
  }
}

Json JsonParser::parse_literal(const char* s, Json&& j)
{
  r.skipspace();
  r.expect(s);
  return j;
}

Json JsonParser::parse_number()
{
#define EXP_AND_SKIP_NUM                      \
  REDBUD_THROW_PEX_IF(!Token::digit(r.now()), \
                   "digits 0 - 9",            \
                   std::to_string(r.now()),   \
                   r.getp());                 \
  do { r.to(1); } while (Token::digit(r.now()))

  r.skipspace();
  size_t p = r.getp();
  r.skip('-');
  if (r.now() == '0')
  {
    r.to(1);
  }
  else
  {
    REDBUD_THROW_PEX_IF(!Token::digit(r.now()),
                        "Valid JSON value.",
                        std::to_string(r.now()),
                        r.getp());
    do { r.to(1); } while (Token::digit(r.now()));
  }
  if (r.match('.'))
  {
    EXP_AND_SKIP_NUM;
  }
  if (r.now() == 'e' || r.now() == 'E')
  {
    r.to(1);
    if (r.now() == '+' || r.now() == '-')
    {
      r.to(1);
    }
    EXP_AND_SKIP_NUM;
  }
  errno = 0;
  double d = strtod(r.getsub(p, r.getp() - p).c_str(), nullptr);
  REDBUD_THROW_PEX_IF(errno == ERANGE, "Valid numbers", std::to_string(d), p);
  return d;

#undef EXP_AND_SKIP_NUM
}

std::string JsonParser::parse_string()
{
#define PUTC(ch)                        \
  str.push_back(static_cast<char>(ch)); \
  r.to(1);                              \
  continue

  r.skipspace();
  r.expect('\"');
  if (r.match('\"'))
  {
    return{};
  }

  std::string str;
  while (!r.eof())
  {
    if (r.now() == '\"')
    {  // End of string.
      r.to(1);
      return str;
    }
    else if (r.now() == '\\')
    {  // Escaped characters.
      r.to(1);
      switch (r.now())
      {
        case '\"': PUTC('\"');
        case '\\': PUTC('\\');
        case '/':  PUTC('/');
        case 'b':  PUTC('\b');
        case 'f':  PUTC('\f');
        case 'n':  PUTC('\n');
        case 'r':  PUTC('\r');
        case 't':  PUTC('\t');
        case 'u':
          r.to(-1); // Parses `\uXXXX`.
          str += parse_utf8();
          continue;
        default:    // Parses fail.
          size_t p = r.getp();
          bool InvalidEscapedCharacters = true;
          REDBUD_THROW_PEX_IF(InvalidEscapedCharacters,
                              "Valid escaped characters.",
                              r.getsub(p - 1, 2),
                              p);
      }
    }
    else
    { // Other characters.
      PUTC(r.now());
    }
  }
  REDBUD_THROW_PEX_IF(r.eof(),
                      "'\"' at the end of the JSON string",
                      "",
                      r.getp());
  return{};  // Ignores the warning.

#undef PUTC
}

Json JsonParser::parse_array()
{
  r.skipspace();
  r.expect('[');
  r.skipspace();
  Json::Array arr;
  if (r.match(']'))
  {
    return arr;
  }

  while (!r.eof())
  {
    arr.push_back(parse_json());
    r.skipspace();
    if (r.now() == ']')
    {
      r.to(1);
      return arr;
    }
    else if (r.now() == ',')
    {
      r.to(1);
      continue;
    }
    else
    {
      REDBUD_THROW_PEX_IF(r.now() != ',' && r.now() != ']',
                          " ',' or ']'",
                          std::string(1, r.now()),
                          r.getp());
    }
  }
  REDBUD_THROW_PEX_IF(r.eof(),
                      " ']' at end of the JSON array.",
                      "",
                      r.getp());
  return{};  // Ignores the warning.
}

Json JsonParser::parse_object()
{
  r.skipspace();
  r.expect('{');
  r.skipspace();
  Json::Object obj;
  if (r.match('}'))
  {
    return obj;
  }

  while (!r.eof())
  {
    auto key = parse_string();
    r.skipspace();
    r.expect(':');
    obj[key] = parse_json();
    r.skipspace();
    if (r.now() == '}')
    {
      r.to(1);
      return obj;
    }
    else if (r.now() == ',')
    {
      r.to(1);
      continue;
    }
    else
    {
      REDBUD_THROW_PEX_IF(r.now() != ',' && r.now() != '}',
                          "',' or '}'",
                          std::to_string(r.now()),
                          r.getp());
    }
  }
  REDBUD_THROW_PEX_IF(r.eof(),
                      " '}' at end of the JSON object.",
                      "",
                      r.getp());
  return{};  // Ignores the warning.
}

void JsonParser::parse_hex4(uint32_t& u)
{
  size_t p = r.getp();
  REDBUD_THROW_PEX_IF(!r.match("\\u"), "\\uXXXX", r.getsub(p, 6), p);
  for (int i = 0; i < 4; ++i, r.to(1))
  {
    REDBUD_THROW_PEX_IF(!Token::xdigit(r.now()), "\\uXXXX", r.getsub(p, 6), p);
    u <<= 4;
    u |= Token::to_digit(r.now());
  }
}

std::string JsonParser::parse_utf8()
{
#define PUTC(ch) str.push_back(static_cast<char>(ch))

  uint32_t u = 0;
  uint32_t u2 = 0;
  size_t p = r.getp();
  parse_hex4(u);
  if (u >= 0xD800 && u <= 0xDBFF)  // surrogate pair
  {
    parse_hex4(u2);
    REDBUD_THROW_PEX_IF(u2 < 0xDC00 || u2 > 0xDFFF,
                        "low surrogate range from U+DC00 to U+DFFF",
                        r.getsub(p + 6, 6),
                        p + 6);
    u = (((u - 0xD800) << 10) | (u2 - 0xDC00)) + 0x10000;
  }

  std::string str;
  if (u <= 0x7F)
  {
    PUTC(u & 0xFF);
  }
  else if (u <= 0x7FF)
  {
    PUTC(0xC0 | ((u >> 6) & 0xFF));
    PUTC(0x80 | (u & 0x3F));
  }
  else if (u <= 0xFFFF)
  {
    PUTC(0xE0 | ((u >> 12) & 0xFF));
    PUTC(0x80 | ((u >> 6) & 0x3F));
    PUTC(0x80 | (u & 0x3F));
  }
  else
  {
    REDBUD_THROW_PEX_IF(u > 0x10FFFF,
                        "Valid UTF-8 encode range.",
                        r.getsub(p, 12),
                        p);
    PUTC(0xF0 | ((u >> 18) & 0xFF));
    PUTC(0x80 | ((u >> 12) & 0x3F));
    PUTC(0x80 | ((u >> 6) & 0x3F));
    PUTC(0x80 | (u & 0x3F));
  }
  return str;

#undef PUTC
}


} // namespace json
} // namespace parser
} // namespace redbud
