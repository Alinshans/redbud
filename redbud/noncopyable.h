// ============================================================================
// Copyright (c) 2017 Alinshans. All rights reserved.
// Licensed under the MIT License. See LICENSE for details.
// 
// Header File : redbud/noncopyable.h 
//
// This file contains a noncopyable class, those classes which can not be
// copied should inherit this class.
// ============================================================================

#ifndef ALINSHANS_REDBUD_NONCOPYABLE_H_
#define ALINSHANS_REDBUD_NONCOPYABLE_H_

namespace redbud
{

// Avoid name conflicts.
namespace redbud_noncopyable
{

// ============================================================================
// Base class : noncopyable
//
// Deletes copy constructor and copy assignment ensure 
// classes derived from this class can not be copied.
//
// Example:
//   class Employee : public noncopyable { ... };
//   
//   Employee somepeople;
//   Employee otherpeople(somepeople); // error
class noncopyable
{

 protected:

  noncopyable() = default;
  ~noncopyable() = default;

  noncopyable(const noncopyable&) = delete;
  noncopyable& operator=(const noncopyable&) = delete;

};

} // namespace redbud_noncopyable

typedef redbud_noncopyable::noncopyable noncopyable;

} // namespace redbud
#endif // !ALINSHANS_REDBUD_NONCOPYABLE_H_

