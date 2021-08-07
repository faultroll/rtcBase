/*
 *  Copyright 2004 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef RTC_BASE_CRITICALSECTION_H_
#define RTC_BASE_CRITICALSECTION_H_

#include "rtc_base/atomicops.h"
#include "rtc_base/checks.h"
#include "rtc_base/constructormagic.h"
#include "rtc_base/platform_thread_types.h"
#include "rtc_base/thread_annotations.h"
#include "typedefs.h"  // NOLINT(build/include)

#if !defined(NDEBUG) || defined(DCRIT_ALWAYS_ON)
#define RTC_DCRIT_IS_ON 1
#else
#define RTC_DCRIT_IS_ON 0
#endif

#if defined(WEBRTC_WIN)
// Include winsock2.h before including <windows.h> to maintain consistency with
// win32.h.  We can't include win32.h directly here since it pulls in
// headers such as basictypes.h which causes problems in Chromium where webrtc
// exists as two separate projects, webrtc and libjingle.
#include <winsock2.h>
#include <windows.h>
#include <sal.h>  // must come after windows headers.
#endif  // defined(WEBRTC_WIN)

#if defined(WEBRTC_POSIX)
#include <pthread.h>
#endif

#if RTC_DCRIT_IS_ON
#define CS_DEBUG_CODE(x) x
#else  // RTC_DCRIT_IS_ON
#define CS_DEBUG_CODE(x)
#endif  // RTC_DCRIT_IS_ON

namespace rtc {

// Locking methods (Enter, TryEnter, Leave)are const to permit protecting
// members inside a const context without requiring mutable CriticalSections
// everywhere.
class RTC_LOCKABLE CriticalSection {
 public:
  CriticalSection();
  ~CriticalSection();

  void Enter() const RTC_EXCLUSIVE_LOCK_FUNCTION();
  bool TryEnter() const RTC_EXCLUSIVE_TRYLOCK_FUNCTION(true);
  void Leave() const RTC_UNLOCK_FUNCTION();

 private:
  // Use only for RTC_DCHECKing.
  bool CurrentThreadIsOwner() const;

#if defined(WEBRTC_WIN)
  mutable CRITICAL_SECTION crit_;
#elif defined(WEBRTC_POSIX)
  mutable pthread_mutex_t mutex_;
# if RTC_DCRIT_IS_ON
  mutable Thrd thread_;  // Only used by RTC_DCHECKs.
  mutable int recursion_count_;       // Only used by RTC_DCHECKs.
# endif  // RTC_DCRIT_IS_ON
#else  // !defined(WEBRTC_WIN) && !defined(WEBRTC_POSIX)
# error Unsupported platform.
#endif
};

// CritScope, for serializing execution through a scope.
class RTC_SCOPED_LOCKABLE CritScope {
 public:
  explicit CritScope(const CriticalSection* cs) RTC_EXCLUSIVE_LOCK_FUNCTION(cs);
  ~CritScope() RTC_UNLOCK_FUNCTION();

 private:
  const CriticalSection* const cs_;
  RTC_DISALLOW_COPY_AND_ASSIGN(CritScope);
};

// Tries to lock a critical section on construction via
// CriticalSection::TryEnter, and unlocks on destruction if the
// lock was taken. Never blocks.
//
// IMPORTANT: Unlike CritScope, the lock may not be owned by this thread in
// subsequent code. Users *must* check locked() to determine if the
// lock was taken. If you're not calling locked(), you're doing it wrong!
class TryCritScope {
 public:
  explicit TryCritScope(const CriticalSection* cs);
  ~TryCritScope();
#if defined(WEBRTC_WIN)
  _Check_return_ bool locked() const;
#elif defined(WEBRTC_POSIX)
  bool locked() const __attribute__ ((__warn_unused_result__));
#else  // !defined(WEBRTC_WIN) && !defined(WEBRTC_POSIX)
# error Unsupported platform.
#endif
 private:
  const CriticalSection* const cs_;
  const bool locked_;
  mutable bool lock_was_called_;  // Only used by RTC_DCHECKs.
  RTC_DISALLOW_COPY_AND_ASSIGN(TryCritScope);
};

// A POD lock used to protect global variables. Do NOT use for other purposes.
// No custom constructor or private data member should be added.
class RTC_LOCKABLE GlobalLockPod {
 public:
  void Lock() RTC_EXCLUSIVE_LOCK_FUNCTION();

  void Unlock() RTC_UNLOCK_FUNCTION();

  volatile int lock_acquired;
};

class GlobalLock : public GlobalLockPod {
 public:
  GlobalLock();
};

// GlobalLockScope, for serializing execution through a scope.
class RTC_SCOPED_LOCKABLE GlobalLockScope {
 public:
  explicit GlobalLockScope(GlobalLockPod* lock)
      RTC_EXCLUSIVE_LOCK_FUNCTION(lock);
  ~GlobalLockScope() RTC_UNLOCK_FUNCTION();

 private:
  GlobalLockPod* const lock_;
  RTC_DISALLOW_COPY_AND_ASSIGN(GlobalLockScope);
};

} // namespace rtc

#endif // RTC_BASE_CRITICALSECTION_H_
