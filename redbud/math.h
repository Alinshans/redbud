// ============================================================================
// Copyright (c) 2017 Alinshans. All rights reserved.
// Licensed under the MIT License. See LICENSE for details.
// 
// Header File : redbud/math.h 
//
// This file contains some mathematically related functions and templates.
// ============================================================================

#ifndef ALINSHANS_REDBUD_MATH_H_
#define ALINSHANS_REDBUD_MATH_H_

#include <stdint.h>

#include <limits>

#include "__undef_minmax.h"
#include "platform.h"
#include "type_traits.h"

namespace redbud
{

#if defined(REDBUD_MSVC)
  #pragma warning(push)
  #pragma warning(disable : 4800) // forcing value to bool 'true' or 'false' 
  #pragma warning(disable : 4804) // unsafe use of type 'bool' in operation
#endif

// ============================================================================
// Finds the absolute value securely.
//
// This function template is used for arithmetic type(bool, integer type,
// floating-point type), and return the absolute value of the parameter.
// The return value is is more accurate than std::abs / std::fabs.
//
// Example:
//   int32_t int_min = -2147483648i32;
//   std::cout << std::abs(int_min);         // -2147483648
//   std::cout << redbud::safe_abs(int_min); // 2147483648
// ============================================================================

// ----------------------------------------------------------------------------
// Details.
namespace details
{

// Gets the return type of safe_abs:
//   bool          -> bool
//   float type    -> float type
//   unsigned type -> unsigned type
//   signed type   -> unsigned type
template <bool, typename T>
struct abs_return_type
{
};

template <typename T>
struct abs_return_type<true, T>
{
  typedef std::make_unsigned_t<T> type;
};

template <typename T>
struct abs_return_type<false, T>
{
  typedef T type;
};

template <typename T>
using return_t = typename abs_return_type<
  std::is_signed<T>::value &&
  !std::is_floating_point<T>::value,
  T>::type;

} // namespace details

// ----------------------------------------------------------------------------
// Function template : safe_abs

// For signed type.
template <typename T>
details::return_t<T> __safe_abs(T n, std::true_type)
{
  if (std::is_floating_point_v<T>)
  { // Floating point number.
    return n < 0.0 ? -n : n;
  }
  // Signed integer number.
  details::return_t<T> n_copy = n;
  if (n_copy > static_cast<details::return_t<T>>(
    std::numeric_limits<T>::min()))
  { // MIN < n < 0
    return -n;
  }
  return n_copy;
}

// For unsigned type.
template <typename T>
details::return_t<T> __safe_abs(T n, std::false_type)
{
  return n;
}

template <typename T>
details::return_t<T> safe_abs(T n)
{
  static_assert(std::is_arithmetic_v<T>, "arithmetic type required");
  return __safe_abs(n, std::is_signed<T>());
}

#if defined(REDBUD_MSVC)
  #pragma warning(pop)
#endif

} // namespace redbud
#endif // !ALINSHANS_REDBUD_MATH_H_

