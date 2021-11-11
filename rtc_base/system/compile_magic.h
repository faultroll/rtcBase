/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

// This file contains platform-specific typedefs and defines.
// Much of it is derived from Chromium's build/build_config.h.

#ifndef RTC_BASE_SYSTEM_COMPILE_MAGIC_H_
#define RTC_BASE_SYSTEM_COMPILE_MAGIC_H_

// From rtc_base/checks.h
#ifndef RTC_NORETURN
// Annotate a function that will not return control flow to the caller.
#if defined(WEBRTC_WIN)
#define RTC_NORETURN __declspec(noreturn)
#elif defined(WEBRTC_POSIX)
#define RTC_NORETURN __attribute__ ((__noreturn__))
#else
#define RTC_NORETURN
#endif // defined(WEBRTC_WIN)
#endif // RTC_NORETURN

// Added, used in rtc_base/critical_section.h
#ifndef RTC_CHECKRETURN
#if defined(WEBRTC_WIN)
#define RTC_CHECKRETURN _Check_return_
#elif defined(WEBRTC_POSIX)
#define RTC_CHECKRETURN __attribute__ ((__warn_unused_result__))
#else
#define RTC_CHECKRETURN
#endif // defined(WEBRTC_WIN)
#endif // RTC_CHECKRETURN

// From rtc_base/system/unused.h
// Prevent the compiler from warning about an unused variable. For example:
//   int result = DoSomething();
//   assert(result == 17);
//   RTC_UNUSED(result);
// Note: In most cases it is better to remove the unused variable rather than
// suppressing the compiler warning.
#ifndef RTC_UNUSED
#define RTC_UNUSED(x) static_cast<void>(x)
#endif // RTC_UNUSED

// From rtc_base/system/rtc_export.h
// RTC_EXPORT is used to mark symbols as exported or imported when WebRTC is
// built or used as a shared library.
// When WebRTC is built as a static library the RTC_EXPORT macro expands to
// nothing.
#ifndef RTC_EXPORT
#if defined(WEBRTC_WIN)
#define RTC_EXPORT __declspec(dllexport)
#elif defined(WEBRTC_POSIX)
#define RTC_EXPORT __attribute__((visibility("default")))
#else
#define RTC_EXPORT
#endif // defined(WEBRTC_WIN)
#endif // RTC_EXPORT

// From rtc_base/compile_assert_c.h
// Use this macro to verify at compile time that certain restrictions are met.
// The argument is the boolean expression to evaluate.
// Example:
//   RTC_COMPILE_ASSERT(sizeof(foo) < 128);
// Note: In C++, use static_assert instead!
#define RTC_COMPILE_ASSERT(expression) \
  switch (0) {                         \
    case 0:                            \
    case expression:;                  \
  }

#endif  // RTC_BASE_SYSTEM_COMPILE_MAGIC_H_
