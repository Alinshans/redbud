// ============================================================================
// Copyright (c) 2017 Alinshans. All rights reserved.
// Licensed under the MIT License. See LICENSE for details.
// 
// Source File : redbud/parser/reader.cc
//
// This file contains the implementation of Reader class.
// ============================================================================

#include "reader.h"

#include <cstring>

#include <fstream>

#include "tokenizer.h"
#include "../exception.h"

namespace redbud
{
namespace parser
{

// ----------------------------------------------------------------------------
// Copy Constructors.

Reader::Reader(const char* sz)
  :context_(sz), p_(0)
{
}

Reader::Reader(const std::string& str)
  : context_(str), p_(0)
{
}

Reader::Reader(std::string&& str)
  : context_(std::move(str)), p_(0)
{
}

Reader::Reader(std::ifstream* file)
  : p_(0)
{
  if (file == nullptr)
  {
    return;
  }
  char buf[1024];
  while (!file->eof())
  {
    file->getline(buf, sizeof(buf));
    context_ += buf;
  }
}

// ----------------------------------------------------------------------------
// Getter.

size_t Reader::getp() const
{
  return p_;
}

const std::string& Reader::gets() const
{
  return context_;
}

std::string Reader::getsub(size_t i, size_t n) const
{
  return context_.substr(i, n);
}

char Reader::now() const
{
  return context_[p_];
}

char Reader::next() const
{
  REDBUD_THROW_EX_IF(p_ + 1 >= context_.size(), "End of file.");
  return context_[p_ + 1];
}

bool Reader::eof() const
{
  return p_ == context_.size();
}

// ----------------------------------------------------------------------------
// Setter.

void Reader::to(int32_t n)
{
  p_ += n;
}

void Reader::skipspace()
{
  for (; Token::space(context_[p_]); ++p_)
    ; // Empty loop body.
}

void Reader::skip(char ch)
{
  if (context_[p_] == ch)
  {
    ++p_;
  }
}

void Reader::skip(const char* sz)
{
  size_t len = std::strlen(sz);
  size_t pz = 0;
  for (size_t p = p_; pz < len; ++p, ++pz)
  {
    if (context_[p] != *(sz + pz))
    {
      break;
    }
  }
  if (pz == len)
  {
    p_ += len;
  }
}

bool Reader::match(char ch)
{
  if (context_[p_] == ch)
  {
    ++p_;
    return true;
  }
  return false;
}

bool Reader::match(const std::string& str)
{
  if (context_.substr(p_, str.size()) == str)
  {
    p_ += str.size();
    return true;
  }
  return false;
}

bool Reader::match(std::function<bool(char)> f)
{
  if (f(context_[p_]))
  {
    ++p_;
    return true;
  }
  return false;
}

bool Reader::expect(char ch)
{
  REDBUD_THROW_PEX_IF(match(ch) == false,
                      std::to_string(ch),
                      std::to_string(context_[p_]),
                      p_);
  return true;
}

bool Reader::expect(const std::string& str)
{
  auto act = context_.substr(p_, str.size());
  REDBUD_THROW_PEX_IF(act != str, str.c_str(), act.c_str(), p_);
  p_ += str.size();
  return true;
}

bool Reader::expect(std::function<bool(char)> f)
{
  REDBUD_THROW_PEX_IF(match(f) == false,
                      "Makes the function return true",
                      std::to_string(context_[p_]),
                      p_);
  return true;
}

} // namespace parser
} // namespace redbud
