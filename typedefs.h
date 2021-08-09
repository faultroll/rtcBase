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

#ifndef TYPEDEFS_H_
#define TYPEDEFS_H_

#ifndef RTC_NORETURN
// Annotate a function that will not return control flow to the caller.
#if defined(WEBRTC_WIN)
#define RTC_NORETURN __declspec(noreturn)
#elif defined(WEBRTC_POSIX)
#define RTC_NORETURN __attribute__ ((__noreturn__))
#else
#define RTC_NORETURN
#endif
#endif

#ifndef RTC_CHECKRETURN
#if defined(WEBRTC_WIN)
#define RTC_CHECKRETURN _Check_return_
#elif defined(WEBRTC_POSIX)
#define RTC_CHECKRETURN __attribute__ ((__warn_unused_result__))
#else
#define RTC_CHECKRETURN
#endif
#endif

// Prevent the compiler from warning about an unused variable. For example:
//   int result = DoSomething();
//   assert(result == 17);
//   RTC_UNUSED(result);
// Note: In most cases it is better to remove the unused variable rather than
// suppressing the compiler warning.
#ifndef RTC_UNUSED
#define RTC_UNUSED(x) static_cast<void>(x)
#endif  // RTC_UNUSED

#endif  // TYPEDEFS_H_
