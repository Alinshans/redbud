// ============================================================================
// Copyright (c) 2017 Alinshans. All rights reserved.
// Licensed under the MIT License. See LICENSE for details.
// 
// Source File : redbud/bignumber.cc 
//
// This file contains the implementation of BigInteger class.
// ============================================================================

#include "bignumber.h"

namespace redbud
{
namespace redbud_bignumber
{

// ============================================================================
// Macro definition.

#define POS_PART  (0x3fffi16)  // Number part
#define NEG_PART  (0x4000i16)  // Symbol part

#define BITNUM(ch, d) (((ch) - '0') * d)

#define GROUP_NUM(sz, index)           \
  BITNUM(*((sz) + (index) - 1), 1) +   \
  BITNUM(*((sz) + (index) - 2), 10) +  \
  BITNUM(*((sz) + (index) - 3), 100) + \
  BITNUM(*((sz) + (index) - 4), 1000)

#define MOD(x)        ((x) % 10000)
#define DIV(x)        ((x) / 10000)
#define MUL(x)        ((x) * 10000)

#define POSITIVE1   (0x1i16)
#define NEGATIVE1   (0x4001i16)
#define MAX_GROUPS  (0x3FFFFFFEui32)
#define MAX_DIGITS  (0xFFFFFFFCi64)

// ============================================================================
// Constructor / Assignment operator

BigInteger::BigInteger(const char* s)
{
  _string_init(s);
}

BigInteger::BigInteger(const BigInteger& other) 
  :number_(other.number_)
{
}

BigInteger::BigInteger(BigInteger&& other)
  :number_(std::move(other.number_))
{
}

BigInteger& BigInteger::operator=(const char* s)
{
  BigInteger tmp(s);
  *this = std::move(tmp);
  return *this;
}

BigInteger& BigInteger::operator=(const BigInteger& other)
{
  number_ = other.number_;
  return *this;
}

BigInteger& BigInteger::operator=(BigInteger&& other)
{
  number_ = std::move(other.number_);
  return *this;
}

// ============================================================================
// Element access.

bool BigInteger::is_positive() const
{
  return !is_negative() && !is_zero();
}

bool BigInteger::is_negative() const
{
  return number_[0] > 10000;
}

bool BigInteger::is_zero() const
{
  return number_[0] == 0 && number_.size() == 1;
}

bool BigInteger::is_odd() const
{
  return (_group(0) & 1) == 1;
}

bool BigInteger::is_even() const
{
  return (_group(0) & 1) == 0;
}

int16_t BigInteger::compare(const self & other) const
{
  if (!is_negative() ^ !other.is_negative())
  {
    return is_negative() ? -1 : 1;
  }
  else if (!is_negative())
  {
    return _compare(other);
  }
  else
  {
    return other._compare(*this);
  }
}

size_t BigInteger::digits() const
{
  size_t d = ((_group_size() - 1) << 2) + 1;
  for (int16_t i = 10; number_.back() / i; ++d, i *= 10)
    ; // Empty loop body.
  return d;
}

size_t BigInteger::max_digits() const
{
  return static_cast<size_t>(-4);
}

BigInteger BigInteger::opposite() const
{
  BigInteger result(*this);
  result.reverse();
  return result;
}

BigInteger BigInteger::absolute() const
{
  BigInteger result(*this);
  result._symbol_pos();
  return result;
}

BigInteger BigInteger::power(const BigInteger& n) const
{
  return _power_of(n);
}

std::string BigInteger::to_string() const
{
  std::string s = is_negative() ? "-" : "";
  for (int32_t i = _group_size() - 1; i >= 0; --i)
  {
    s += _group_to_string(i);
  }
  return s;
}

void BigInteger::print(char sep) const
{
  if (is_negative())
  {
    std::printf("-%d", _group(_group_size() - 1));
  }
  else
  {
    std::printf("%d", _group(_group_size() - 1));
  }
  for (int32_t i = _group_size() - 2; i >= 0; --i)
  {
    std::printf("%04d", _group(i));
  }
  if (sep != '\0')
  {
    std::printf("%c", sep);
  }
}

// ============================================================================
// Modifiers.

void BigInteger::reverse()
{
  if (is_negative())
  {
    _symbol_pos();
  }
  else if (is_positive())
  {
    _symbol_neg();
  }
}

void BigInteger::swap(BigInteger& rhs)
{
  number_.swap(rhs.number_);
}

// ============================================================================
// Overloads operators.

BigInteger& BigInteger::operator+=(const BigInteger& addend)
{
  if (!is_negative() && !addend.is_negative())
  {
    _plus_with_pos(addend);
  }
  else if (!is_negative() && addend.is_negative())
  {
    // x + (-y) = x - y
    _minus_with_pos(addend.opposite());
  }
  else if (is_negative() && !addend.is_negative())
  {
    // (-x) + y = -(x - y)
    _symbol_pos();
    _minus_with_pos(addend);
    reverse();
  }
  else
  {
    // (-x) + (-y) = -(x + y)
    _symbol_pos();
    _plus_with_pos(addend.opposite());
    _symbol_neg();
  }
  return *this;
}

BigInteger& BigInteger::operator-=(const BigInteger& subtrahend)
{
  if (!is_negative() && !subtrahend.is_negative())
  {
    _minus_with_pos(subtrahend);
  }
  else if (!is_negative() && subtrahend.is_negative())
  {
    // x - (-y) = x + y
    _plus_with_pos(subtrahend.opposite());
  }
  else if (is_negative() && !subtrahend.is_negative())
  {
    // (-x) - y = -(x + y)
    _symbol_pos();
    _plus_with_pos(subtrahend);
    _symbol_neg();
  }
  else
  {
    // (-x) - (-y) = -(x - y)
    _symbol_pos();
    _minus_with_pos(subtrahend.opposite());
    reverse();
  }
  _clear_excess_zeros();
  return *this;
}

BigInteger& BigInteger::operator*=(const BigInteger& multiplier)
{
  if (is_zero() || multiplier.is_zero())
  {
    *this = std::move(BigInteger(0));
    return *this;
  }
  bool result_neg = is_negative() ^ multiplier.is_negative();
  _symbol_pos();
  int64_t pow_of_ten = multiplier.absolute()._is_pow10();
  if (pow_of_ten >= 0)
  {
    _shift10(static_cast<size_t>(pow_of_ten), kMoveLeft);
  }
  else
  {
    _multiply_with_pos(multiplier.absolute());
  }
  if (result_neg)
  {
    _symbol_neg();
  }
  return *this;
}

BigInteger& BigInteger::operator/=(const BigInteger& divisor)
{
  REDBUD_THROW_EX_IF(divisor.is_zero(), "The divisor can not be zero.");
  bool result_neg = is_negative() ^ divisor.is_negative();
  _symbol_pos();
  int16_t comp = compare(divisor.absolute());
  if (is_zero() || comp < 0)
  {
    *this = std::move(BigInteger(0));
  }
  else if (comp == 0)
  {
    *this = std::move(BigInteger(1));
  }
  else
  {
    int64_t pow_of_ten = divisor.absolute()._is_pow10();
    if (pow_of_ten >= 0)
    {
      _shift10(static_cast<size_t>(pow_of_ten), kMoveRight);
    }
    else
    {
      _divide_with_pos(divisor.absolute());
    }
  }
  if (result_neg)
  {
    _symbol_neg();
  }
  return *this;
}

BigInteger& BigInteger::operator%=(const BigInteger& modulus)
{
  REDBUD_THROW_EX_IF(modulus.is_zero(), "The modulus can not be zero.");
  // Modulus operation(%) only guaranteed (a / b) * b + (a % b) == a.
  self tmp = (*this / modulus) * modulus;
  *this -= tmp;
  return *this;
}

BigInteger& BigInteger::operator<<=(const BigInteger& n)
{
  REDBUD_THROW_EX_IF(n.is_negative(), "Positive required.");
  *this *= BigInteger(2).power(n);
  return *this;
}

BigInteger& BigInteger::operator>>=(const BigInteger& n)
{
  REDBUD_THROW_EX_IF(n.is_negative(), "Positive required.");
  *this /= BigInteger(2).power(n);
  return *this;
}

BigInteger& BigInteger::operator++()
{
  if (!is_negative())
  {
    size_t g = 0;
    bool carry = false;
    do
    {
      carry = _add_and_carry(g, 1);
      ++g;
    } while (carry);
  }
  else
  {
    // -x + 1 = -(x - 1)
    reverse();
    operator--();
    reverse();
  }
  return *this;
}

BigInteger& BigInteger::operator--()
{
  if (is_zero())
  {
    number_[0] = NEGATIVE1;
  }
  else if (!is_negative())
  {
    size_t g = 0;
    bool borrow = false;
    do
    {
      if (_group(g) == 0)
      {
        _group_adjust(g, 9999);
        ++g, borrow = 1;
      }
      else
      {
        borrow = _add_and_carry(g, -1);
      }
    } while (borrow);
    _clear_excess_zeros();
  }
  else
  {
    // -x - 1 = -(x + 1)
    reverse();
    operator++();
    reverse();
  }
  return *this;
}

BigInteger BigInteger::operator++(int)
{
  BigInteger tmp(*this);
  ++*this;
  return tmp;
}

BigInteger BigInteger::operator--(int)
{
  BigInteger tmp(*this);
  --*this;
  return tmp;
}

BigInteger BigInteger::operator+()
{
  return *this;
}

BigInteger BigInteger::operator-()
{
  return opposite();
}

// ============================================================================
// Helper functions.

// Set the sign of this BigInteger.
void BigInteger::_symbol_pos()
{
  number_[0] &= POS_PART;
}

void BigInteger::_symbol_neg()
{
  if (!is_zero())
  {
    number_[0] |= NEG_PART;
  }
}

// Returns the number of nth group, does not include the sign.
int16_t BigInteger::_group(size_t n) const
{
  return number_[n] & POS_PART;
}

size_t BigInteger::_group_size() const
{
  return number_.size();
}

void BigInteger::_group_push(int16_t value)
{
  REDBUD_THROW_EX_IF(_group_size() >= MAX_GROUPS, "Overflow.");
  number_.push_back(value);
}

void BigInteger::_group_adjust(size_t n, int16_t value)
{
  number_[n] = value;
}

// Clear the extra zero in front of this BigInteger.
void BigInteger::_clear_excess_zeros()
{
  for (size_t i = _group_size() - 1; i > 0 && _group(i) == 0; --i)
  {
    number_.pop_back();
  }
}

// The BigInteger calls this function must be non-negative.
// If this BigInteger is power of ten, then returns to the power.
// Otherwise it returns -1.
int64_t BigInteger::_is_pow10() const
{
  std::string num = to_string();
  if (num[0] == '1' && num.find_first_not_of('0', 1) == num.npos)
  {
    return num.size() - 1;
  }
  return -1;
}

// An integer should satisfy the following rules:
//   1. It can have at most one sign.
//   2. No extra zero can appear in front of the number.
//   3. Numbers can be expressed in general notation or standard 
//      scientific notation "a*10^b"(aEb), where 1<= |a| < 10 and b >= 1.
//
// The regular expression to match an integer string looks like this:
//   r```("^[+-]?"                              # sign
//        "(0|"                                 # 0
//        "[1-9]\d*|"                           # positive integer
//        "[1-9](\.\d+)?[eE]\+?[1-9]\d*)$")```  # scientific notation
NumberType BigInteger::_is_integer(const char* sz)
{
  if (*sz == '+' || *sz == '-')
  {
    sz++;
  }
  if (*sz == '0')
  {
    // Matches zero.
    REDBUD_THROW_EX_IF(*(sz + 1) != '\0', "Invalid expression.");
    return kZero;
  }
  else
  {
    REDBUD_THROW_EX_IF(*sz < '1' || *sz > '9', "Invalid expression.");
    sz++;
    if (*sz == '.' || *sz == 'e' || *sz == 'E')
    {
      // Matches the integer of scientific notation.
      if (*sz == '.')
      {
        sz++;
        REDBUD_THROW_EX_IF(*sz < '0' || *sz > '9', "Invalid expression.");
        for (sz++; '0' <= *sz && *sz <= '9'; sz++)
          ; // Empty loop body
      }
      sz++;
      if (*sz == '+')
      {
        sz++;
      }
      REDBUD_THROW_EX_IF(*sz < '1' || *sz > '9', "Invalid expression.");
      for (sz++; '0' <= *sz && *sz <= '9'; sz++)
        ; // Empty loop body
      REDBUD_THROW_EX_IF(*sz != '\0', "Invalid expression.");
      return kScientificNotation;
    }
    else
    {
      for (; '0' <= *sz && *sz <= '9'; sz++)
        ; // Empty loop body
      REDBUD_THROW_EX_IF(*sz != '\0', "Invalid expression.");
      return kPositiveInteger;
    }
  }
}

// Initialize with numeric literal.
void BigInteger::_integer_init(uint64_t n, SymbolType negative)
{
  if (n == 0)
  {
    _group_push(static_cast<int16_t>(0));
    return;
  }
  while (n)
  {
    _group_push(static_cast<int16_t>(n % 10000));
    n /= 10000;
  }
  if (negative)
  {
    _symbol_neg();
  }
}

// Initialize with string literal.
void BigInteger::_string_init(const char* sz)
{
  NumberType t = _is_integer(sz);
  if (t == kZero)
  {
    number_.push_back(0);
    return;
  }
  bool result_neg = false;
  if (*sz == '+' || *sz == '-')
  {
    result_neg = *sz == '-';
    sz++;
  }

  if (t == kPositiveInteger)
  {
    // Initialize with a positive integer.
    size_t i = strlen(sz);
    for (; 4 < i; i -= 4)
    {
      _group_push(GROUP_NUM(sz, i));
    }
    int16_t num = 0;
    for (int16_t m = 1; i > 0; --i, m *= 10)
    {
      num += BITNUM(*(sz + i - 1), m);
    }
    _group_push(num);
  }
  else
  {
    // Initialize with scientific notation.
    size_t exp = 0;
    size_t i = strlen(sz);
    for (; '0' <= *(sz + i - 1) && *(sz + i - 1) <= '9'; --i)
      ; // Empty loop body
    size_t shift = strtoul(sz + i, nullptr, 10);
    // TODO: do not use for
    for (; *(sz + i - 1) < '0' || *(sz + i - 1) > '9'; exp = i, --i)
      ; // Empty loop body
    if (i == 1)
    { // xen = x * 10^n
      _group_push(*sz - '0');
      _shift10(shift, kMoveLeft);
    }
    else
    {
      for (; 5 < i; i -= 4)
      {
        _group_push(GROUP_NUM(sz, i));
      }
      int16_t num = 0;
      for (int16_t m = 1; i > 0; --i)
      {
        if (*(sz + i - 1) == '.')
        {
          continue;
        }
        num += BITNUM(*(sz + i - 1), m);
        m *= 10;
      }
      _group_push(num);
      // Notes that the exp is at least 3.
      REDBUD_THROW_EX_IF(shift < exp - 3, "Not an integer string.");
      _shift10(shift - (exp - 3), kMoveLeft);
    }
  }
  if (result_neg)
  {
    _symbol_neg();
  }
}

// The BigInteger calls this function must be non-negative.
// The nth group adds the value on the original basis and 
// returns true if it has a carry.
inline bool BigInteger::_add_and_carry(size_t n, int16_t value)
{
  REDBUD_THROW_EX_IF(n >= MAX_GROUPS &&
                     number_[n] + value > 9999, "Overflow.");
  if (n == number_.size())
  {
    number_.push_back(value);
    return false;
  }
  // 0 <= number_[n], value < 10000
  // if value + number_[n] < 10000
  //    number_[n] = value + number_[n] >= value
  // if value + number_[n] >= 10000
  //    number_[n] = value + number_[n] - 10000 
  //               = value - (10000 - number_[n]) < value
  number_[n] = MOD(number_[n] + value);
  return number_[n] < value;
}

std::string BigInteger::_group_to_string(size_t n) const
{
  if (n == _group_size() - 1)
  {
    return std::to_string(_group(n));
  }
  int16_t num = _group(n);
  std::string str;
  if (num < 10)
  {
    str += "000";
  }
  else if (num < 100)
  {
    str += "00";
  }
  else if (num < 1000)
  {
    str += "0";
  }
  str += std::to_string(num);
  return str;
}

int16_t BigInteger::_compare(const BigInteger& rhs) const
{
  int32_t i = _group_size() - 1;
  int32_t j = rhs._group_size() - 1;
  if (i != j)
  {
    return j < i ? 1 : -1;
  }
  for (; i >= 0; --i, --j)
  {
    if (_group(i) > rhs._group(j))
    {
      return 1;
    }
    if (rhs._group(i) > _group(j))
    {
      return -1;
    }
  }
  return 0;
}


BigInteger& BigInteger::_plus_with_pos(const BigInteger& addend)
{
  if (addend.is_zero())
  {
    return *this;
  }
  if (is_zero())
  {
    *this = addend;
    return *this;
  }

  int16_t carry = 0;
  size_t i = 0;
  for (; i < _group_size() && i < addend._group_size(); ++i)
  {
    int16_t sum = _group(i) + addend._group(i) + carry;
    _group_adjust(i, MOD(sum));
    carry = DIV(sum);
  }
  for (; i < _group_size(); ++i)
  {
    int16_t sum = _group(i) + carry;
    _group_adjust(i, MOD(sum));
    carry = DIV(sum);
  }
  for (; i < addend._group_size(); ++i)
  {
    int16_t sum = addend._group(i) + carry;
    _group_push(MOD(sum));
    carry = DIV(sum);
  }
  if (carry)
  {
    _group_push(carry);
  }
  return *this;
}

BigInteger& BigInteger::_minus_with_pos(const BigInteger& subtrahend)
{
  if (subtrahend.is_zero())
  {
    return *this;
  }
  if (is_zero())
  {
    *this = subtrahend.opposite();
    return *this;
  }

  // The larger one minus the smaller one.
  BigInteger larger(*this);
  BigInteger smaller(subtrahend);
  int16_t cmp = larger.compare(smaller);
  if (cmp == 0)
  {
    *this = std::move(BigInteger(0));
    return *this;
  }
  bool neg_symbol = false;
  if (cmp < 0)
  {
    larger.swap(smaller);
    neg_symbol = true;
  }

  int16_t borrow = 0;
  size_t i = 0;
  for (; i < smaller._group_size(); ++i)
  {
    int16_t diff = larger._group(i) - smaller._group(i) - borrow;
    if (diff < 0)
    {
      diff += 10000;
      borrow = 1;
    }
    else
    {
      borrow = 0;
    }
    larger._group_adjust(i, diff);
  }
  for (; i < larger._group_size(); ++i)
  {
    int16_t diff = larger._group(i) - borrow;
    if (diff < 0)
    {
      diff += 10000;
      borrow = 1;
    }
    else
    {
      borrow = 0;
    }
    larger._group_adjust(i, diff);
  }
  larger._clear_excess_zeros();
  if (neg_symbol)
  {
    larger._symbol_neg();
  }
  *this = std::move(larger);
  return *this;
}

BigInteger& BigInteger::_multiply_with_pos(const BigInteger& m)
{
  // The product of n-digits number multiplied by m-digits number
  // has at least (n + m - 1) digits.
  REDBUD_THROW_EX_IF(static_cast<int64_t>(digits()) +
                     static_cast<int64_t>(m.digits()) - 1 > MAX_DIGITS,
                     "Overflow.");
  // The group of result is less than or equal to the sum of two multiplier.
  BigInteger result(0);
  result.number_.assign(_group_size() + m._group_size(), 0);

  size_t i = 0;
  size_t j = 0;
  for (; j < m._group_size(); ++j)
  {
    int32_t carry = 0;
    for (i = 0; i < _group_size(); ++i)
    {
      int32_t p = static_cast<int32_t>(m._group(j))
        * static_cast<int32_t>(_group(i)) + carry;
      carry = result._add_and_carry(j + i, MOD(p)) ? DIV(p) + 1 : DIV(p);
    }
    if (carry != 0)
    { // The carry must be less than 10000.
      result._add_and_carry(j + i, carry);
    }
  }
  result._clear_excess_zeros();
  *this = std::move(result);
  return *this;
}

BigInteger& BigInteger::_divide_with_pos(const BigInteger& divisor)
{
  size_t g1 = _group_size();
  size_t g2 = divisor._group_size();
  BigInteger dividend(0);
  BigInteger multiple(0);
  BigInteger result(0);
  result.number_.assign(g1, 0);

  // Each time take the same group as divisor's as new dividend,
  // if dividend is less than divisor, takes a more group.
  // Then finds their quotient as the result in the corresponding
  // group of the result, and minus the corresponding multiple.
  // Repeats this process until the number of their group is equal. 
  for (; g1 >= g2; g1 = _group_size())
  {
    dividend = _high_range(g2);
    int16_t cmp = dividend.compare(divisor);
    if (g1 == g2)
    {
      if (cmp < 0)
      {
        result._clear_excess_zeros();
        *this = std::move(result);
        return *this;
      }
      if (cmp == 0)
      {
        result._group_adjust(g1 - 1, 1);
        result._clear_excess_zeros();
        *this = std::move(result);
        return *this;
      }
    }
    size_t borrow = cmp < 0 ? 1 : 0;
    dividend = _high_range(g2 + borrow);
    int32_t quotient = _search(dividend, divisor);
    multiple = (divisor * BigInteger(quotient));
    multiple._shift10((g1 - g2 - borrow) << 2, kMoveLeft);
    result._group_adjust(g1 - g2 - borrow, quotient);
    *this -= multiple;
  }
  result._clear_excess_zeros();
  *this = std::move(result);
  return *this;
}

BigInteger BigInteger::_power_of(const BigInteger& n) const
{
  REDBUD_THROW_EX_IF(is_zero() && !n.is_positive(), "Invalid value.");
  if (is_zero() || (_group(0) != 1 && n.is_negative()))
  {
    return BigInteger(0);
  }
  if (n.is_zero() ||                            
     (number_.size() == 1 && number_[0] == POSITIVE1) ||
     (number_.size() == 1 && number_[0] == NEGATIVE1 && n.is_even()))
  { // x^0, x != 0 || 1^n || (-1)^n, n is even
    return BigInteger(1);
  }
  if (number_.size() == 1 && number_[0] == NEGATIVE1 && n.is_odd()) 
  { // (-1)^n, n is odd
    return BigInteger(-1);
  }
  if (n.number_.size() == 1 && n.number_[0] == POSITIVE1)             
  { // x^1
    return BigInteger(*this);
  }

  // If this BigInteger is power of ten, just shift to get the result.
  int64_t p = absolute()._is_pow10();
  if (p >= 0)
  {
    auto pair = n.to_integer<uint32_t>();
    REDBUD_THROW_EX_IF(pair.second == false, "Overflow.");
    int64_t shift = p * static_cast<int64_t>(pair.first);
    REDBUD_THROW_EX_IF(shift >= MAX_DIGITS, "Overflow.");
    BigInteger result(1);
    result._shift10(static_cast<size_t>(shift), kMoveLeft);
    if (is_negative() && n.is_odd())
    {
      result._symbol_neg();
    }
    return result;
  }

  int64_t d1 = digits() - 1;
  auto pair = n.to_integer<uint32_t>();
  REDBUD_THROW_EX_IF(pair.second == false, "Overflow.");
  int64_t shift = d1 * static_cast<int64_t>(pair.first);
  REDBUD_THROW_EX_IF(shift >= MAX_DIGITS, "Overflow.");
  BigInteger result(*this);
  if (n.is_odd())
  {
    result = result.power(n / 2) * result.power(n / 2) * result;
  }
  else
  {
    result = result.power(n / 2) * result.power(n / 2);
  }
  return result;
}

// Shift int the decimal system(base 10).
void BigInteger::_shift10(size_t n, ShiftType direction)
{
  if (is_zero())
  {
    return;
  }
  if (direction == kMoveLeft)
  {
    REDBUD_THROW_EX_IF(static_cast<int64_t>(digits()) +
                       static_cast<int64_t>(n) > MAX_DIGITS, "Overflow.");
    if ((n & 3) == 0)
    {  // Multiple of 4
      number_.insert(number_.begin(), n >> 2, 0);
      return;
    }
    std::string num = to_string();
    num.append(n, '0');
    *this = num.data();
  }
  else // Moveright.
  {
    if (n >= digits())
    {
      number_.clear();
    }
    else
    {
      if ((n & 3) == 0)
      {  // Multiple of 4
        number_.erase(number_.begin(), number_.begin() + (n >> 2));
        return;
      }
      std::string num = to_string();
      num.erase(num.end() - n, num.end());
      *this = num.data();
    }
  }
}

// Returns a BigInteger which is former n group of this BigInteger.
// e.g. If there is a BigInteger b("123456789999"), b._high_range(1) will 
// get BigInteger("1234"), b._high_range(2) will get BigInteger("12345678").
BigInteger BigInteger::_high_range(size_t n) const
{
  size_t g = _group_size();
  BigInteger result(0);
  for (size_t i = 1; i <= n; ++i)
  {
    BigInteger g_num(_group(g - i));
    g_num._shift10((n - i) << 2, kMoveLeft);
    result += g_num;
  }
  return result;
}

// Finds the quotient of dividend and divisor by binary search.
int16_t BigInteger::
_search(const BigInteger& dividend, const BigInteger& divisor) const
{
  // The range of quotient is [1, 9999]
  int16_t low = 1;
  int16_t high = 9999;
  while (low < high)
  {
    int16_t half = (low + high) >> 1;
    if (low + 1 == high)
    {
      return dividend.compare(divisor * BigInteger(high)) < 0 ? low : high;
    }
    if (dividend.compare(divisor * BigInteger(half)) < 0)
    {
      high = half - 1;
    }
    else
    {
      low = half;
    }
  }
  return low;
}

// ============================================================================
// Overloads arithmetic operators.

BigInteger operator+(const BigInteger& lhs, const BigInteger& rhs)
{
  BigInteger result(lhs);
  result += rhs;
  return result;
}

BigInteger operator-(const BigInteger& lhs, const BigInteger& rhs)
{
  BigInteger result(lhs);
  result -= rhs;
  return result;
}

BigInteger operator*(const BigInteger& lhs, const BigInteger& rhs)
{
  BigInteger result(lhs);
  result *= rhs;
  return result;
}

BigInteger operator/(const BigInteger& lhs, const BigInteger& rhs)
{
  BigInteger result(lhs);
  result /= rhs;
  return result;
}

BigInteger operator%(const BigInteger& lhs, const BigInteger& rhs)
{
  BigInteger result(lhs);
  result %= rhs;
  return result;
}

BigInteger operator<<(const BigInteger& lhs, const BigInteger& rhs)
{
  BigInteger result(lhs);
  result <<= rhs;
  return result;
}

BigInteger operator >> (const BigInteger& lhs, const BigInteger& rhs)
{
  BigInteger result(lhs);
  result >>= rhs;
  return result;
}

// ============================================================================
// Overloads comparison opeaators.

bool operator==(const BigInteger& lhs, const BigInteger& rhs)
{
  return lhs.number_ == rhs.number_;
}

bool operator!=(const BigInteger& lhs, const BigInteger& rhs)
{
  return !(lhs == rhs);
}

bool operator<(const BigInteger& lhs, const BigInteger& rhs)
{
  return lhs.compare(rhs) < 0;
}

bool operator>(const BigInteger& lhs, const BigInteger& rhs)
{
  return rhs < lhs;
}

bool operator<=(const BigInteger& lhs, const BigInteger& rhs)
{
  return !(rhs < lhs);
}

bool operator>=(const BigInteger& lhs, const BigInteger& rhs)
{
  return !(lhs < rhs);
}

// ============================================================================
// Overloads I/O stream operator.

std::istream& operator >> (std::istream& is, BigInteger& b)
{
  std::string buf;
  is >> buf;
  b.number_.clear();
  b._string_init(buf.data());
  return is;
}

std::ostream& operator<<(std::ostream& os, const BigInteger& b)
{
  b.print();
  return os;
}

// ============================================================================
// Cancels macro definition.

#undef POS_PORT
#undef NEG_PORT
#undef BITNUM
#undef GROUP_NUM
#undef MOD
#undef DIV
#undef MUL
#undef POSITIVE1
#undef NEGATIVE1
#undef MAX_GROUPS
#undef MAX_DIGITS

} // namespace redbud_bignumber
} // namespace redbud