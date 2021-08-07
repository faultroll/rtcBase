/*
 *  Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef RTC_BASE_PLATFORM_THREAD_TYPES_H_
#define RTC_BASE_PLATFORM_THREAD_TYPES_H_

// mimic c11 <threads.h> from https://github.com/tinycthread/tinycthread

#if defined(WEBRTC_WIN)
    // Include winsock2.h before including <windows.h> to maintain consistency with
    // win32.h.  We can't include win32.h directly here since it pulls in
    // headers such as basictypes.h which causes problems in Chromium where webrtc
    // exists as two separate projects, webrtc and libjingle.
    #include <winsock2.h>
    #include <windows.h>
    #include <sal.h>  // must come after windows headers.
#elif defined(WEBRTC_POSIX)
    #include <pthread.h>
    #include <unistd.h>
#else
    #include <threads.h>  // c11 thread
#endif

// namespace rtc {

// Thread
#if defined(WEBRTC_WIN)
    typedef HANDLE Thrd;
#elif defined(WEBRTC_POSIX)
    typedef pthread_t Thrd;
#else // c11 <threads.h>
    typedef thrd_t Thrd;
#endif

// Any thread that is started with the |Rtc_ThrdCreate| function must be
// started through a function of this type.
typedef int (*ThrdStartFunction)(void *arg);

// Create a new thread
int Rtc_ThrdCreate(Thrd *thr, ThrdStartFunction func, void *arg);

// Retrieves a reference to the current thread. On Windows, this is the same
// as CurrentThreadId. On other platforms it's the pthread_t returned by
// pthread_self().
Thrd Rtc_ThrdCurrent(void);

// Dispose of any resources allocated to the thread when that thread exits
// int Rtc_ThrdDetach(Thrd thr);

// Compares two thread identifiers for equality.
bool Rtc_ThrdEqual(Thrd thr0, Thrd thr1);

// Terminate execution of the calling thread
// RTC_NORETURN void Rtc_ThrdExit(void);

// Wait for a thread to terminate
int Rtc_ThrdJoin(Thrd thr);

// Sleeps the calling thread for the specified number of milliseconds, during
// which time no processing is performed. Returns false if sleeping was
// interrupted by a signal (POSIX only).
bool Rtc_ThrdSleep(int milliseconds);

// Yield execution to another thread. Permit other threads to run,
// even if current thread would ordinarily continue to run.
void Rtc_ThrdYield(void);

typedef enum ThrdPrio_ {
#ifdef WEBRTC_WIN
    kLowPrio = THREAD_PRIORITY_BELOW_NORMAL,
    kNormalPrio = THREAD_PRIORITY_NORMAL,
    kHighPrio = THREAD_PRIORITY_ABOVE_NORMAL,
    kHighestPrio = THREAD_PRIORITY_HIGHEST,
    kRealtimePrio = THREAD_PRIORITY_TIME_CRITICAL,
#else // #elif defined(WEBRTC_POSIX)
    kLowPrio = 1,
    kNormalPrio = 2,
    kHighPrio = 3,
    kHighestPrio = 4,
    kRealtimePrio = 5,
#endif
} ThrdPrio;

// Set the priority of the thread. Must be called when thread is running.
bool Rtc_ThrdSetPrio(Thrd thr, ThrdPrio prio);

// Sets the current thread name.
void Rtc_ThrdSetName(const char *name);

// TLs(Thread Local-storage) or TsS(Thread-specific Storage)
#if defined(WEBRTC_WIN)
    typedef DWORD Tss;
#elif defined(WEBRTC_POSIX)
    typedef pthread_key_t Tss;
#else // c11 <threads.h>
    typedef tss_t Tss;
#endif

// Destructor function for a TsS
// typedef void (*TssDtorFunction)(void *val);

// Create a TsS
// int Rtc_TssCreate(Tss *key, TssDtorFunction dtor);
Tss Rtc_TssCreate(void);

// Delete a TsS
void Rtc_TssDelete(Tss key);

// Get the value for a TsS
void *Rtc_TssGet(Tss key);

// Set the value for a TsS
int Rtc_TssSet(Tss key, void *val);

// Mutex
#if defined(_TTHREAD_WIN32_)
    typedef CRITICAL_SECTION Mtx;
#elif defined(WEBRTC_POSIX)
    typedef pthread_mutex_t Mtx;
#else // c11 <threads.h>
    typedef mtx_t Mtx;
#endif

// Create a mutex object
int Rtc_MtxInit(Mtx *mtx);

// Release any resources used by the given mutex
void Rtc_MtxDestroy(Mtx *mtx);

// Lock the given mutex
// Blocks until the given mutex can be locked. If the mutex is non-recursive, and
// the calling thread already has a lock on the mutex, this call will block forever.
int Rtc_MtxLock(Mtx *mtx);

// Lock the given mutex, or block until a specific point in time.
// Blocks until either the given mutex can be locked, or the specified TIME_UTC
// based time.
// int Rtc_MtxTimedLock(mtx_t *mtx, int milliseconds);

// Try to lock the given mutex
// The specified mutex shall support either test and return or timeout. If the
// mutex is already locked, the function returns without blocking.
int Rtc_MtxTryLock(Mtx *mtx);

// Unlock the given mutex
int Rtc_MtxUnlock(Mtx *mtx);

// }  // namespace rtc

#endif  // RTC_BASE_PLATFORM_THREAD_TYPES_H_
