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

#include <cstring>     // strlen

#include <algorithm>   // for_each
#include <string>      // string, to_string
#include <limits>      // numeric_limits
#include <utility>     // forward

#include "__undef_minmax.h"
#include "type_traits.h"

namespace redbud
{

// ============================================================================
// String conversion.
//
// This part contains two templates: toString, spliceString.
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
//  char        -> string
//  char*       -> string
//  char[]      -> string
//  string      -> string
//  other       -> "[?]"
// ----------------------------------------------------------------------------

// Unknown type.
template <typename T, typename std::enable_if_t<
  !std::is_arithmetic_v<T> &&
  !std::is_constructible_v<std::string, T>, int> = 0>
std::string toString(const T& value)
{
  return "[?]";
}

// For std::string, const char*.
template <typename T, typename std::enable_if_t<
  std::is_constructible_v<std::string, T> &&
  !std::is_array_v<T>, int> = 0>
std::string toString(const T& value)
{
  return std::string{ value };
}

// For bool type.
template <typename T, typename std::enable_if_t<
  std::is_same_v<bool, T>, int> = 0>
std::string toString(T x)
{
  return x ? "true" : "false";
}

// For char type.
template <typename T, typename std::enable_if_t<
  is_char_e<T>, int> = 0>
std::string toString(T c)
{
  return std::string(1, c);
}

// For integral type.
template <typename T, typename std::enable_if_t<
  is_integer_e<T>, int> = 0>
std::string toString(T n)
{
  return std::to_string(n);
}

// For floating type.
template <typename T, typename std::enable_if_t<
  std::is_floating_point_v<T>, int> = 0>
  std::string toString(T n)
{
  char buf[22];
  if (std::is_same_v<float, T>)
  {
    std::snprintf(buf, sizeof(buf), "%.6g", n);
  }
  else
  {
    std::snprintf(buf, sizeof(buf), "%.15g", n);
  }
  return std::string{ buf };
}

// For raw array.
template <typename T, std::size_t N, typename std::enable_if_t<
  is_char_e<T>, int> = 0>
std::string toString(T (&v)[N])
{
  std::string res;
  res.reserve(N + 1);
  std::for_each(std::begin(v), std::end(v), [&res](const T& e) {
    res.append(std::string(1, e));
  });
  return res;
}

template <typename T, std::size_t N, typename std::enable_if_t<
  is_integer_e<T> || std::is_floating_point_v<T>, int> = 0>
std::string toString(T (&v)[N])
{
  std::string res;
  res.reserve(N + 1);
  std::for_each(std::begin(v), std::end(v), [&res](const T& e) {
    res.append(toString(std::forward<decltype(e)>(e)));
  });
  return res;
}

// ----------------------------------------------------------------------------
// Converts all the parameters to std::string and splices them.
// 
// The conversion of parameters to string uses redbud::toString.
//
// Example:
//   std::string str = spliceString(1, '>', 0, " is ", 1 > 0);
//   std::cout << str;  // "1>0 is true"
// ----------------------------------------------------------------------------
inline std::string spliceString()
{
  return std::string{};
}

template <typename First>
std::string spliceString(First&& str)
{
  return toString(std::forward<First>(str));
}

template <typename First, typename ...Args>
std::string spliceString(First&& str, Args&& ...args)
{
  return toString(std::forward<First>(str)) + 
         spliceString(std::forward<Args>(args)...);
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
template <typename To, typename From>
To integer_cast_safe(From value)
{
  static_assert(std::is_integral_v<To> && std::is_integral_v<From>, 
                "integer type required");
  if (std::numeric_limits<From>::digits > std::numeric_limits<To>::digits)
  { // Bits of From is larger than bits of To, or
    // From is unsigned type and To is opposite.
    if (value > static_cast<From>(std::numeric_limits<To>::max()))
    {
      return std::numeric_limits<To>::max();
    }
    if (std::is_signed_v<From> &&
        value < static_cast<From>(std::numeric_limits<To>::min()))
    {
      return std::numeric_limits<To>::min();
    }
    return static_cast<To>(value);
  }
  else if (std::is_unsigned_v<To> && value < 0)
  {
    return 0;
  }
  return static_cast<To>(value);
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
template <typename To, typename From>
To float_cast_safe(From value)
{
  static_assert(std::is_floating_point_v<To> && std::is_floating_point_v<From>,
                "float type required");
  if (std::numeric_limits<From>::digits > 
      std::numeric_limits<To>::digits)
  {
    if (value > std::numeric_limits<To>::max())
    {
      return std::numeric_limits<To>::max();
    }
    if (value < std::numeric_limits<To>::lowest())
    {
      return std::numeric_limits<To>::lowest();
    }
  }
  return static_cast<To>(value);
}

} // namespace redbud
#endif // !ALINSHANS_REDBUD_CONVERT_H_

