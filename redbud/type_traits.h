// ============================================================================
// Copyright (c) 2017 Alinshans. All rights reserved.
// Licensed under the MIT License. See LICENSE for details.
// 
// Header File : redbud/type_traits.h 
//
// This file is used to extract type's traits.
// ============================================================================

#ifndef ALINSHANS_REDBUD_TYPE_TRAITS_H_
#define ALINSHANS_REDBUD_TYPE_TRAITS_H_

#include <type_traits>

#include "platform.h"

// ============================================================================
// For CXX11 to the latest standard.

namespace std
{

#if !REDBUD_HAS_CXX14

#define TEMPLATE_T_V(op)                             \
  template <typename T>                              \
  constexpr bool op##_v = ::std::op<T>::value

#define TEMPLATE_T_T(op)                      \
  template <typename T>                       \
  using op##_t = typename ::std::op<T>::type

#define TEMPLATE_T_U_V(op)             \
  template <typename T, typename U>    \
  constexpr bool op##_v = ::std::op<T, U>::value 

#define TEMPLATE_T_ARGS_V(op)             \
  template <typename T, typename... Args> \
  constexpr bool op##_v =          \
  ::std::op<T, Args...>::value

#define TEMPLATE_BOOL_VOID_T(op)               \
  template <bool B, typename T = void>         \
  using op##_t = typename ::std::op<B, T>::type

// Primary type categories
TEMPLATE_T_V(is_integral);
TEMPLATE_T_V(is_floating_point);

// Composite type categories
TEMPLATE_T_V(is_arithmetic);

// Type properties
TEMPLATE_T_V(is_signed);
TEMPLATE_T_V(is_unsigned);

// Supported operations
TEMPLATE_T_ARGS_V(is_constructible);
TEMPLATE_T_ARGS_V(is_trivially_constructible);
TEMPLATE_T_ARGS_V(is_nothrow_constructible);

// Type relationships
TEMPLATE_T_U_V(is_same);
TEMPLATE_T_U_V(is_base_of);
TEMPLATE_T_U_V(is_convertible);

// Type modifications
TEMPLATE_T_T(make_signed);
TEMPLATE_T_T(make_unsigned);

// Miscellaneous transformations
TEMPLATE_BOOL_VOID_T(enable_if);

#undef TEMPLATE_T_V
#undef TEMPLATE_T_T
#undef TEMPLATE_T_U_V
#undef TEMPLATE_T_ARGS_V
#undef TEMPLATE_BOOL_VOID_T

#endif // !REDBUD_HAS_CXX14

}

namespace redbud
{

template <typename T>
constexpr bool is_real_integer =
std::is_integral_v<T> &&
!std::is_same_v<bool, T> &&
!std::is_same_v<char, T> &&
!std::is_same_v<char16_t, T> &&
!std::is_same_v<char32_t, T> &&
!std::is_same_v<wchar_t, T>;

}

#endif // !ALINSHANS_REDBUD_TYPE_TRAITS_H_

