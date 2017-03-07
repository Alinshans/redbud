// ============================================================================
// Copyright (c) 2017 Alinshans. All rights reserved.
// Licensed under the MIT License. See LICENSE for details.
// 
// Header File : redbud/bignumber.h 
//
// This file contains the definition of BigInteger class.
// ============================================================================

#ifndef ALINSHANS_REDBUD_BIGNUMBER_H_
#define ALINSHANS_REDBUD_BIGNUMBER_H_

#include <stdint.h>

#include <vector>      // vector 
#include <iosfwd>      // istream, ostream, printf
#include <string>      // string
#include <utility>     // move
#include <limits>      // numeric_limits
#include <type_traits> // enable_if, is_same

#include "math.h"
#include "exception.h"

namespace redbud
{

#if defined(REDBUD_MSVC)
  #pragma warning(push)
  #pragma warning(disable : 4804) // unsafe use of type 'bool' in operation
#endif

// Avoid name conflicts.
namespace redbud_bignumber
{

// ============================================================================
// Enum constants definition.
// ----------------------------------------------------------------------------

typedef enum kSymbol
{
  kPositive = 0,
  kNegative = 1
}SymbolType;

typedef enum kShift
{
  kMoveRight = 0,
  kMoveLeft  = 1
}ShiftType;

typedef enum kNumber
{
  kZero               = 0,
  kPositiveInteger    = 1,
  kScientificNotation = 2
}NumberType;

// ============================================================================
// BigInteger class
//
// BigInteger Provides convenient operations for large integer, the BigInteger
// can be expressed in the range (-10^4294967292, 10^4294967292). These
// operations are not very efficient, so the BigInteger should not be used
// when the requirements are more stringent.
//
// The default constructor is not provided, so it must be explicitly
// constructed like:
//   BigInteger not_init; // error
//   BigInteger b_int1(0);
//   BigInteger b_int2("1234567890");
//   BigInteger b_int3(b_int1);
//   BigInteger b_int4(std::move(b_int2));
// or use the assignment operator:
//   BigInteger b_int1 = 0;
//   BigInteger b_int2 = "1234567890";
//   BigInteger b_int3 = b_int1;
//   BigInteger b_int4 = std::move(b_int2);
//
// BigInteger supports the commonly used arithmetic operations, 
// bit operations, comparison operations with build-in integer type
// and BigInteger like:
//   +, -, *, /, %, +=, -=, *=, /=, %=, ++, --, <<, >>, <<=, >>=, 
//   ==, !=, <, >, <= ,>=, and standard I/O streams.
//
// Example:
//   BigInteger b(0);
//   std::cin >> b;                  // input : 999999999999
//   b += BigInteger("1111111111");
//   std::cout << b;                 // 1001111111110
//   if (b.is_positive())
//     b.reverse();
//   std::cout<< b / 10000;          // -100111111
class BigInteger
{

  // --------------------------------------------------------------------------
  // Type definition.
 public:

  typedef BigInteger                     self;
  typedef std::vector<int16_t>           value_type;
  typedef typename value_type::size_type size_type;
  typedef std::string                    string_type;

  // --------------------------------------------------------------------------
  // Constructor / Copy constructor / Move constructor / Destructor
 public:

  // The default constructor is not provided.
  BigInteger() = delete;

  // Constructs with an integer(not including boolean type, character type),
  // rejects other implicit conversions. This design is to prevent unnecessary
  // conversion like `bool` or `char` converts to BigInteger, unless you
  // really need that, then use explicit cast:
  //   char ch = 'A';
  //   BigInteger b(static_cast<int>(ch));
  template <typename T, typename = typename std::enable_if<
    std::is_integral<T>::value ||
    !std::is_same<bool, T>::value &&
    !std::is_same<char, T>::value &&
    !std::is_same<char16_t, T>::value &&
    !std::is_same<char32_t, T>::value &&
    !std::is_same<wchar_t, T>::value, T>::type>
  BigInteger(T n);

  // Constructs with a string.
  BigInteger(const char* s);

  BigInteger(const BigInteger& other);
  BigInteger(BigInteger&& other);

  ~BigInteger() = default;

  // --------------------------------------------------------------------------
  // Copy assignment operator / Move assignment operator

  BigInteger& operator=(const char* s);

  BigInteger& operator=(const BigInteger& other);
  BigInteger& operator=(BigInteger&& other);

  // --------------------------------------------------------------------------
  // Deletes other pointer-like constructor and assignment operator.

  BigInteger(std::nullptr_t) = delete;
  BigInteger& operator=(std::nullptr_t) = delete;

  template <typename T>
  BigInteger(T*) = delete;
  template <typename T>
  BigInteger& operator=(T*) = delete;

  // --------------------------------------------------------------------------
  // Element access.
 public:

  bool is_positive() const;

  bool is_negative() const;

  bool is_zero()     const;

  bool is_odd()      const;

  bool is_even()     const;

  // Compare to another BigInteger, return 1 if greater than another
  // one, return -1 if less than another one. Otherwise returns 0.
  // 
  // If you need to use the comparsion results twice or more, it is
  // better to use a variable to save the comparison results.
  int16_t compare(const self& other) const;

  // Returns the number of decimal digits of this BigInteger.
  size_t digits() const;

  // Returns the maximum number of decimal digits.
  size_t max_digits() const;

  // Returns the opposite number, does not modify itself.
  // e.g. BigInteger(123).opposite will returns BigInteger(-123).
  BigInteger opposite() const;

  // Returns the absolute value, does not modify itself.
  // e.g. BigInteger(-123).absolute will returns BigInteger(123).
  BigInteger absolute() const;

  // Returns the nth power of this BigInteger. Notes that
  // the exponent must be positive when the base is zero,
  // otherwise, an exception will be thrown.
  BigInteger power(const BigInteger& n) const;

  // Returns a string of this number. If this BigInteger is negative,
  // there will be a negative sign, otherwise there will not be.
  // e.g. BigInteger(-123).to_string will returns std::string("-123").
  std::string to_string() const;

  // Converts this BigInteger to an integer if possible, and returns
  // a pair, the second argument will be false if this BigInteger
  // exceeds the range that can be represented by the type to convert.
  // Notes that, you can only convert to the type that can be constructed.
  //
  // Example:
  //   BigInteger b(2147483647);
  //   auto p = b.to_integer<int32_t>();
  //   if (p.second) {
  //     std::cout << "The number is " << p.first << "\n";
  //   }
  //   else {
  //     std::cout << "The number can not convert.\n";
  //   }
  template <typename T>
  std::pair<T, bool> to_integer() const;

  // Of course, this function is for faster output efficiency.
  // The larger the BigInteger, the more calls, the better
  // the performance than std::ostream.
  // 
  // And for convenience, you can you can pass in a character 
  // argument as a separator like:
  //   BigInteger b("123");
  //   for (int i = 0; i < 3; ++i)
  //     b.print(' ');              // 123 123 123 
  //   for (int i = 0; i < 3; ++i)
  //     b.print('\n');             // 123
  //                                // 123
  //                                // 123
  void print(char separator = '\0') const;

  // --------------------------------------------------------------------------
  // Modifiers.
 public:

  // Instead of reversing the numbers, the symbol are reversed.
  // e.g. BigInteger(123).reverse will returns BigInteger(-123).
  //      BigInteger(-123).reverse will returns BigInteger(123).
  void reverse();

  void swap(BigInteger& rhs);

  // --------------------------------------------------------------------------
  // Overloads for commonly used operators, which are member functions.
 public:

  // Arithmetic assignment operators.
  BigInteger& operator+=(const BigInteger& rhs);
  BigInteger& operator-=(const BigInteger& rhs);
  BigInteger& operator*=(const BigInteger& rhs);
  BigInteger& operator/=(const BigInteger& rhs);
  BigInteger& operator%=(const BigInteger& rhs);
  BigInteger& operator<<=(const BigInteger& rhs);
  BigInteger& operator>>=(const BigInteger& rhs);

  // Pre-increment and pre-decrement.
  BigInteger& operator++();
  BigInteger& operator--();

  // Post-increment and post-decrement.
  BigInteger operator++(int);
  BigInteger operator--(int);

  // Unary plus and minus.
  BigInteger operator+();
  BigInteger operator-();

  // --------------------------------------------------------------------------
  // Overloads for commonly used operators, which are friend functions.
 public:

  // Overloads arithmetic operators.
  friend BigInteger operator+(const BigInteger& lhs, const BigInteger& rhs);
  friend BigInteger operator-(const BigInteger& lhs, const BigInteger& rhs);
  friend BigInteger operator*(const BigInteger& lhs, const BigInteger& rhs);
  friend BigInteger operator/(const BigInteger& lhs, const BigInteger& rhs);
  friend BigInteger operator%(const BigInteger& lhs, const BigInteger& rhs);

  friend BigInteger operator<<(const BigInteger& lhs, const BigInteger& rhs);
  friend BigInteger operator>>(const BigInteger& lhs, const BigInteger& rhs);

  // Overloads comparison operators.
  friend bool operator==(const BigInteger& lhs, const BigInteger& rhs);
  friend bool operator!=(const BigInteger& lhs, const BigInteger& rhs);
  friend bool operator< (const BigInteger& lhs, const BigInteger& rhs);
  friend bool operator> (const BigInteger& lhs, const BigInteger& rhs);
  friend bool operator<=(const BigInteger& lhs, const BigInteger& rhs);
  friend bool operator>=(const BigInteger& lhs, const BigInteger& rhs);

  // Overloads I/O stream operator.
  friend std::istream& operator>>(std::istream& is, BigInteger& b);
  friend std::ostream& operator<<(std::ostream& os, const BigInteger& b);

  // --------------------------------------------------------------------------
  // Helper functions.
 private:

  void        _symbol_pos();
  void        _symbol_neg();
  int16_t     _group(size_t n) const;
  size_t      _group_size() const;
  void        _group_push(int16_t value);
  void        _group_adjust(size_t n, int16_t value);
  void        _clear_excess_zeros();
  int64_t     _is_pow10() const;
  NumberType  _is_integer(const char* sz);
  void        _integer_init(uint64_t n, SymbolType negative);
  void        _string_init(const char* sz);
  bool        _add_and_carry(size_t n, int16_t value);
  std::string _group_to_string(size_t n) const;
  int16_t     _compare(const BigInteger& rhs) const;
  BigInteger& _plus_with_pos(const BigInteger& addend);
  BigInteger& _minus_with_pos(const BigInteger& subtrahend);
  BigInteger& _multiply_with_pos(const BigInteger& multiplier);
  BigInteger& _divide_with_pos(const BigInteger& divisor);
  BigInteger  _power_of(const BigInteger& n) const;
  void        _shift10(size_t n, ShiftType left);
  BigInteger  _high_range(size_t n) const;
  int16_t     _search(const BigInteger& dd, const BigInteger& ds) const;

  // --------------------------------------------------------------------------
  // Member data.
 private:

  // Divides a large integer into a group of four digits, that is, based on
  // a system of 10000. The large integer stored in the vector by group from
  // low bit to high bit. The type of each group is int16_t, the 14th bit of
  // first group controls the symbol, it is negative when this bit is 1.
  value_type number_;
};

// ============================================================================
// Template implementation.

template <typename T, typename U>
BigInteger::BigInteger(T n)
{
  _integer_init(static_cast<uint64_t>(
    redbud::safe_abs(n)), n < 0 ? kNegative : kPositive);
}

template <typename T>
std::pair<T, bool> BigInteger::to_integer() const
{
  if (compare(BigInteger(std::numeric_limits<T>::min())) < 0 ||
      compare(BigInteger(std::numeric_limits<T>::max())) > 0)
  {
    return std::make_pair(0, false);
  }
  T n = 0;
  if (is_negative())
  {
    for (int32_t i = _group_size() - 1; i >= 0; --i)
    {
      n = n * 10000 - _group(i);
    }
  }
  else
  {
    for (int32_t i = _group_size() - 1; i >= 0; --i)
    {
      n = n * 10000 + _group(i);
    }
  }
  return std::make_pair(n, true);
}

#if defined(REDBUD_MSVC)
  #pragma warning(pop)
#endif

} // namespace redbud_bignumber

typedef redbud_bignumber::BigInteger BigInteger;

} // namespace redbud
#endif // !ALINSHANS_REDBUD_BIGNUMBER_H_

