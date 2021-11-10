
// https://en.cppreference.com/w/c/chrono
// https://github.com/open-webrtc-toolkit/owt-deps-webrtc
// https://github.com/cdschreiber/c11

#ifndef _CTIME_H
#define _CTIME_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

#if !defined(_USE_STD) && !defined(_USE_POSIX) && !defined(_USE_WIN) && !defined(_USE_NONE)
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
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
#endif /* __STDC_VERSION__ >= 201112L */
#endif /* !defined(_USE_STD) && ... */

#if defined(_USE_STD)
    #include <time.h>
#else
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
#endif /* defined(_USE_STD) */

// helper
#define MS_PER_SEC  INT64_C(1000)
#define US_PER_SEC  INT64_C(1000000)
#define NS_PER_SEC  INT64_C(1000000000)
#define US_PER_MS   INT64_C(1000)       // (US_PER_SEC / MS_PER_SEC)
#define NS_PER_MS   INT64_C(1000000)    // (NS_PER_SEC / MS_PER_SEC)
#define NS_PER_US   INT64_C(1000)       // (NS_PER_SEC / US_PER_SEC)
static inline void timespec_normalize(struct timespec *ts)
{
    if (ts->tv_nsec >= NS_PER_SEC) {
        ts->tv_sec  += ts->tv_nsec / NS_PER_SEC;
        ts->tv_nsec = ts->tv_nsec % NS_PER_SEC;
        /* ts->tv_sec++;
        ts->tv_nsec -= NS_PER_SEC; */
    }
}
// Convert milliseconds to timespec
static inline void timespec_from_millisec(struct timespec *ts, int64_t milliseconds)
{
    ts->tv_sec  = milliseconds / MS_PER_SEC;
    ts->tv_nsec = (milliseconds % MS_PER_SEC) * NS_PER_MS;
    timespec_normalize(ts);
}
// Convert timespec to milliseconds
static inline int64_t timespec_to_millisec(struct timespec *ts)
{
    return ((int64_t)(ts->tv_sec) * MS_PER_SEC +
            (int64_t)(ts->tv_nsec) / NS_PER_MS);
}

#if defined(_USE_STD)
    static inline int timespec_get_systime(struct timespec *ts)
    {
        // how to get systick?
        return timespec_get(ts, TIME_UTC);
    }
    static inline int timespec_get_utctime(struct timespec *ts)
    {
        return timespec_get(ts, TIME_UTC);
    }
    // static inline int timespec_get(struct timespec *ts, int base);
#else

    #if defined(_USE_POSIX)
        // Returns the current (since power-on) timespec |CLOCK_MONOTONIC|
        // tick / freq
        static inline int timespec_get_systime(struct timespec *ts)
        {
            // TODO(deadbeef): Do we need to handle the case when CLOCK_MONOTONIC is not
            // supported?
            return clock_gettime(CLOCK_MONOTONIC, ts);
        }
        // Returns the UTC timespec |CLOCK_REALTIME|
        // us since 1970/1/1 00:00:00, usually get from rtc
        static inline int timespec_get_utctime(struct timespec *ts)
        {
        #if 0
            struct timeval time;
            gettimeofday(&time, nullptr);
            // Convert from second (1.0) and microsecond (1e-6).
            return ((int64_t)(time.tv_sec) * MS_PER_SEC +
                    (int64_t)(time.tv_usec) / US_PER_MS);
        #else
            return clock_gettime(CLOCK_REALTIME, ts);
        #endif /* 0 */
        }
        static inline int timespec_get(struct timespec *ts, int base)
        {
            return timespec_get_utctime(ts);
        }
    #elif defined(_USE_WIN)
        static inline int timespec_get_systime(struct timespec *ts)
        {
            int64_t ticks;

            static volatile LONG last_timegettime = 0;
            static volatile int64_t num_wrap_timegettime = 0;
            volatile LONG *last_timegettime_ptr = &last_timegettime;
            DWORD now = timeGetTime();
            // Atomically update the last gotten time
            DWORD old = InterlockedExchange(last_timegettime_ptr, now);
            if (now < old) {
                // If now is earlier than old, there may have been a race between threads.
                // 0x0fffffff ~3.1 days, the code will not take that long to execute
                // so it must have been a wrap around.
                if (old > 0xf0000000 && now < 0x0fffffff) {
                    num_wrap_timegettime++;
                }
            }
            ticks = now + (num_wrap_timegettime << 32);
            // TODO(deadbeef): Calculate with nanosecond precision. Otherwise, we're
            // just wasting a multiply and divide when doing Time() on Windows.
            // ticks = ticks * NS_PER_MS;

            timespec_from_millisec(ts, ticks);
        }
        static inline int timespec_get_utctime(struct timespec *ts)
        {
            struct _timeb time;
            _ftime(&time);
            // Convert from second (1.0) and milliseconds (1e-3).
            ts->tv_sec = (int64_t)(time.time);
            ts->tv_nsec = (int64_t)(time.millitm) * NS_PER_MS;

            return 0;
        }
        static inline int timespec_get(struct timespec *ts, int base)
        {
            return timespec_get_utctime(ts);
        }
    #elif defined(_USE_NONE)
        static inline int timespec_get_systime(struct timespec *ts)
        {
            int64_t ticks;
            // how to get systick?
            // ticks = systick / freq;
            timespec_from_millisec(ts, ticks);

            return 0;
        }
        static inline int timespec_get_utctime(struct timespec *ts)
        {
            // how to get time from rtc?
            return timespec_get_systime(ts);
        }
        static inline int timespec_get(struct timespec *ts, int base);
        {
            return timespec_get_utctime(ts);
        }
    #else
        #error "time: unknown branch in defined(_USE_POSIX)"
    #endif /* defined(_USE_POSIX) */
#endif /* defined(_USE_STD) */

/* // http://doc.ntp.org/archives/4.1.2/leap/
// us since 1900/1/1 00:00:00, usually get from ntp server
static inline int timespec_get_ntptime(struct timespec *ts)
{
    static const int64_t kNtpJan1970Sec = 2208988800;
    timespec_get_utctime(ts);
    ts->tc_sec += kNtpJan1970Sec;
    return 0;
} */

// force undef current defines 
#undef _USE_STD
#undef _USE_POSIX
#undef _USE_WIN
#undef _USE_NONE

#if defined(__cplusplus)
}
#endif

#endif /* _CTIME_H */