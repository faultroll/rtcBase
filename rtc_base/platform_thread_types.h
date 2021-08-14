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

// Mimic c11 <threads.h> from https://github.com/tinycthread/tinycthread

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
    #warning "Should define either WEBRTC_WIN or WEBRTC_POSIX."
    #include <threads.h>  // c11 thread
#endif

// namespace rtc {

// Function return values
enum {
    kThrdSuccess    = 0,  // The requested operation succeeded
    kThrdNomem      = -4, // The requested operation failed because it was unable to allocate memory
    kThrdTimedout   = -3, // The time specified in the call was reached without acquiring the requested resource
    kThrdBusy       = -2, // The requested operation failed because a tesource requested by a test and return function is already in use
    kThrdError      = -1, // The requested operation failed
};

// Thread
#if defined(WEBRTC_WIN)
    typedef HANDLE Thrd;
#elif defined(WEBRTC_POSIX)
    typedef pthread_t Thrd;
#else // c11 <threads.h>
    typedef thrd_t Thrd;
#endif
static const Thrd kNullThrd = static_cast<Thrd>(0);

// Any thread that is started with the |rtc_ThrdCreate| function must be
// started through a function of this type.
typedef int (*ThrdStartFunction)(void *arg);

// Create a new thread
int rtc_ThrdCreate(Thrd *thr, ThrdStartFunction func, void *arg);

// Retrieves a reference to the current thread. On Windows, this is the same
// as CurrentThreadId. On other platforms it's the pthread_t returned by
// pthread_self().
Thrd rtc_ThrdCurrent(void);

// Dispose of any resources allocated to the thread when that thread exits
// int rtc_ThrdDetach(Thrd thr);

// Compares two thread identifiers for equality.
bool rtc_ThrdEqual(Thrd thr0, Thrd thr1);

// Terminate execution of the calling thread
// RTC_NORETURN void rtc_ThrdExit(void);

// Wait for a thread to terminate
int rtc_ThrdJoin(Thrd thr);

// Sleeps the calling thread for the specified number of milliseconds, during
// which time no processing is performed. Returns false if sleeping was
// interrupted by a signal (POSIX only).
bool rtc_ThrdSleep(int milliseconds);

// Yield execution to another thread. Permit other threads to run,
// even if current thread would ordinarily continue to run.
void rtc_ThrdYield(void);

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
bool rtc_ThrdSetPrio(Thrd thr, ThrdPrio prio);

// Sets the current thread name.
void rtc_ThrdSetName(const char *name);

// TLs(Thread Local-storage) or TsS(Thread-specific Storage)
#if defined(WEBRTC_WIN)
    typedef DWORD Tss;
#elif defined(WEBRTC_POSIX)
    typedef pthread_key_t Tss;
#else // c11 <threads.h>
    typedef tss_t Tss;
#endif
// static const Tss kNullTss = static_cast<Tss>(0);

// Destructor function for a TsS
// typedef void (*TssDtorFunction)(void *val);

// Create a TsS
// int rtc_TssCreate(Tss *key, TssDtorFunction dtor);
Tss rtc_TssCreate(void);

// Delete a TsS
void rtc_TssDelete(Tss key);

// Get the value for a TsS
void *rtc_TssGet(Tss key);

// Set the value for a TsS
int rtc_TssSet(Tss key, void *val);

// Mutex
#if defined(WEBRTC_WIN)
    typedef CRITICAL_SECTION Mtx;
#elif defined(WEBRTC_POSIX)
    typedef pthread_mutex_t Mtx;
#else // c11 <threads.h>
    typedef mtx_t Mtx;
#endif
// static const Mtx kNullMtx = static_cast<Mtx>(0);

// Create a mutex object
int rtc_MtxInit(Mtx *mtx);

// Release any resources used by the given mutex
void rtc_MtxDestroy(Mtx *mtx);

// Lock the given mutex
// Blocks until the given mutex can be locked. If the mutex is non-recursive, and
// the calling thread already has a lock on the mutex, this call will block forever.
int rtc_MtxLock(Mtx *mtx);

// Lock the given mutex, or block until a specific point in time.
// Blocks until either the given mutex can be locked, or the specified TIME_UTC
// based time.
// int rtc_MtxTimedLock(Mtx *mtx, int milliseconds);

// Try to lock the given mutex
// The specified mutex shall support either test and return or timeout. If the
// mutex is already locked, the function returns without blocking.
int rtc_MtxTryLock(Mtx *mtx);

// Unlock the given mutex
int rtc_MtxUnlock(Mtx *mtx);

#if 0 // Webrtc is using |Event|(like Semaphore) instead of |Condition|
// Condition variable
#if defined(WEBRTC_WIN)
enum {
    kCndEventToOne, // signal
    kCndEventToAll, // broadcast
    kCndEventBUTT,
};
typedef struct Cnd_ {
    HANDLE events_[kCndEventButt];  // Signal and broadcast event HANDLEs.
    size_t waiter_count_;           // Count of the number of waiters.
    Mtx waiter_mutex_;              // Serialize access to |waiter_count_|
} Cnd;
#elif defined(WEBRTC_POSIX)
typedef pthread_cond_t Cnd;
#else // c11 <threads.h>
typedef cnd_t Cnd;
#endif

// Create a condition variable object
int rtc_CndInit(Cnd *cond);

// Release any resources used by the given condition variable
void rtc_CndDestroy(Cnd *cond);

// Signal a condition variable
// Unblocks one of the threads that are blocked on the given condition variable
// at the time of the call. If no threads are blocked on the condition variable
// at the time of the call, the function does nothing and return success.
// int rtc_CndSignal(Cnd *cond);

// Broadcast a condition variable.
// Unblocks all of the threads that are blocked on the given condition variable
// at the time of the call. If no threads are blocked on the condition variable
// at the time of the call, the function does nothing and return success.
// int rtc_CndBroadcast(Cnd *cond);

// Wait for a condition variable to become signaled.
// The function atomically unlocks the given mutex and endeavors to block until
// the given condition variable is signaled by a call to rtc_Cndsignal or to rtc_Cndbroadcast.
// When the calling thread becomes unblocked it locks the mutex before it returns.
// int rtc_CndWait(Cnd *cond, Mtx *mtx);

// Wait for a condition variable to become signaled.
// The function atomically unlocks the given mutex and endeavors to block until
// the given condition variable is signaled by a call to rtc_Cndsignal or to
// rtc_Cndbroadcast, or until after the specified time. When the calling thread
// becomes unblocked it locks the mutex before it returns.
int rtc_CndTimedWait(Cnd *cond, Mtx *mtx, int milliseconds);
#endif

// }  // namespace rtc

#endif  // RTC_BASE_PLATFORM_THREAD_TYPES_H_
