/*
 *  Copyright 2004 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "rtc_base/event.h"

#include "rtc_base/checks.h"
#include "rtc_base/timeutils.h"

namespace rtc {

Event::Event(bool manual_reset, bool initially_signaled) {
  Rtc_EvntInit(&event_handle_, manual_reset, initially_signaled);
}

Event::~Event() {
  Rtc_EvntDestroy(&event_handle_);
}

void Event::Set() {
  Rtc_EvntSet(&event_handle_);
}

void Event::Reset() {
  Rtc_EvntReset(&event_handle_);
}

bool Event::Wait(int milliseconds) {
  int ms = (milliseconds == kForever) ? INFINITE : milliseconds;
  return Rtc_EvntWait(&event_handle_, ms);
}

}  // namespace rtc
