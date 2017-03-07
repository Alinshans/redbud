// ============================================================================
// Copyright (c) 2017 Alinshans. All rights reserved.
// Licensed under the MIT License. See LICENSE for details.
// 
// Header File : redbud/parser/reader.h 
//
// This file contains a Reader class which is used to read a text.
// ============================================================================

#ifndef ALINSHANS_REDBUD_PARSER_READER_H_
#define ALINSHANS_REDBUD_PARSER_READER_H_

#include <iosfwd>      // ifstream
#include <string>      // string
#include <functional>  // function

namespace redbud
{
namespace parser
{

// ============================================================================
// Reader class
// 
// A container for placing text, provides some common operation.
class Reader
{

  // --------------------------------------------------------------------------
  // Constructor / Copy constructor / Move constructor / Destructor
 public:

  Reader() = default;

  Reader(const char* sz);
  Reader(const std::string& str);
  Reader(std::string&& str);
  Reader(std::ifstream* file);

  ~Reader() = default;

  // --------------------------------------------------------------------------
  // Getter.
 public:

  // Gets the location where you are reading now.
  size_t getp() const;

  // Gets the whole string you reads.
  const std::string& gets() const;

  // Gets the substring whose length is n from the subscript i.
  std::string getsub(size_t i, size_t n) const;

  // Gets the currently read character.
  char now() const;

  // Gets the next read character
  char next() const;

  // True if the currently read position has reached the end.
  bool eof() const;

  // --------------------------------------------------------------------------
  // Setter.

  // Advances n(the default is 1) characters, of course, the n 
  // can be negative for the backwards.
  // Example:
  //   Reader r("hello");
  //   r.now();  // 'h'
  //   r.to();
  //   r.now();  // 'e'
  //   r.to(3);
  //   r.now();  // 'o'
  //   r.to(-3);
  //   r.now();  // 'e'
  void to(int32_t n = 1);

  // Skip the whitespace.
  void skipspace();

  // If the character of current position is that you want to skip,
  // it will be skipped.
  void skip(char ch);

  // Likes the previous one, the difference is skipping a string.
  void skip(const char* sz);

  // Matchs a rule(a character, a string, even a function object),
  // if matchs successfully, it will advance the corresponding distance
  // and return true, otherwise it will not advance and return false.
  bool match(char ch);
  bool match(const std::string& str);
  bool match(std::function<bool(char)> f);

  // Likes match, the difference is that the rule must be met, if not,
  // a exception will be throw.
  bool expect(char ch);
  bool expect(const std::string& str);
  bool expect(std::function<bool(char)> f);

  // --------------------------------------------------------------------------
 private:
  std::string context_;  // The text to be read.
  size_t      p_;        // The current read position.

};

} // namespace parser
} // namespace redbud
#endif // !ALINSHANS_REDBUD_PARSER_READER_H_
