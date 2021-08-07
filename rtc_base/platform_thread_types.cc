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
#include <stdlib.h>

// namespace rtc {

// Information to pass to the new thread (what to run).
typedef struct ThrdStartWrapperArgs_ {
    ThrdStartFunction func_; // Pointer to the function to be executed.
    void *arg_;             // Function argument for the thread function.
} ThrdStartWrapperArgs;

#if defined(WEBRTC_WIN)
    static DWORD WINAPI ThrdStartWrapperFunc(LPVOID wrapper)
#elif defined(WEBRTC_POSIX)
    static void *ThrdStartWrapperFunc(void *wrapper)
#else
    static int ThrdStartWrapperFunc(void *wrapper)
#endif
{
#if defined(WEBRTC_WIN)
    // The GetLastError() function only returns valid results when it is called
    // after a Win32 API function that returns a "failed" result. A crash dump
    // contains the result from GetLastError() and to make sure it does not
    // falsely report a Windows error we call SetLastError here.
    ::SetLastError(ERROR_SUCCESS);
#endif

    ThrdStartFunction fun;
    void *arg;
    int  result;

    // Get thread startup information
    ThrdStartWrapperArgs *ti = (ThrdStartWrapperArgs *) wrapper;
    fun = ti->func_;
    arg = ti->arg_;

    // The thread is responsible for freeing the startup information
    free((void *)ti);

    // Call the actual client thread function
    result = fun(arg);

#if defined(WEBRTC_WIN)
    return (DWORD)result;
#elif defined(WEBRTC_POSIX)
    return (void *)(intptr_t)result;
#else
    return result;
#endif
}

int Rtc_ThrdCreate(Thrd *thr, ThrdStartFunction func, void *arg)
{
#if 0 // !(defined(WEBRTC_WIN) || defined(WEBRTC_POSIX))
    return thrd_create(thr, func, arg);
#endif

    // Fill out the thread startup information
    // (passed to the thread wrapper, which will eventually free it)
    ThrdStartWrapperArgs *ti = (ThrdStartWrapperArgs *)malloc(sizeof(ThrdStartWrapperArgs));
    if (ti == NULL) {
        return -1;
    }
    ti->func_ = func;
    ti->arg_ = arg;

    // Create the thread
#if defined(WEBRTC_WIN)
    // See bug 2902 for background on STACK_SIZE_PARAM_IS_A_RESERVATION.
    // Set the reserved stack stack size to 1M, which is the default on Windows
    // and Linux.
    *thr = /* :: */CreateThread(nullptr, 1024 * 1024, &ThrdStartWrapperFunc, (LPVOID) ti,
                                STACK_SIZE_PARAM_IS_A_RESERVATION, NULL);

#elif defined(WEBRTC_POSIX)
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    // Set the stack stack size to 1M.
    pthread_attr_setstacksize(&attr, 1024 * 1024);
    if (pthread_create(thr, &attr, &ThrdStartWrapperFunc, (void *)ti) != 0) {
        *thr = 0;
    }
    pthread_attr_destroy(&attr);
#else
    // Nothing
#endif

    // Did we fail to create the thread?
    if (!*thr) {
        free(ti);
        return -1;
    }

    return 0;
}

Thrd Rtc_ThrdCurrent(void)
{
#if defined(WEBRTC_WIN)
    return GetCurrentThread();
#elif defined(WEBRTC_POSIX)
    return pthread_self();
#else
    return thrd_current();
#endif
}

int Rtc_ThrdDetach(Thrd thr)
{
#if defined(WEBRTC_WIN)
    // https://stackoverflow.com/questions/12744324/how-to-detach-a-thread-on-windows-c#answer-12746081
    return CloseHandle(thr);
#elif defined(WEBRTC_POSIX)
    return pthread_detach(thr);
#else
    return thrd_detach(thr);
#endif
}

bool Rtc_ThrdEqual(Thrd thr0, Thrd thr1)
{
#if defined(WEBRTC_WIN)
    return GetThreadId(thr0) == GetThreadId(thr1);
#elif defined(WEBRTC_POSIX)
    return pthread_equal(thr0, thr1);
#else
    return thrd_equal(thr0, thr1);
#endif
}

RTC_NORETURN void Rtc_ThrdExit(void)
{
    const int result = 0;
#if defined(WEBRTC_WIN)
    ExitThread((DWORD)result);
#elif defined(WEBRTC_POSIX)
    pthread_exit((void *)(intptr_t)result);
#else
    thrd_exit(result);
#endif
}

int Rtc_ThrdJoin(Thrd thr)
{
#if defined(WEBRTC_WIN)
    WaitForSingleObject(thr, INFINITE);
    CloseHandle(thr);
    return 0;
#elif defined(WEBRTC_POSIX)
    return pthread_join(thr, NULL);
#else
    return thrd_join(thr, NULL);
#endif
}

bool Rtc_ThrdSleep(int milliseconds)
{
#if defined(WEBRTC_WIN)
    /* :: */Sleep(milliseconds);
    return true;
#else
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    // Normalize.
    if (ts.tv_nsec >= 1000000000) {
        ts.tv_sec++;
        ts.tv_nsec -= 1000000000;
    }
#if defined(WEBRTC_POSIX)
    // POSIX has both a usleep() and a nanosleep(), but the former is deprecated,
    // so we use nanosleep() even though it has greater precision than necessary.
    int ret = nanosleep(&ts, NULL);
    if (ret != 0) {
        /* RTC_LOG_ERR(LS_WARNING) << "nanosleep() returning early"; */
        return false;
    }
    return true;
#else
    return thrd_sleep(&ts, NULL);
#endif
#endif  // defined(WEBRTC_WIN)
}

void Rtc_ThrdYield(void)
{
#if defined(WEBRTC_WIN)
    // Alertable sleep to permit RaiseFlag to run and update |stop_|.
    SleepEx(0, true); // ::Sleep(0);
#elif defined(WEBRTC_POSIX)
    static const struct timespec ts_null = {0, 0};
    nanosleep(&ts_null, NULL);
#else
    return thrd_yield();
#endif
}

/* #if defined(WEBRTC_LINUX)
#include <sys/prctl.h>
#include <sys/syscall.h>
#endif */
void Rtc_ThrdSetName(const char *name)
{
#if 0
#if defined(WEBRTC_WIN)
    struct {
        DWORD dwType;
        LPCSTR szName;
        DWORD dwThreadID;
        DWORD dwFlags;
    } threadname_info = {0x1000, name, static_cast<DWORD>(-1), 0};

    __try {
        ::RaiseException(0x406D1388, 0, sizeof(threadname_info) / sizeof(DWORD),
                         reinterpret_cast<ULONG_PTR *>(&threadname_info));
    } __except (EXCEPTION_EXECUTE_HANDLER) {  // NOLINT
    }
#elif defined(WEBRTC_POSIX)
    /* #if defined(WEBRTC_LINUX)
      prctl(PR_SET_NAME, reinterpret_cast<unsigned long>(name));  // NOLINT
#else */
    /* #endif */
#else
    // Nothing
#endif  // defined(WEBRTC_WIN)
#endif
    RTC_UNUSED(name);
}

Tss Rtc_TssCreate(void)
{
    Tss key;
#if defined(WEBRTC_WIN)
    key = TlsAlloc();
#elif defined(WEBRTC_POSIX)
    pthread_key_create(&key, NULL);
#else
    tss_create(&key, NULL);
#endif
    return key;
}

void Rtc_TssDelete(Tss key)
{
#if defined(WEBRTC_WIN)
    TlsFree(key);
#elif defined(WEBRTC_POSIX)
    pthread_key_delete(key);
#else
    tss_delete(key);
#endif
}

void *Rtc_TssGet(Tss key)
{
#if defined(WEBRTC_WIN)
    return TlsGetValue(key);
#elif defined(WEBRTC_POSIX)
    return pthread_getspecific(key);
#else
    return tss_get(key);
#endif
}

int Rtc_TssSet(Tss key, void *val)
{
#if defined(WEBRTC_WIN)
    return TlsSetValue(key, val);
#elif defined(WEBRTC_POSIX)
    return pthread_setspecific(key, val);
#else
    return tss_set(key);
#endif
}

// }  // namespace rtc
