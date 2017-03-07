// ============================================================================
// Copyright (c) 2017 Alinshans. All rights reserved.
// Licensed under the MIT License. See LICENSE for details.
// 
// Header File : redbud/exception.h 
//
// This file contains some exception classes which are used to handle and
// describe the exception automatically.
// ============================================================================

#ifndef ALINSHANS_REDBUD_EXCEPTION_H_
#define ALINSHANS_REDBUD_EXCEPTION_H_

#include <string>

namespace redbud
{

// ============================================================================
// Exception class
//
// Common exception class, has 4 parameters, the first parameter indicates
// an exception condition, the second parameter indicates an Prompt message.
class Exception
{

public:

  Exception() = default;

  Exception(const std::string& cond,
            const std::string& msg,
            const char* file,
            size_t line);

};

// ============================================================================
// ParseException class
//
// Yeilds an exception when parses something wrong, has 6 parameters, the
// first parameter indicates the exception condition, the second parameter
// indicates something that is expected, the third parameter indicates
// the actual things, and the fourth parameter indicates the position where
// the error occurred.
class ParseException : public Exception
{

 public:

  ParseException() = default;

  ParseException(const std::string& cond,
                 const std::string& exp,
                 const std::string& act,
                 size_t pos,
                 const char* file,
                 size_t line);

};

// Simplify operations with macros.
// 
// Example:
//   REDBUD_THROW_EX_IF(n == 0, "n con not be 0");
// If n is equal to 0, the exception will be throw and the program 
// will carsh after describing the exception message.
#define REDBUD_THROW_EX_IF(condition, message)      \
  do {                                              \
    if (condition) {                                \
      throw Exception(#condition,                   \
                      message, __FILE__, __LINE__); \
    }                                               \
  } while(0)

#define REDBUD_THROW_PEX_IF(cond, exp, act, pos)    \
  do {                                              \
    if (cond) {                                     \
      throw ParseException(#cond, exp, act, pos,    \
                           __FILE__, __LINE__);     \
    }                                               \
  } while(0)


} // namespace redbud
#endif // !ALINSHANS_REDBUD_EXCEPTION_H_

