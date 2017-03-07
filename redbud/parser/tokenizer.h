// ============================================================================
// Copyright (c) 2017 Alinshans. All rights reserved.
// Licensed under the MIT License. See LICENSE for details.
// 
// Header File : redbud/parser/tokenizer.h 
//
// This file is used for token analysis.
// ============================================================================

#ifndef ALINSHANS_REDBUD_PARSER_TOKENIZER_H_
#define ALINSHANS_REDBUD_PARSER_TOKENIZER_H_

#include "../noncopyable.h"

namespace redbud
{
namespace parser
{

// ============================================================================
// Token class
//
// This class has many static functions that can be called directly like this:
//   std::string text = "th is is a  t e   x t.";
//   for (auto&& c : text)
//   {
//     if (!Token::blank(c)) 
//       printf("%c", c);
//   }
class Token : public noncopyable
{

  // Static functions.
 public:

  static bool blank(char ch)
  {
    return ch == ' ' || ch == '\t' || ch == '\v';
  }

  static bool space(char ch)
  {
    return ch == ' ' || ch == '\r' || ch == '\n' ||
           ch == '\f' || ch == '\t' || ch == '\v';
  }

  static bool digit(char ch)
  {
    return '0' <= ch && ch <= '9';
  }

  static bool digit1to9(char ch)
  {
    return '1' <= ch && ch <= '9';
  }

  static bool xdigit(char ch)
  {
    return ('0' <= ch && ch <= '9') ||
           ('a' <= ch && ch <= 'f') ||
           ('A' <= ch && ch <= 'F');
  }

  static bool alnum(char ch)
  {
    return '0' <= ch && ch <= '9';
  }

  static bool alpha(char ch)
  {
    return ('a' <= ch && ch <= 'z') ||
           ('A' <= ch && ch <= 'Z');
  }

  static bool lower(char ch)
  {
    return 'a' <= ch && ch <= 'z';
  }

  static bool upper(char ch)
  {
    return 'A' <= ch && ch <= 'Z';
  }

  static bool word(char ch)
  {
    return ('a' <= ch && ch <= 'z') ||
           ('A' <= ch && ch <= 'Z') ||
           (ch == '_');
  }

  static bool escape(char ch)
  {
    return ch == '\a' || ch == '\b' || ch == '\f' ||
           ch == '\n' || ch == '\r' || ch == '\t' ||
           ch == '\v' || ch == '\\' || ch == '\'' ||
           ch == '\"' || ch == '\?';
  }

  static bool printable(char ch)
  {
    return ch >= 0x20;
  }

  static int to_digit(char ch)
  {
    if ('0' <= ch && ch <= '9') return ch - '0';
    if ('a' <= ch && ch <= 'z') return ch - 'a' + 10;
    if ('A' <= ch && ch <= 'Z') return ch - 'A' + 10;
    return -1;
  }

  static char to_escape(char ch)
  {
    switch (ch)
    {
      case 'a':  return '\a';
      case 'b':  return '\b';
      case 'f':  return '\f';
      case 'n':  return '\n';
      case 'r':  return '\r';
      case 't':  return '\t';
      case 'v':  return '\v';
      case '\\': return '\\';
      case '?':  return '?';
      case '\'': return '\'';
      case '"':  return '\"';

        // We expect escape sequences to have been validated separately.
      default:   return '?';
    }
  }
};

} // namespace parser
} // namespace redbud
#endif // !ALINSHANS_REDBUD_PARSER_TOKENIZER_H_

