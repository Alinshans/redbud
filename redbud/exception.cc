// ============================================================================
// Copyright (c) 2017 Alinshans. All rights reserved.
// Licensed under the MIT License. See LICENSE for details.
// 
// Source File : redbud/exception.cc 
//
// This file contains the implementation of Exception class.
// ============================================================================

#include "exception.h"

#include <cstdio>
#include <cstdlib>

namespace redbud
{

// ----------------------------------------------------------------------------
// Outputs the exception messages and quits.

Exception::Exception(const std::string& cond,
                     const std::string& msg,
                     const char* file,
                     size_t line)
{
  fprintf(stderr, "Exception : [ %s ], %s : %d\n"
          "     Note : %s\n",
          cond.c_str(), file, line, msg.c_str());
  exit(1);
}

ParseException::ParseException(const std::string& cond,
                               const std::string& exp,
                               const std::string& act,
                               size_t pos,
                               const char* file,
                               size_t line)
{
  fprintf(stderr, "Exception : [ %s ], %s : %d\n"
          "   Expect : %s, Actual : %s at postion %d.\n",
          cond.c_str(), file, line, exp.c_str(), act.c_str(), pos);
  exit(1);
}

} // namespace redbud

