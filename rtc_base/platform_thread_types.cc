/*
 *  Copyright (c) 2018 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "rtc_base/platform_thread_types.h"
#include "typedefs.h"  // NOLINT(build/include)

/* #if defined(WEBRTC_LINUX)
#include <sys/prctl.h>
#include <sys/syscall.h>
#endif */

namespace rtc {

PlatformThreadId CurrentThreadId() {
#if defined(WEBRTC_WIN)
  return GetCurrentThreadId();
#elif defined(WEBRTC_POSIX)
/* #if defined(WEBRTC_LINUX)
  return syscall(__NR_gettid);
#else */
  // TODO error: invalid cast from type ‘pthread_t’ {aka ‘long unsigned int’} to type ‘pid_t’ {aka ‘int’}
  // return reinterpret_cast<pid_t>(pthread_self());
  return static_cast<pid_t>(pthread_self());
/* #endif */
#endif  // defined(WEBRTC_WIN)
}

PlatformThreadRef CurrentThreadRef() {
#if defined(WEBRTC_WIN)
  return GetCurrentThreadId();
#elif defined(WEBRTC_POSIX)
  return pthread_self();
#endif
}

bool IsThreadRefEqual(const PlatformThreadRef& a, const PlatformThreadRef& b) {
#if defined(WEBRTC_WIN)
  return a == b;
#elif defined(WEBRTC_POSIX)
  return pthread_equal(a, b);
#endif
}

void SetCurrentThreadName(const char* name) {
#if defined(WEBRTC_WIN)
  struct {
    DWORD dwType;
    LPCSTR szName;
    DWORD dwThreadID;
    DWORD dwFlags;
  } threadname_info = {0x1000, name, static_cast<DWORD>(-1), 0};

  __try {
    ::RaiseException(0x406D1388, 0, sizeof(threadname_info) / sizeof(DWORD),
                     reinterpret_cast<ULONG_PTR*>(&threadname_info));
  } __except (EXCEPTION_EXECUTE_HANDLER) {  // NOLINT
  }
#elif defined(WEBRTC_POSIX)
/* #if defined(WEBRTC_LINUX)
  prctl(PR_SET_NAME, reinterpret_cast<unsigned long>(name));  // NOLINT
#else */
  RTC_UNUSED(name);
/* #endif */
#endif  // defined(WEBRTC_WIN)
}

PlatformTlsKey AllocTls()
{
  PlatformTlsKey key;
#if defined(WEBRTC_WIN)
  key = TlsAlloc();
#elif defined(WEBRTC_POSIX)
  pthread_key_create(&key, nullptr);
#endif  // defined(WEBRTC_WIN)
  return key;
}

void FreeTls(PlatformTlsKey key)
{
#if defined(WEBRTC_WIN)
  TlsFree(key);
#elif defined(WEBRTC_POSIX)
  pthread_key_delete(key);
#endif  // defined(WEBRTC_WIN)
}

void *GetTlsValue(PlatformTlsKey key)
{
#if defined(WEBRTC_WIN)
  return TlsGetValue(key);
#elif defined(WEBRTC_POSIX)
  return pthread_getspecific(key);
#endif  // defined(WEBRTC_WIN)
}

void SetTlsValue(PlatformTlsKey key, const void *value)
{
#if defined(WEBRTC_WIN)
  TlsSetValue(key, value);
#elif defined(WEBRTC_POSIX)
  pthread_setspecific(key, value);
#endif  // defined(WEBRTC_WIN)
}

bool ThreadSleep(int milliseconds)
{
#if defined(WEBRTC_WIN)
  ::Sleep(milliseconds);
  return true;
#else
  // POSIX has both a usleep() and a nanosleep(), but the former is deprecated,
  // so we use nanosleep() even though it has greater precision than necessary.
  struct timespec ts;
  ts.tv_sec = milliseconds / 1000;
  ts.tv_nsec = (milliseconds % 1000) * 1000000;
  // Normalize.
  if (ts.tv_nsec >= 1000000000) {
    ts.tv_sec++;
    ts.tv_nsec -= 1000000000;
  }
  int ret = nanosleep(&ts, nullptr);
  if (ret != 0) {
    /* RTC_LOG_ERR(LS_WARNING) << "nanosleep() returning early"; */
    return false;
  }
  return true;
#endif  // defined(WEBRTC_WIN)
}

void ThreadYield(void)
{
#if defined(WEBRTC_WIN)
    // Alertable sleep to permit RaiseFlag to run and update |stop_|.
    SleepEx(0, true); // ::Sleep(0);
#else
    static const struct timespec ts_null = {0, 0};
    nanosleep(&ts_null, nullptr);
#endif  // defined(WEBRTC_WIN)
}

}  // namespace rtc
