// ============================================================================
// Copyright (c) 2017 Alinshans. All rights reserved.
// Licensed under the MIT License. See LICENSE for details.
// 
// Header File : redbud/io/color.h 
//
// This file is used to control font format and color in the terminal, which
// refers to a project on github, see https://github.com/agauniyal/rang .
// ============================================================================

#ifndef ALINSHANS_REDBUD_IO_COLOR_H_
#define ALINSHANS_REDBUD_IO_COLOR_H_

#include "../platform.h"

#if defined(REDBUD_LINUX) || defined(REDBUD_OSX)
  #include <unistd.h>  // getenv
  #include <cstring>   // strstr
#elif defined(REDBUD_WIN)
  #include <Windows.h>
  #include <VersionHelpers.h>
  #include <io.h>
#endif

#include <cstdlib>
#include <iostream>
#include <type_traits>

namespace redbud
{
namespace io
{

// ============================================================================
// Enumerates the ANSI escape code corresponding to the font attributes.
// See https://en.wikipedia.org/wiki/ANSI_escape_code for details.
//
// Example:
//   using namespace redbud::io;
//   std::cout << fg::red << "This text has a red foreground color\n";
//   std::cout << bg::green << "This text has a green background color\n"
// ============================================================================

// Sets the text format, some of them is not widely supported.
enum class format
{
  reset      = 0,  // All attributes off.
  bold       = 1,  // Bold or increased intensity.
  faint      = 2,  // Faint (decreased intensity).
  italic     = 3,  // Italian font.
  underline  = 4,  // Underline.
  blinkslow  = 5,  // Blink slowly.
  blinkrapid = 6,  // Blink quickly.
  inverse    = 7,  // Swap foreground and background.
  conceal    = 8,  // Hide the text.
  strikeline = 9   // Characters legible, but marked for deletion.
};

// Sets the foreground color.
enum class fg
{
  black   = 30,
  red     = 31,
  green   = 32,
  yellow  = 33,
  blue    = 34,
  purple  = 35,
  cyan    = 36,
  white   = 37,
  reserve = 38,
  reset   = 39
};

// Sets the background color.
enum class bg
{
  black   = 40,
  red     = 41,
  green   = 42,
  yellow  = 43,
  blue    = 44,
  purple  = 45,
  cyan    = 46,
  white   = 47,
  reserve = 38,
  reset   = 39
};

// Sets the highlight foreground color.
enum class hfg
{
  black  = 90,
  red    = 91,
  green  = 92,
  yellow = 93,
  blue   = 94,
  purple = 95,
  cyan   = 96,
  white  = 97
};

// Sets the highlight background color.
enum class hbg
{
  black  = 100,
  red    = 101,
  green  = 102,
  yellow = 103,
  blue   = 104,
  purple = 105,
  cyan   = 106,
  white  = 107
};

// Sets the control state.
enum class state
{
  automatic = 0,  // Automatic control.
  manual    = 1   // Manual control.
};

// ----------------------------------------------------------------------------
// Details
namespace details
{

// Manages associated stream buffer.
inline const std::streambuf*& GetCoutBuf()
{
  static const std::streambuf* out_p = std::cout.rdbuf();
  return out_p;
}

inline const std::streambuf*& GetCerrBuf()
{
  static const std::streambuf* err_p = std::cerr.rdbuf();
  return err_p;
}

inline const std::streambuf*& GetClogBuf()
{
  static const std::streambuf* log_p = std::clog.rdbuf();
  return log_p;
}

// Gets an unique integer to use as index to iword()
inline int GetIword()
{
  static int i = std::ios_base::xalloc();
  return i;
}

// Determines whether the terminal color of this system can be modified.
inline bool IsMutable()
{
#if defined(REDBUD_LINUX) || defined(REDBUD_OSX)
  static constexpr const char* Terms[] = {
    "ansi", "color", "console", "cygwin", "gnome", "konsole", "kterm",
    "linux", "msys", "putty", "rxvt", "screen", "vt100", "xterm"
  };
  const char *env_p = std::getenv("TERM");
  if (env_p == nullptr)
  {
    return false;
  }
  bool result = false;
  for (const auto& t : Terms)
  {
    if (std::strstr(env_p, t) != nullptr)
    {
      result = true;
      break;
    }
  }

#elif defined(REDBUD_WIN)
  static constexpr bool result = true;
#endif
  return result;
}

/// Determines whether the buffer stream reaches the end.
inline bool IsTerminal(const std::streambuf* buf)
{
  if (buf == GetCoutBuf())
  {
#if defined(REDBUD_LINUX) || defined(REDBUD_OSX)
    return isatty(fileno(stdout)) ? true : false;
#elif defined(REDBUD_WIN)
    return _isatty(_fileno(stdout)) ? true : false;
#endif
  }
  if (buf == GetCerrBuf() || buf == GetClogBuf())
  {
#if defined(REDBUD_LINUX) || defined(REDBUD_OSX)
    return isatty(fileno(stderr)) ? true : false;
#elif defined(REDBUD_WIN)
    return _isatty(_fileno(stderr)) ? true : false;
#endif
  }
  return false;
}

// For overloading standard output stream.
template <typename T>
using color_return_t = typename std::enable_if<
  std::is_same<T, redbud::io::format>::value ||
  std::is_same<T, redbud::io::fg>::value ||
  std::is_same<T, redbud::io::bg>::value ||
  std::is_same<T, redbud::io::hfg>::value ||
  std::is_same<T, redbud::io::hbg>::value,
  std::ostream&>::type;

template <typename T>
using state_return_t = typename std::enable_if<
  std::is_same<T, redbud::io::state>::value,
  std::ostream&>::type;

// Sets the format and color of the text.
#if defined(REDBUD_LINUX) || defined(REDBUD_OSX)
template <typename T>
inline color_return_t<T> SetColor(std::ostream& os, const T& value)
{
  return os << "\033[" << static_cast<int>(value) << "m";
}

#elif defined(REDBUD_WIN)

static constexpr WORD default_state = (FOREGROUND_BLUE | 
                                       FOREGROUND_GREEN |
                                       FOREGROUND_RED);

// Gets the corresponding RGB value on Windows.
inline WORD GetWinRGB(WORD rgb)
{
  static constexpr WORD cor[8] = { 0, 4, 2, 6, 1, 5, 3, 7 };
  return cor[rgb];
}

// Sets font attributes on Windows.
inline void SetWinAttributes(redbud::io::fg color, WORD& state)
{
  if (color == redbud::io::fg::reserve)
  {
    return;
  }
  state &= 0xFFF0;
  if (color == redbud::io::fg::reset)
  {
    state |= default_state;
    return;
  }
  state |= GetWinRGB(static_cast<WORD>(color) - 30);
}

inline void SetWinAttributes(redbud::io::bg color, WORD& state)
{
  if (color == redbud::io::bg::reserve)
  {
    return;
  }
  state &= 0xFF0F;
  if (color == redbud::io::bg::reset)
  {
    return;
  }
  state |= GetWinRGB(static_cast<WORD>(color) - 40) << 4;
}

inline void SetWinAttributes(redbud::io::hfg color, WORD& state)
{
  state &= 0xFFF0;
  state |= (static_cast<WORD>(0x8) | 
            GetWinRGB(static_cast<WORD>(color) - 90));
}

inline void SetWinAttributes(redbud::io::hbg color, WORD& state)
{
  state &= 0xFF0F;
  state |= (static_cast<WORD>(0x80) | 
            GetWinRGB(static_cast<WORD>(color) - 100) << 4);
}

inline void SetWinAttributes(redbud::io::format format, WORD& state)
{
  if (format == redbud::io::format::reset)
  {
    state = default_state;
  }
}

inline WORD& current_state()
{
  static WORD state = default_state;
  return state;
}

inline HANDLE GetConsoleHandle()
{
  static HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
  return h;
}

template <typename T>
inline color_return_t<T> SetColor(std::ostream& os, const T& value)
{
  HANDLE h = GetConsoleHandle();
  if (h && IsTerminal(os.rdbuf()))
  {
    SetWinAttributes(value, current_state());
    SetConsoleTextAttribute(h, current_state());
    return os;
  }
  return os;
}
#endif

} // namespace details

// ----------------------------------------------------------------------------
// Overloads standard output stream to control the color of text.

template <typename T>
inline details::color_return_t<T> 
operator<<(std::ostream& os, const T& value)
{
  const std::streambuf* buf = os.rdbuf();
  return (os.iword(details::GetIword()) ||
          details::IsMutable() &&
          details::IsTerminal(os.rdbuf()))
    ? details::SetColor(os, value)
    : os;
}

template <typename T>
inline details::state_return_t<T> 
operator<<(std::ostream& os, const T& value)
{
  if (value == redbud::io::state::automatic)
  {
    os.iword(details::GetIword()) = 0;
  }
  else if (value == redbud::io::state::manual)
  {
    os.iword(details::GetIword()) = 1;
  }
  return os;
}

} // namespace io
} // namespace redbud
#endif // !ALINSHANS_REDBUD_IO_COLOR_H_
