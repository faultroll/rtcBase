
// https://en.cppreference.com/w/c/thread
// https://github.com/tinycthread/tinycthread
// https://github.com/cdschreiber/c11
// https://github.com/zserge/pt

#ifndef _CTHREAD_H
#define _CTHREAD_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#if defined(__cplusplus)
extern "C" {
#endif

#if !defined(_USE_STD) && !defined(_USE_POSIX) && !defined(_USE_WIN) && !defined(_USE_NONE)
    #if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L) \
        && !defined(__STDC_NO_THREADS__)
        #define _USE_STD
    #elif defined(__GNUC__) || defined(__clang__) // defined(__linux__) || defined(__APPLE__)
        #define _USE_POSIX
        #ifndef _POSIX_C_SOURCE
            #define _POSIX_C_SOURCE 200809L
        #endif /* _POSIX_C_SOURCE */
    #elif defined(_MSC_VER) // defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
        #define _USE_WIN
    #else
        #define _USE_NONE
    #endif /* !defined(__STDC_NO_THREADS__) */
#endif /* !defined(_USE_STD) && ... */

#if defined(_USE_STD)
    // #include <time.h>
    // #include <stdnoreturn.h>
    #include <threads.h>
#else
    // #include "tinycthread.h"

    /**
     * timespec
     *
     **/
    #if defined(_USE_POSIX)
        #include <time.h>
        // #include <sys/time.h>
    #elif defined(_USE_WIN)
        #ifndef WIN32_LEAN_AND_MEAN
            #define WIN32_LEAN_AND_MEAN
        #endif /* WIN32_LEAN_AND_MEAN */
        #include <windows.h>
        #include <mmsystem.h>
        #include <sys/timeb.h>
        #ifndef TIME_UTC
            #define TIME_UTC 1
        #endif /* TIME_UTC */
        #ifndef _STRUCT_TIMESPEC
            struct timespec {
                time_t tv_sec;
                long   tv_nsec;
            };
            #define _STRUCT_TIMESPEC
        #endif /* _STRUCT_TIMESPEC */
    #elif defined(_USE_NONE)
    #else
        #error "timespec: unknown branch in defined(_USE_POSIX)"
    #endif /* defined(_USE_POSIX) */
    // helper
    static const struct timespec ts_null = {0, 0};

    /**
     * stdnoreturn
     * 
     **/
    #if defined(_USE_POSIX)
        #ifndef noreturn
            // something is wrong with |__cplusplus|
            // error: '__noreturn__' was not declared in this scope
            #if defined(__cplusplus)
                #define noreturn 
            #else
                #define noreturn __attribute__ ((__noreturn__))
            #endif /* defined(__cplusplus) */
        #endif /* noreturn */
    #elif defined(_USE_WIN)
        #ifndef noreturn
            #define noreturn __declspec(noreturn)
        #endif /* noreturn */
    #elif defined(_USE_NONE)
        #ifndef noreturn
            #define noreturn 
        #endif /* noreturn */
    #else
        #error "stdnoreturn: unknown branch in defined(_USE_POSIX)"
    #endif /* defined(_USE_POSIX) */

    /**
     * thrd
     *
     **/
    // Any thread that is started with the |thrd_create| function must be
    // started through a function of this type.
    typedef int (*thrd_start_t)(void *arg);
    static int thrd_start_wrapper_func_helper(void *wrapper);
    #if defined(_USE_POSIX)
        #include <pthread.h>
        #include <unistd.h>
        #include <sched.h>
        // Function return values
        enum {
            thrd_success    = 0,  // The requested operation succeeded
            thrd_nomem      = -4, // The requested operation failed because it was unable to allocate memory
            thrd_timedout   = -3, // The time specified in the call was reached without acquiring the requested resource
            thrd_busy       = -2, // The requested operation failed because a tesource requested by a test and return function is already in use
            thrd_error      = -1, // The requested operation failed
        };
        // just set |thrd_success| to different in different platform
        // #define thrd_check_return(result) do { return ((0 == (result)) ? thrd_success : thrd_error); } while(0)
        typedef pthread_t thrd_t;
        static const thrd_t thrd_null = (thrd_t)(0);
        static void *thrd_start_wrapper_func(void *wrapper)
        {
            return (void *)(intptr_t)thrd_start_wrapper_func_helper(wrapper);
        }
        // Create the thread
        static thrd_t thrd_create_helper(void *ti)
        {
            thrd_t thr;
            pthread_attr_t attr;
            pthread_attr_init(&attr);
            // Set the stack stack size to 1M.
            pthread_attr_setstacksize(&attr, 1024 * 1024);
            if (pthread_create(&thr, &attr, &thrd_start_wrapper_func, (void *)ti) != 0) {
                thr = thrd_null;
            }
            pthread_attr_destroy(&attr);

            return thr;
        }
        // Retrieves a reference to the current thread. On Windows, this is the same
        // as CurrentThreadId. On other platforms it's the pthread_t returned by
        // pthread_self().
        static inline thrd_t thrd_current(void)
        {
            return pthread_self();
        }
        // Dispose of any resources allocated to the thread when that thread exits
        static inline int thrd_detach(thrd_t thr)
        {
            return pthread_detach(thr);
        }
        // Compares two thread identifiers for equality.
        // Non-zero value if thr0 and thr1 refer to the same value, ​0​ otherwise.
        static inline int thrd_equal(thrd_t thr0, thrd_t thr1)
        {
            return (0 == pthread_equal(thr0, thr1)) ? 1 : 0;
        }
        // Terminate execution of the calling thread
        static inline noreturn void thrd_exit(int res)
        {
            pthread_exit((void *)(intptr_t)res);
        }
        // Wait for a thread to terminate
        static inline int thrd_join(thrd_t thr, int *res)
        {
            void *pres;
            if (pthread_join(thr, &pres) != 0) {
                return thrd_error;
            }
            if (res != NULL) {
                *res = (int)(intptr_t)pres;
            }
            return thrd_success;
        }
        // ​0​ on successful sleep, -1 if a signal occurred, other negative value if an error occurred.
        static inline int thrd_sleep(const struct timespec *duration, struct timespec *remaining)
        {
            return nanosleep(duration, remaining);
        }
        // Yield execution to another thread. Permit other threads to run,
        // even if current thread would ordinarily continue to run.
        static inline void thrd_yield(void)
        {
            sched_yield(); // nanosleep(&ts_null, NULL);
        }
    #elif defined(_USE_WIN)
        // Include winsock2.h before including <windows.h> to maintain consistency with
        // win32.h.  We can't include win32.h directly here since it pulls in
        // headers such as basictypes.h which causes problems in Chromium where webrtc
        // exists as two separate projects, webrtc and libjingle.
        #include <winsock2.h>
        #include <windows.h>
        #include <sal.h>  // must come after windows headers.
        // Function return values
        enum {
            thrd_success    = ERROR_SUCCESS /* 1 */,  // The requested operation succeeded
            thrd_nomem      = -4, // The requested operation failed because it was unable to allocate memory
            thrd_timedout   = -3, // The time specified in the call was reached without acquiring the requested resource
            thrd_busy       = -2, // The requested operation failed because a tesource requested by a test and return function is already in use
            thrd_error      = -1, // The requested operation failed
        };
        // #define thrd_check_return(result) do { return (((result) != 0) ? thrd_success : thrd_error); } while(0)
        typedef HANDLE thrd_t;
        static const thrd_t thrd_null = (thrd_t)(0);
        static DWORD WINAPI thrd_start_wrapper_func(LPVOID wrapper)
        {
            // The GetLastError() function only returns valid results when it is called
            // after a Win32 API function that returns a "failed" result. A crash dump
            // contains the result from GetLastError() and to make sure it does not
            // falsely report a Windows error we call SetLastError here.
            SetLastError(ERROR_SUCCESS);

            return (DWORD)thrd_start_wrapper_func_helper(wrapper);
        }
        static thrd_t thrd_create_helper(void *ti)
        {
            // Create the thread
            // See bug 2902 for background on STACK_SIZE_PARAM_IS_A_RESERVATION.
            // Set the reserved stack stack size to 1M, which is the default on Windows
            // and Linux.
            return CreateThread(NULL, 1024 * 1024, &thrd_start_wrapper_func, (LPVOID) ti,
                                STACK_SIZE_PARAM_IS_A_RESERVATION, NULL);
        }
        static inline thrd_t thrd_current(void)
        {
            return GetCurrentThread();
        }
        static inline int thrd_detach(thrd_t thr)
        {
            // https://stackoverflow.com/questions/12744324/how-to-detach-a-thread-on-windows-c#answer-12746081
            return CloseHandle(thr);
        }
        static inline int thrd_equal(thrd_t thr0, thrd_t thr1)
        {
            /* // OwningThread has type HANDLE but actually contains the Thread ID:
            // http://stackoverflow.com/questions/12675301/why-is-the-owningthread-member-of-critical-section-of-type-handle-when-it-is-de
            // Converting through size_t avoids the VS 2015 warning C4312: conversion from
            // 'type1' to 'type2' of greater size
            return crit_.OwningThread ==
                reinterpret_cast<HANDLE>(static_cast<size_t>(GetCurrentThreadId())); */
            return (GetThreadId(thr0) == GetThreadId(thr1)) ? 1 : 0;
        }
        static inline noreturn void thrd_exit(int res)
        {
            ExitThread((DWORD)res);
        }
        static inline int thrd_join(thrd_t thr, int *res)
        {
            DWORD dwRes;
            if (WaitForSingleObject(thr, INFINITE) == WAIT_FAILED) {
                return thrd_error;
            }
            if (res != NULL) {
                if (GetExitCodeThread(thr, &dwRes) == 0) {
                    return thrd_error;
                }
                *res = (int)dwRes;
            }
            CloseHandle(thr);
            return thrd_success;
        }
        static inline int thrd_sleep(const struct timespec *duration, struct timespec *remaining)
        {
            SleepEx((DWORD)(duration->tv_sec * 1000 +
                    duration->tv_nsec / 1000000 +
                    (((duration->tv_nsec % 1000000) == 0) ? 0 : 1)),
                    TRUE);
            return 0;
        }
        static inline void thrd_yield(void)
        {
            // Alertable sleep to permit RaiseFlag to run and update |stop_|.
            SleepEx(0, true); // Sleep(0);
        }
    #elif defined(_USE_NONE)
        // use coroutine
        // rtos |taskDelay| ...
        // #include "pt.h"
    #else
        #error "thrd: unknown branch in defined(_USE_POSIX)"
    #endif /* defined(_USE_POSIX) */
    // Information to pass to the new thread (what to run).
    typedef struct {
        thrd_start_t func_; // Pointer to the function to be executed.
        void *arg_;         // Function argument for the thread function.
    } thrd_start_wrapper_args_t;
    static int thrd_start_wrapper_func_helper(void *wrapper)
    {
        thrd_start_t fun;
        void *arg;
        int  result = thrd_error;

        // Get thread startup information
        thrd_start_wrapper_args_t *ti = (thrd_start_wrapper_args_t *) wrapper;
        fun = ti->func_;
        arg = ti->arg_;

        // The thread is responsible for freeing the startup information
        free((void *)ti);

        // Call the actual client thread function
        if (fun)
            result = fun(arg);

        return result;
    }
    // Create a new thread
    static inline int thrd_create(thrd_t *thr, thrd_start_t func, void *arg)
    {
        // Fill out the thread startup information
        // (passed to the thread wrapper, which will eventually free it)
        thrd_start_wrapper_args_t *ti = (thrd_start_wrapper_args_t *)malloc(sizeof(thrd_start_wrapper_args_t));
        if (ti == NULL) {
            return thrd_nomem;
        }
        ti->func_ = func;
        ti->arg_ = arg;

        *thr = thrd_create_helper((void *)ti);

        // Did we fail to create the thread?
        if (thrd_null == *thr) {
            free(ti);
            return thrd_error;
        }

        return thrd_success;
    }

    /**
     * tss
     * 
     **/
    #if defined(_USE_POSIX)
        // no need to check inner re-defines
        // #ifndef thread_local
        #define thread_local __thread
        // #endif /* thread_local */
        // Destructor function for a TsS
        typedef void (*tss_dtor_t)(void *val);
        // TLs(Thread Local-storage) or TsS(Thread-specific Storage)
        typedef pthread_key_t tss_t;
        static const tss_t tss_null = (tss_t)(0);
        // Create a TsS
        static inline int tss_create(tss_t *key, tss_dtor_t dtor)
        {
            return pthread_key_create(key, dtor);
        }
        // Delete a TsS
        static inline void tss_delete(tss_t key)
        {
            pthread_key_delete(key);
        }
        // Get the value for a TsS
        static inline void *tss_get(tss_t key)
        {
            return pthread_getspecific(key);
        }
        // Set the value for a TsS
        static inline int tss_set(tss_t key, void *val)
        {
            return pthread_setspecific(key, val);
        }
    #elif defined(_USE_WIN)
        // #ifndef thread_local
        #define thread_local __declspec(thread)
        // #endif /* thread_local */
        typedef DWORD tss_t;
        static const tss_t tss_null = (tss_t)(TLS_OUT_OF_INDEXES); 
        static inline int tss_create(tss_t *key, tss_dtor_t dtor)
        {
            if ((*key = TlsAlloc()) == tss_null) {
                return thrd_error
            } else {
                return thrd_success;
            }
        }
        static inline void tss_delete(tss_t key)
        {
            TlsFree(key);
        }
        static inline void *tss_get(tss_t key)
        {
            return TlsGetValue(key);
        }
        static inline int tss_set(tss_t key, void *val)
        {
            return TlsSetValue(key, val);
        }
    #elif defined(_USE_NONE)

    #else
        #error "tss: unknown branch in defined(_USE_POSIX)"
    #endif /* defined(_USE_POSIX) */

    /**
     * mtx
     * 
     **/
    // Bit-mask
    enum {
        mtx_plain = 0/* unspecified */, // default
        mtx_recursive = 1 << 0/* unspecified */, // recursive
        mtx_timed = 1 << 1/* unspecified */, // supports timeout
    };
    #if defined(_USE_POSIX)
        // Mutex
        typedef pthread_mutex_t mtx_t;
        // static const mtx_t mtx_null = (mtx_t)(0);
        // Create a mutex object
        static inline int mtx_init(mtx_t *mtx, int type)
        {
            int result;
            pthread_mutexattr_t mutex_attribute;
            pthread_mutexattr_init(&mutex_attribute);
            if (type & mtx_recursive) {
                pthread_mutexattr_settype(&mutex_attribute, PTHREAD_MUTEX_RECURSIVE);
            }
            result = pthread_mutex_init(mtx, &mutex_attribute);
            pthread_mutexattr_destroy(&mutex_attribute);
            return result;
        }
        // Release any resources used by the given mutex
        static inline void mtx_destroy(mtx_t *mtx)
        {
            pthread_mutex_destroy(mtx);
        }
        // Lock the given mutex
        // Blocks until the given mutex can be locked. If the mutex is non-recursive, and
        // the calling thread already has a lock on the mutex, this call will block forever.
        static inline int mtx_lock(mtx_t *mtx)
        {
            return pthread_mutex_lock(mtx);
        }
        // Lock the given mutex, or block until a specific point in time.
        // Blocks until either the given mutex can be locked, or the specified TIME_UTC
        // based time.
        static inline int mtx_timedlock(mtx_t *mtx, const struct timespec *ts)
        {
            return pthread_mutex_timedlock(mtx, ts);
        }
        // Try to lock the given mutex
        // The specified mutex shall support either test and return or timeout. If the
        // mutex is already locked, the function returns without blocking.
        static inline int mtx_trylock(mtx_t *mtx)
        {
            return pthread_mutex_trylock(mtx);
        }
        // Unlock the given mutex
        static inline int mtx_unlock(mtx_t *mtx)
        {
            return pthread_mutex_unlock(mtx);
        }
    #elif defined(_USE_WIN)
        typedef CRITICAL_SECTION mtx_t;
        // static const mtx_t mtx_null = (mtx_t)(0);
        static inline int mtx_init(mtx_t *mtx, int type)
        {
            return InitializeCriticalSection(mtx);
        }
        static inline void mtx_destroy(mtx_t *mtx)
        {
            DeleteCriticalSection(mtx);
        }
        static inline int mtx_lock(mtx_t *mtx)
        {
            return EnterCriticalSection(mtx);
        }
        static inline int mtx_timedlock(mtx_t *mtx, const struct timespec *ts)
        {
            return thrd_error;
        }
        static inline int mtx_trylock(mtx_t *mtx)
        {
            return TryEnterCriticalSection(mtx);
        }
        static inline int mtx_unlock(mtx_t *mtx)
        {
            return LeaveCriticalSection(mtx);
        }
    #elif defined(_USE_NONE)

    #else
        #error "mtx: unknown branch in defined(_USE_POSIX)"
    #endif /* defined(_USE_POSIX) */

    /**
     * cnd
     * 
     **/
    #if defined(_USE_POSIX)
        // Condition variable
        typedef pthread_cond_t cnd_t;
        // Create a condition variable object
        static inline int cnd_init(cnd_t *cond)
        {
            return pthread_cond_init(cond, NULL);
        }
        // Release any resources used by the given condition variable
        static inline void cnd_destroy(cnd_t *cond)
        {
            pthread_cond_destroy(cond);
        }
        // Signal a condition variable
        // Unblocks one of the threads that are blocked on the given condition variable
        // at the time of the call. If no threads are blocked on the condition variable
        // at the time of the call, the function does nothing and return success.
        static inline int cnd_signal(cnd_t *cond)
        {
            return pthread_cond_signal(cond);
        }
        // Broadcast a condition variable.
        // Unblocks all of the threads that are blocked on the given condition variable
        // at the time of the call. If no threads are blocked on the condition variable
        // at the time of the call, the function does nothing and return success.
        static inline int cnd_broadcast(cnd_t *cond)
        {
            return pthread_cond_broadcast(cond);
        }
        // Wait for a condition variable to become signaled.
        // The function atomically unlocks the given mutex and endeavors to block until
        // the given condition variable is signaled by a call to cnd_signal or to cnd_broadcast.
        // When the calling thread becomes unblocked it locks the mutex before it returns.
        static inline int cnd_wait(cnd_t *cond, mtx_t *mtx)
        {
            return pthread_cond_wait(cond, mtx);
        }
        // Wait for a condition variable to become signaled.
        // The function atomically unlocks the given mutex and endeavors to block until
        // the given condition variable is signaled by a call to cnd_signal or to
        // cnd_broadcast, or until after the specified time. When the calling thread
        // becomes unblocked it locks the mutex before it returns.
        static inline int cnd_timedwait(cnd_t *cond, mtx_t *mtx, const struct timespec *ts)
        {
            return pthread_cond_timedwait(cond, mtx, ts);
        }
    #elif defined(_USE_WIN)
        enum {
            cnd_event_signal = 0, // signal
            cnd_event_broadcast, // broadcast
            cnd_event_num,
        };
        typedef struct {
            HANDLE events_[cnd_event_num];  // Signal and broadcast event HANDLEs.
            size_t waiter_count_;           // Count of the number of waiters.
            CRITICAL_SECTION waiter_mutex_; // Serialize access to |waiter_count_|
        } cnd_t;
        static inline int cnd_init(cnd_t *cond)
        {
            cond->waiter_count_ = 0;
            // Init critical section
            InitializeCriticalSection(&cond->waiter_mutex_);
            // Init events
            cond->events_[cnd_event_signal] = CreateEvent(NULL, false, false, NULL);
            if (cond->events_[cnd_event_signal] == NULL) {
                cond->events_[cnd_event_broadcast] = NULL;
                return thrd_error;
            }
            cond->events_[cnd_event_broadcast] = CreateEvent(NULL, true, false, NULL);
            if (cond->events_[cnd_event_broadcast] == NULL) {
                CloseHandle(cond->events_[cnd_event_signal]);
                cond->events_[cnd_event_signal] = NULL;
                return thrd_error;
            }
            return thrd_success;
        }
        static inline void cnd_destroy(cnd_t *cond)
        {
            if (cond->events_[cnd_event_signal] != NULL) {
                CloseHandle(cond->events_[cnd_event_signal]);
            }
            if (cond->events_[cnd_event_broadcast] != NULL) {
                CloseHandle(cond->events_[cnd_event_broadcast]);
            }
            DeleteCriticalSection(&cond->waiter_mutex_);
        }
        static inline int cnd_signal(cnd_t *cond)
        {
            return thrd_error;
        }
        static inline int cnd_broadcast(cnd_t *cond)
        {
            return thrd_error;
        }
        static inline int cnd_wait(cnd_t *cond, mtx_t *mtx)
        {
            return thrd_error;
        }
        static inline int cnd_timedwait(cnd_t *cond, mtx_t *mtx, const struct timespec *ts)
        {
            return thrd_error;
        }
    #elif defined(_USE_NONE)

    #else
        #error "cnd: unknown branch in defined(_USE_POSIX)"
    #endif /* defined(_USE_POSIX) */

    /**
     * once
     * 
     **/
    #if defined(_USE_POSIX)
        #define once_flag pthread_once_t
        #define ONCE_FLAG_INIT PTHREAD_ONCE_INIT
        #define call_once(flag,func) pthread_once(flag,func)
    #elif defined(_USE_WIN)
        typedef struct {
            volatile LONG status;
            CRITICAL_SECTION lock;
        } once_flag;
        #define ONCE_FLAG_INIT {0,}
        void call_once(once_flag *flag, void (*func)(void))
        {
            /* The idea here is that we use a spin lock (via the
            InterlockedCompareExchange function) to restrict access to the
            critical section until we have initialized it, then we use the
            critical section to block until the callback has completed
            execution. */
            while (flag->status < 3) {
                switch (flag->status) {
                    case 0:
                        if (InterlockedCompareExchange(&(flag->status), 1, 0) == 0) {
                            InitializeCriticalSection(&(flag->lock));
                            EnterCriticalSection(&(flag->lock));
                            flag->status = 2;
                            func();
                            flag->status = 3;
                            LeaveCriticalSection(&(flag->lock));
                            return;
                        }
                        break;
                    case 1:
                        break;
                    case 2:
                        EnterCriticalSection(&(flag->lock));
                        LeaveCriticalSection(&(flag->lock));
                        break;
                }
            }
        }
    #elif defined(_USE_NONE)

    #else
        #error "once: unknown branch in defined(_USE_POSIX)"
    #endif /* defined(_USE_POSIX) */

#endif /* defined(_USE_STD) */

// force undef current defines 
#undef _USE_STD
#undef _USE_POSIX
#undef _USE_WIN
#undef _USE_NONE

#if defined(__cplusplus)
}
#endif

#endif /* _CTHREAD_H */
