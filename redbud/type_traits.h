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

namespace redbud
{

template <typename T>
constexpr bool is_char_e =
std::is_same_v<char, std::decay_t<T>> ||
std::is_same_v<signed char, std::decay_t<T>> ||
std::is_same_v<unsigned char, std::decay_t<T>>;

template <typename T>
constexpr bool is_integer_e =
std::is_integral_v<T> &&
!std::is_same_v<bool, T> &&
!is_char_e<T> &&
!std::is_same_v<wchar_t, T> &&
!std::is_same_v<char16_t, T> &&
!std::is_same_v<char32_t, T>;

}

#endif // !ALINSHANS_REDBUD_TYPE_TRAITS_H_

