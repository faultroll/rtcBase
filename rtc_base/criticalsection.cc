/*
 *  Copyright 2015 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "rtc_base/criticalsection.h"

#include "rtc_base/checks.h"
#include "rtc_base/platform_thread_types.h"

// TODO(tommi): Split this file up to per-platform implementation files.

namespace rtc {

CriticalSection::CriticalSection() {
#if defined(WEBRTC_WIN)
  InitializeCriticalSection(&crit_);
#elif defined(WEBRTC_POSIX)
  pthread_mutexattr_t mutex_attribute;
  pthread_mutexattr_init(&mutex_attribute);
  pthread_mutexattr_settype(&mutex_attribute, PTHREAD_MUTEX_RECURSIVE);
  pthread_mutex_init(&mutex_, &mutex_attribute);
  pthread_mutexattr_destroy(&mutex_attribute);

  CS_DEBUG_CODE(thread_ = 0);
  CS_DEBUG_CODE(recursion_count_ = 0);
#else
# error Unsupported platform.
#endif
}

CriticalSection::~CriticalSection() {
#if defined(WEBRTC_WIN)
  DeleteCriticalSection(&crit_);
#elif defined(WEBRTC_POSIX)
  pthread_mutex_destroy(&mutex_);
#else
# error Unsupported platform.
#endif
}

void CriticalSection::Enter() const RTC_EXCLUSIVE_LOCK_FUNCTION() {
#if defined(WEBRTC_WIN)
  EnterCriticalSection(&crit_);
#elif defined(WEBRTC_POSIX)
  pthread_mutex_lock(&mutex_);

# if RTC_DCRIT_IS_ON
  if (!recursion_count_) {
    RTC_DCHECK(!thread_);
    thread_ = CurrentThreadRef();
  } else {
    RTC_DCHECK(CurrentThreadIsOwner());
  }
  ++recursion_count_;
# endif  // RTC_DCRIT_IS_ON
#else
# error Unsupported platform.
#endif
}

bool CriticalSection::TryEnter() const RTC_EXCLUSIVE_TRYLOCK_FUNCTION(true) {
#if defined(WEBRTC_WIN)
  return TryEnterCriticalSection(&crit_) != FALSE;
#elif defined(WEBRTC_POSIX)
  if (pthread_mutex_trylock(&mutex_) != 0)
    return false;

# if RTC_DCRIT_IS_ON
  if (!recursion_count_) {
    RTC_DCHECK(!thread_);
    thread_ = CurrentThreadRef();
  } else {
    RTC_DCHECK(CurrentThreadIsOwner());
  }
  ++recursion_count_;
# endif  // RTC_DCRIT_IS_ON
  return true;
#else
# error Unsupported platform.
#endif
}

void CriticalSection::Leave() const RTC_UNLOCK_FUNCTION() {
  RTC_DCHECK(CurrentThreadIsOwner());
#if defined(WEBRTC_WIN)
  LeaveCriticalSection(&crit_);
#elif defined(WEBRTC_POSIX)
# if RTC_DCRIT_IS_ON
  --recursion_count_;
  RTC_DCHECK(recursion_count_ >= 0);
  if (!recursion_count_)
    thread_ = 0;
# endif  // RTC_DCRIT_IS_ON

  pthread_mutex_unlock(&mutex_);
#else
# error Unsupported platform.
#endif
}

bool CriticalSection::CurrentThreadIsOwner() const {
#if defined(WEBRTC_WIN)
  // OwningThread has type HANDLE but actually contains the Thread ID:
  // http://stackoverflow.com/questions/12675301/why-is-the-owningthread-member-of-critical-section-of-type-handle-when-it-is-de
  // Converting through size_t avoids the VS 2015 warning C4312: conversion from
  // 'type1' to 'type2' of greater size
  return crit_.OwningThread ==
         reinterpret_cast<HANDLE>(static_cast<size_t>(GetCurrentThreadId()));
#elif defined(WEBRTC_POSIX)
# if RTC_DCRIT_IS_ON
  return IsThreadRefEqual(thread_, CurrentThreadRef());
# else
  return true;
# endif  // RTC_DCRIT_IS_ON
#else
# error Unsupported platform.
#endif
}

CritScope::CritScope(const CriticalSection* cs) : cs_(cs) { cs_->Enter(); }
CritScope::~CritScope() { cs_->Leave(); }

TryCritScope::TryCritScope(const CriticalSection* cs)
    : cs_(cs), locked_(cs->TryEnter()) {
  CS_DEBUG_CODE(lock_was_called_ = false);
  RTC_UNUSED(lock_was_called_);
}

TryCritScope::~TryCritScope() {
  CS_DEBUG_CODE(RTC_DCHECK(lock_was_called_));
  if (locked_)
    cs_->Leave();
}

bool TryCritScope::locked() const {
  CS_DEBUG_CODE(lock_was_called_ = true);
  return locked_;
}

void GlobalLockPod::Lock() {
#if !defined(WEBRTC_WIN)
  const struct timespec ts_null = {0, 0};
#endif

  while (AtomicOps::CompareAndSwap(&lock_acquired, 0, 1)) {
#if defined(WEBRTC_WIN)
    ::Sleep(0);
#else
    nanosleep(&ts_null, nullptr);
#endif
  }
}

void GlobalLockPod::Unlock() {
  int old_value = AtomicOps::CompareAndSwap(&lock_acquired, 1, 0);
  RTC_DCHECK_EQ(1, old_value) /* << "Unlock called without calling Lock first" */;
}

GlobalLock::GlobalLock() {
  lock_acquired = 0;
}

GlobalLockScope::GlobalLockScope(GlobalLockPod* lock)
    : lock_(lock) {
  lock_->Lock();
}

GlobalLockScope::~GlobalLockScope() {
  lock_->Unlock();
}

}  // namespace rtc
