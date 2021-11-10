/*
 *  Copyright 2011 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef RTC_BASE_ATOMIC_OPS_H_
#define RTC_BASE_ATOMIC_OPS_H_

/**
 * std::unique_ptr is in <memory> which includes <shared_ptr_atomic.h> 
 * however, it only implements the atomic_flag and cannot use <stdatomics.h>
 * 
 */
#if defined(WEBRTC_WIN)
    #define _USE_WIN
#elif defined(WEBRTC_POSIX)
    #define _USE_POSIX
    #ifndef _POSIX_C_SOURCE
        #define _POSIX_C_SOURCE 200809L
    #endif /* _POSIX_C_SOURCE */
#else
    #define _USE_NONE
#endif // defined(WEBRTC_WIN)
#include "rtc_base/c11runtime/catomic.h"

namespace rtc {
class AtomicOps {
 public:
  // reinterpret_cast<volatile LONG*>(i)
  // Assumes sizeof(int) == sizeof(LONG), which it is on Win32 and Win64.
  static int Increment(volatile int* i) {
    return ATOMIC_VAR_FAA(i, 1);
  }
  static int Decrement(volatile int* i) {
    return ATOMIC_VAR_FAA(i, -1);
  }
  static int AcquireLoad(volatile const int* i) {
    return ATOMIC_VAR_LOAD(i);
  }
  static void ReleaseStore(volatile int* i, int value) {
    ATOMIC_VAR_STOR(i, value);
  }
  static bool CompareAndSwap(volatile int* i, int old_value, int new_value) {
    return ATOMIC_VAR_CAS(i, old_value, new_value);
  }
};

}  // namespace rtc

#endif  // RTC_BASE_ATOMIC_OPS_H_
