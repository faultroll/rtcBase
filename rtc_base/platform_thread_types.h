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
    #include <winsock2.h>
    #include <windows.h>
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

// Thread start function.
// Any thread that is started with the |Rtc_ThrdCreate| function must be
// started through a function of this type.
typedef int (*ThrdStartFunction)(void *arg);

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

// }  // namespace rtc

#endif  // RTC_BASE_PLATFORM_THREAD_TYPES_H_
