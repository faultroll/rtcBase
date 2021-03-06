/*
 *  Copyright 2004 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "rtc_base/time_utils.h"
// #include "rtc_base/checks.h"

namespace rtc {

/* int64_t SystemTimeNanos() {
  return SystemTimeMillis() * kNumNanosecsPerMillisec;
} */

int64_t SystemTimeMillis() {
  struct timespec ts;
  timespec_get_systime(&ts);
  return timespec_to_millisec(&ts);
}

int64_t UTCTimeMillis() {
  struct timespec ts;
  timespec_get_utctime(&ts);
  return timespec_to_millisec(&ts);
}

int64_t TimeMillis() {
  return SystemTimeMillis();
}

int64_t TimeNanos() {
  return TimeMillis() * kNumNanosecsPerMillisec;
}

int64_t TimeMicros() {
  return TimeMillis() * kNumMicrosecsPerMillisec;
}

int64_t TimeAfter(int64_t elapsed) {
  // RTC_DCHECK_GE(elapsed, 0);
  return TimeMillis() + elapsed;
}

int64_t TimeDiff(int64_t later, int64_t earlier) {
  return later - earlier;
}

/* TimestampWrapAroundHandler::TimestampWrapAroundHandler()
    : last_ts_(0), num_wrap_(-1) {}

int64_t TimestampWrapAroundHandler::Unwrap(uint32_t ts) {
  if (num_wrap_ == -1) {
    last_ts_ = ts;
    num_wrap_ = 0;
    return ts;
  }

  if (ts < last_ts_) {
    if (last_ts_ >= 0xf0000000 && ts < 0x0fffffff)
      ++num_wrap_;
  } else if ((ts - last_ts_) > 0xf0000000) {
    // Backwards wrap. Unwrap with last wrap count and don't update last_ts_.
    return ts + ((num_wrap_ - 1) << 32);
  }

  last_ts_ = ts;
  return ts + (num_wrap_ << 32);
} */

int64_t TmToTime(const struct tm *tm) {
  static short int mdays[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  static short int cumul_mdays[12] = {0,   31,  59,  90,  120, 151,
                                      181, 212, 243, 273, 304, 334};
  int year = tm->tm_year + 1900;
  int month = tm->tm_mon;
  int day = tm->tm_mday - 1;  // Make 0-based like the rest.
  int hour = tm->tm_hour;
  int min = tm->tm_min;
  int sec = tm->tm_sec;

  bool expiry_in_leap_year = (year % 4 == 0 &&
                              (year % 100 != 0 || year % 400 == 0));

  if (year < 1970)
    return -1;
  if (month < 0 || month > 11)
    return -1;
  if (day < 0 || day >= mdays[month] + (expiry_in_leap_year && month == 2 - 1))
    return -1;
  if (hour < 0 || hour > 23)
    return -1;
  if (min < 0 || min > 59)
    return -1;
  if (sec < 0 || sec > 59)
    return -1;

  day += cumul_mdays[month];

  // Add number of leap days between 1970 and the expiration year, inclusive.
  day += ((year / 4 - 1970 / 4) - (year / 100 - 1970 / 100) +
          (year / 400 - 1970 / 400));

  // We will have added one day too much above if expiration is during a leap
  // year, and expiration is in January or February.
  if (expiry_in_leap_year && month <= 2 - 1) // |month| is zero based.
    day -= 1;

  // Combine all variables into second from 1970-01-01 00:00 (except |month|
  // which was accumulated into |day| above).
  int64_t second = (((static_cast<int64_t>
            (year - 1970) * 365 + day) * 24 + hour) * 60 + min) * 60 + sec;
  return second * kNumMillisecsPerSec;
}

void Timespec(struct timespec *ts) {
  timespec_get_systime(ts); // timespec_get(ts, TIME_UTC);
}

void TimespecNormalize(struct timespec *ts) {
  timespec_normalize(ts);
}

void TimespecAfter(struct timespec *ts, struct timespec *elapsed) {
  ts->tv_sec  += elapsed->tv_sec;
  ts->tv_nsec += elapsed->tv_nsec;
  timespec_normalize(ts);
}

void TimeToTimespec(struct timespec *ts, int64_t milliseconds) {
  timespec_from_millisec(ts, milliseconds);
}

int64_t TimespecToTime(struct timespec *ts) {
  return timespec_to_millisec(ts);
}

} // namespace rtc
