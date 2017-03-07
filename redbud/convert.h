// ============================================================================
// Copyright (c) 2017 Alinshans. All rights reserved.
// Licensed under the MIT License. See LICENSE for details.
// 
// Header File : redbud/convert.h 
//
// This file contains some functions and templates of type conversion.
// ============================================================================

#ifndef ALINSHANS_REDBUD_CONVERT_H_
#define ALINSHANS_REDBUD_CONVERT_H_

#include <stdint.h>

#include <cstring> // strlen
#include <string>  // string, to_string
#include <limits>  // numeric_limits

namespace redbud
{

// ============================================================================
// String conversion.
//
// This part contains two templates: ToString, SpliceString.
// ============================================================================

// ----------------------------------------------------------------------------
// Converts a type to a std::string.
// 
// The rules for converting a type to a string are as follows:
//  [Type]      -> [std::string]
//  bool        -> "true" or "false"
//  integral    -> string numbers
//  float       -> maximum  6-bit precision string numbers
//  double      -> maximum 15-bit precision string numbers
//  long double -> maximum 15-bit precision string numbers
//  char        -> 1-bit string
//  char*       -> string
//  char[]      -> string
//  string      -> string
//  other       -> "[?]"
// ----------------------------------------------------------------------------

// Unknown type.
template <typename T>
inline std::string ToString(T value)
{
  return "[?]";
}

// For std::string.
template <>
inline std::string ToString(const std::string& s)
{
  return s;
}

// For const char*.
template <>
inline std::string ToString(const char* s)
{
  return std::string(s);
}

// For char[].
template <>
inline std::string ToString(char s[])
{
  return std::string(s, s + strlen(s));
}

// For char / signed char / unsigned char.
template <>
inline std::string ToString(char c)
{
  return std::string(1, c);
}
template <>
inline std::string ToString(signed char c)
{
  return std::string(1, c);
}
template <>
inline std::string ToString(unsigned char c)
{
  return std::string(1, c);
}

// For bool type.
template <>
inline std::string ToString(bool x)
{
  return x ? std::string("true") : std::string("false");
}

// For integral type.
template <>
inline std::string ToString(int32_t n)
{
  return std::to_string(n);
}
template <>
inline std::string ToString(uint32_t n)
{
  return std::to_string(n);
}
template <>
inline std::string ToString(int64_t n)
{
  return std::to_string(n);
}
template <>
inline std::string ToString(uint64_t n)
{
  return std::to_string(n);
}

// For float / double
template <>
inline std::string ToString(float x)
{
  return std::to_string(x);
}
template <>
inline std::string ToString(double x)
{
  return std::to_string(x);
}

// ----------------------------------------------------------------------------
// Converts all the parameters to std::string and splices them.
// 
// The conversion of parameters to string uses redbud::ToString.
//
// Example:
//   std::string str = SpliceString(1, '>', 0, " is ", 1 > 0);
//   std::cout << str;  // "1>0 is true"
// ----------------------------------------------------------------------------
inline std::string SpliceString()
{
  return "";
}

template <typename Arg1>
inline std::string SpliceString(Arg1 str)
{
  return ToString(str);
}

template <typename Arg, typename ...Args>
std::string SpliceString(Arg str, Args ...args)
{
  return ToString(str) + SpliceString(args...);
}

// ============================================================================
// Safe numerical conversion.
//
// This part contains two safe cast templates: integer_cast_safe and
// float_cast_safe. Both of them can conversion securely, which means they
// will find the most suitable conversion.
// ============================================================================

// ----------------------------------------------------------------------------
// Converts integer numbers securely.
//
// Example:
//   auto a1 = static_cast<int32_t>(4000000000);       // a1 = -294967296
//   auto a2 = integer_cast_safe<int32_t>(4000000000); // a2 = 2147483647
//   auto a3 = static_cast<uint32_t>(-1);              // a3 = 4294967295
//   auto a4 = integer_cast_safe<uint32_t>(-1);        // a4 = 0
// ----------------------------------------------------------------------------
template <typename Target, typename Origin>
Target integer_cast_safe(Origin value)
{
  static_assert(std::numeric_limits<Origin>::is_integer &&
                std::numeric_limits<Target>::is_integer, 
                "Integer type required");
  if (std::numeric_limits<Origin>::digits > 
      std::numeric_limits<Target>::digits)
  { // Bits of Origin is larger than bits of Target, or
    // Origin is unsigned type and Target is opposite.
    if (value > static_cast<Origin>(std::numeric_limits<Target>::max()))
    {
      return std::numeric_limits<Target>::max();
    }
    if (std::numeric_limits<Origin>::is_signed &&
        value < static_cast<Origin>(std::numeric_limits<Target>::min()))
    {
      return std::numeric_limits<Target>::min();
    }
    return static_cast<Target>(value);
  }
  else if (!std::numeric_limits<Target>::is_signed && value < 0)
  {
    return 0;
  }
  return static_cast<Target>(value);
}

// ----------------------------------------------------------------------------
// Converts floating point numbers securely.
//
// Example:
//   auto a1 = static_cast<float>(5.20e+99);      // a1 = inf
//   auto a2 = float_cast_safe<float>(5.20e+99);  // a2 = 3.40282e+38
//   auto a3 = static_cast<float>(-5.20e+99);     // a3 = -inf
//   auto a4 = float_cast_safe<float>(-5.20e+99); // a4 = -3.40282e+38
// ----------------------------------------------------------------------------
template <typename Target, typename Origin>
Target float_cast_safe(Origin value)
{
  static_assert(std::numeric_limits<Origin>::is_specialized &&
                std::numeric_limits<Target>::is_specialized &&
                !std::numeric_limits<Origin>::is_integer &&
                !std::numeric_limits<Target>::is_integer,
                "Float type required");
  if (std::numeric_limits<Origin>::digits > 
      std::numeric_limits<Target>::digits)
  {
    if (value > std::numeric_limits<Target>::max())
    {
      return std::numeric_limits<Target>::max();
    }
    if (value < std::numeric_limits<Target>::lowest())
    {
      return std::numeric_limits<Target>::lowest();
    }
  }
  return static_cast<Target>(value);
}

} // namespace redbud
#endif // !ALINSHANS_REDBUD_CONVERT_H_

