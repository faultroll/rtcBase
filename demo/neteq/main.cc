#include <stdio.h>
#include "system_wrappers/include/sleep.h"
#include "common_audio/signal_processing/include/signal_processing_library.h"
/* TEST4 */
#include "modules/audio_coding/neteq/normal.h"
/* TEST3 */
#include "modules/audio_coding/neteq/accelerate.h"
// #include "modules/audio_coding/neteq/background_noise.h"
/* TEST2 */
#include "modules/audio_coding/neteq/preemptive_expand.h"
/* TEST1 */
#include "modules/audio_coding/neteq/buffer_level_filter.h"
#include "modules/audio_coding/neteq/delay_manager.h"
#include "modules/audio_coding/neteq/delay_peak_detector.h"
#include "modules/audio_coding/neteq/tick_timer.h"

using namespace webrtc;

const size_t num_channels_ = 1;
const int fs_hz_ = 16000;
/* const */ size_t fs_mult_ = fs_hz_ / 8000;
const size_t required_samples_ = static_cast<size_t>(240 * fs_mult_);  // Must have 30 ms.

/* TEST4 */
Normal *normal_;
int DoNormal(int16_t *decoded_buffer, // input_buffer
             size_t decoded_length, // input_samples
             AudioMultiVector *output)
{
    /* assert(normal_.get());
    assert(mute_factor_array_.get()); */

    size_t before_samples, after_samples;
    before_samples = output->Size();
    int return_code =
        normal_->Process(decoded_buffer, decoded_length, output/* algorithm_buffer_.get() */);
    after_samples = output->Size();
    /* if (decoded_length != 0)
    {
        last_mode_ = kModeNormal;
    } */
    printf("return_code(%d), before_samples(%zu), after_samples(%zu)\n",
           return_code, before_samples, after_samples);

    return 0;
}
/* TEST3 */
Accelerate *accelerate_;
int DoAccelerate(int16_t *decoded_buffer, // input_buffer
                 size_t decoded_length, // input_samples
                 AudioMultiVector *output
                )
{
    bool fast_accelerate = false;
    /* size_t borrowed_samples_per_channel = 0;
    size_t decoded_length_per_channel = decoded_length / num_channels_;
    if (decoded_length_per_channel < required_samples_)
    {
        // Must move data from the |sync_buffer_| in order to get 30 ms.
        borrowed_samples_per_channel = static_cast<int>(required_samples_ -
                                       decoded_length_per_channel);
        memmove(&decoded_buffer[borrowed_samples_per_channel * num_channels_],
                decoded_buffer,
                sizeof(int16_t) * decoded_length);
        sync_buffer_->ReadInterleavedFromEnd(borrowed_samples_per_channel,
                                             decoded_buffer);
        decoded_length = required_samples_ * num_channels_;
    } */

    size_t before_samples, after_samples;
    before_samples = output->Size();
    size_t samples_removed;
    Accelerate::ReturnCodes return_code =
        accelerate_->Process(decoded_buffer, decoded_length, fast_accelerate,
                             output /* algorithm_buffer_.get() */, &samples_removed);
    after_samples = output->Size();
    printf("return_code(%d), samples_removed(%zu), before_samples(%zu), after_samples(%zu)\n",
           return_code, samples_removed, before_samples, after_samples);
    /* stats_.AcceleratedSamples(samples_removed); */
    /* switch (return_code)
    {
        case Accelerate::kSuccess:
            last_mode_ = kModeAccelerateSuccess;
            break;
        case Accelerate::kSuccessLowEnergy:
            last_mode_ = kModeAccelerateLowEnergy;
            break;
        case Accelerate::kNoStretch:
            last_mode_ = kModeAccelerateFail;
            break;
        case Accelerate::kError:
            // TODO(hlundin): Map to kModeError instead?
            last_mode_ = kModeAccelerateFail;
            return kAccelerateError;
    } */

    /* if (borrowed_samples_per_channel > 0)
    {
        // Copy borrowed samples back to the |sync_buffer_|.
        size_t length = algorithm_buffer_->Size();
        if (length < borrowed_samples_per_channel)
        {
            // This destroys the beginning of the buffer, but will not cause any
            // problems.
            sync_buffer_->ReplaceAtIndex(*algorithm_buffer_,
                                         length,
                                         sync_buffer_->Size() -
                                         borrowed_samples_per_channel);
            sync_buffer_->PushFrontZeros(borrowed_samples_per_channel - length);
            algorithm_buffer_->PopFront(length);
            assert(algorithm_buffer_->Empty());
        }
        else
        {
            sync_buffer_->ReplaceAtIndex(*algorithm_buffer_,
                                         borrowed_samples_per_channel,
                                         sync_buffer_->Size() -
                                         borrowed_samples_per_channel);
            algorithm_buffer_->PopFront(borrowed_samples_per_channel);
        }
    } */

    return 0;
}
/* TEST2 */
PreemptiveExpand *preemptive_expand_;
int DoPreemptiveExpand(int16_t *decoded_buffer, // input_buffer
                       size_t decoded_length, // input_samples
                       AudioMultiVector *output
                      )
{
    /* size_t borrowed_samples_per_channel = 0;
    size_t old_borrowed_samples_per_channel = 0;
    size_t decoded_length_per_channel = decoded_length / num_channels;
    if (decoded_length_per_channel < required_samples) {
      // Must move data from the |sync_buffer_| in order to get 30 ms.
      borrowed_samples_per_channel =
          required_samples - decoded_length_per_channel;
      // Calculate how many of these were already played out.
      old_borrowed_samples_per_channel =
          (borrowed_samples_per_channel > sync_buffer_->FutureLength()) ?
          (borrowed_samples_per_channel - sync_buffer_->FutureLength()) : 0;
      memmove(&decoded_buffer[borrowed_samples_per_channel * num_channels],
              decoded_buffer,
              sizeof(int16_t) * decoded_length);
      sync_buffer_->ReadInterleavedFromEnd(borrowed_samples_per_channel,
                                           decoded_buffer);
      decoded_length = required_samples * num_channels;
    } */

    size_t before_samples, after_samples;
    before_samples = output->Size();
    size_t samples_added;
    PreemptiveExpand::ReturnCodes return_code = preemptive_expand_->Process(
                decoded_buffer, decoded_length,
                0/* old_borrowed_samples_per_channel */,
                output/* algorithm_buffer_.get() */, &samples_added);
    after_samples = output->Size();
    printf("return_code(%d), samples_added(%zu), before_samples(%zu), after_samples(%zu)\n",
           return_code, samples_added, before_samples, after_samples);
    /* stats_.PreemptiveExpandedSamples(samples_added);
    switch (return_code) {
      case PreemptiveExpand::kSuccess:
        last_mode_ = kModePreemptiveExpandSuccess;
        break;
      case PreemptiveExpand::kSuccessLowEnergy:
        last_mode_ = kModePreemptiveExpandLowEnergy;
        break;
      case PreemptiveExpand::kNoStretch:
        last_mode_ = kModePreemptiveExpandFail;
        break;
      case PreemptiveExpand::kError:
        // TODO(hlundin): Map to kModeError instead?
        last_mode_ = kModePreemptiveExpandFail;
        return kPreemptiveExpandError;
    } */

    /* if (borrowed_samples_per_channel > 0) {
      // Copy borrowed samples back to the |sync_buffer_|.
      sync_buffer_->ReplaceAtIndex(
          *algorithm_buffer_, borrowed_samples_per_channel,
          sync_buffer_->Size() - borrowed_samples_per_channel);
      algorithm_buffer_->PopFront(borrowed_samples_per_channel);
    } */

    return 0;
}
/* TEST1 */
#if 0
/* void set_sample_memory(int32_t value) { sample_memory_ = value; }
void set_prev_time_scale(bool value) { prev_time_scale_ = value; }
switch (*operation) {
  case kExpand: {
    timestamp_ = end_timestamp;
    return 0;
  }
  case kAccelerate:
  case kFastAccelerate: {
    // In order to do an accelerate we need at least 30 ms of audio data.
    if (samples_left >= static_cast<int>(samples_30_ms)) {
      // Already have enough data, so we do not need to extract any more.
      decision_logic_->set_sample_memory(samples_left);
      decision_logic_->set_prev_time_scale(true);
      return 0;
    } else if (samples_left >= static_cast<int>(samples_10_ms) &&
        decoder_frame_length_ >= samples_30_ms) {
      // Avoid decoding more data as it might overflow the playout buffer.
      *operation = kNormal;
      return 0;
    } else if (samples_left < static_cast<int>(samples_20_ms) &&
        decoder_frame_length_ < samples_30_ms) {
      // Build up decoded data by decoding at least 20 ms of audio data. Do
      // not perform accelerate yet, but wait until we only need to do one
      // decoding.
      required_samples = 2 * output_size_samples_;
      *operation = kNormal;
    }
    // If none of the above is true, we have one of two possible situations:
    // (1) 20 ms <= samples_left < 30 ms and decoder_frame_length_ < 30 ms; or
    // (2) samples_left < 10 ms and decoder_frame_length_ >= 30 ms.
    // In either case, we move on with the accelerate decision, and decode one
    // frame now.
    break;
  }
  case kPreemptiveExpand: {
    // In order to do a preemptive expand we need at least 30 ms of decoded
    // audio data.
    if ((samples_left >= static_cast<int>(samples_30_ms)) ||
        (samples_left >= static_cast<int>(samples_10_ms) &&
            decoder_frame_length_ >= samples_30_ms)) {
      // Already have enough data, so we do not need to extract any more.
      // Or, avoid decoding more data as it might overflow the playout buffer.
      // Still try preemptive expand, though.
      decision_logic_->set_sample_memory(samples_left);
      decision_logic_->set_prev_time_scale(true);
      return 0;
    }
    if (samples_left < static_cast<int>(samples_20_ms) &&
        decoder_frame_length_ < samples_30_ms) {
      // Build up decoded data by decoding at least 20 ms of audio data.
      // Still try to perform preemptive expand.
      required_samples = 2 * output_size_samples_;
    }
    // Move on with the preemptive expand decision.
    break;
  }
  case kMerge: {
    required_samples =
        std::max(merge_->RequiredFutureSamples(), required_samples);
    break;
  }
  default: {
    // Do nothing.
  }
}

if (*operation == kAccelerate || *operation == kFastAccelerate ||
    *operation == kPreemptiveExpand) {
  decision_logic_->set_sample_memory(samples_left + extracted_samples);
  decision_logic_->set_prev_time_scale(true);
}
if (*operation == kAccelerate || *operation == kFastAccelerate) {
  // Check that we have enough data (30ms) to do accelerate.
  if (extracted_samples + samples_left < static_cast<int>(samples_30_ms)) {
    // TODO(hlundin): Write test for this.
    // Not enough, do normal operation instead.
    *operation = kNormal;
  }
} */
/* // Adds |value| to |sample_memory_|.
void AddSampleMemory(int32_t value) {
  sample_memory_ += value;
}
if (last_mode_ == kModeAccelerateSuccess ||
    last_mode_ == kModeAccelerateLowEnergy ||
    last_mode_ == kModePreemptiveExpandSuccess ||
    last_mode_ == kModePreemptiveExpandLowEnergy) {
  // Subtract (samples_left + output_size_samples_) from sampleMemory.
  decision_logic_->AddSampleMemory(
      -(samples_left + rtc::checked_cast<int>(output_size_samples_)));
} */
void FilterBufferLevel(size_t buffer_size_samples
                      )
{
    buffer_level_filter_->SetTargetBufferLevel(
        delay_manager_->base_target_level());

    size_t buffer_size_packets = 0;
    if (packet_length_samples_ > 0)
    {
        // Calculate size in packets.
        buffer_size_packets = buffer_size_samples / packet_length_samples_;
    }
    int sample_memory_local = 0;

    if (prev_time_scale_)
    {
        sample_memory_local = sample_memory_;
        timescale_countdown_ =
            tick_timer_->GetNewCountdown(kMinTimescaleInterval);
    }
    buffer_level_filter_->Update(buffer_size_packets, sample_memory_local,
                                 packet_length_samples_);
    prev_time_scale_ = false;
}
/* static const int kReinitAfterExpands = 100;
static const int kMaxWaitForPacket = 10;
void ExpandDecision(Operations operation) {
  if (operation == kExpand) {
    num_consecutive_expands_++;
  } else {
    num_consecutive_expands_ = 0;
  }
}
// Static method returning true if |timestamp| is older than |timestamp_limit|
// but less than |horizon_samples| behind |timestamp_limit|. For instance,
// with timestamp_limit = 100 and horizon_samples = 10, a timestamp in the
// range (90, 100) is considered obsolete, and will yield true.
// Setting |horizon_samples| to 0 is the same as setting it to 2^31, i.e.,
// half the 32-bit timestamp range.
static bool IsObsoleteTimestamp(uint32_t timestamp,
                                uint32_t timestamp_limit,
                                uint32_t horizon_samples) {
  return IsNewerTimestamp(timestamp_limit, timestamp) &&
         (horizon_samples == 0 ||
          IsNewerTimestamp(timestamp, timestamp_limit - horizon_samples));
} */
// Returns the operation that should be done next. |sync_buffer| and |expand|
// are provided for reference. |decoder_frame_length| is the number of samples
// obtained from the last decoded frame. If there is a packet available, it
// should be supplied in |next_packet|; otherwise it should be NULL. The mode
// resulting from the last call to NetEqImpl::GetAudio is supplied in
// |prev_mode|. The output variable |reset_decoder| will be set to true if a reset is
// required; otherwise it is left unchanged (i.e., it can remain true if it
// was true before the call).
Operations GetDecision(size_t decoder_frame_length,
                       Modes prev_mode
                      )
{
    const size_t samples_left =
        sync_buffer.FutureLength() - overlap_samples_/* expand.overlap_length() */; // sync_buffer: Size() - next_index_
    const size_t cur_size_samples =
        samples_left + packet_buffer_.NumSamplesInBuffer(decoder_frame_length); // packet_buffer_: NumSamplesInBuffer(last_decoded_length)

    prev_time_scale_ = prev_time_scale_ &&
                       (prev_mode == kModeAccelerateSuccess ||
                        prev_mode == kModeAccelerateLowEnergy ||
                        prev_mode == kModePreemptiveExpandSuccess ||
                        prev_mode == kModePreemptiveExpandLowEnergy);

    FilterBufferLevel(cur_size_samples);

    // assert(playout_mode_ == kPlayoutOn || playout_mode_ == kPlayoutStreaming);
    // Guard for errors, to avoid getting stuck in error mode.
    if (prev_mode == kModeError)
    {
        if (!next_packet)
        {
            return kExpand;
        }
        else
        {
            return kUndefined;  // flag for a reset.
        }
    }

    uint32_t target_timestamp = sync_buffer.end_timestamp();
    uint32_t available_timestamp = 0;
    if (next_packet)
    {
        available_timestamp = next_packet->timestamp;
    }

    // Handle the case with no packet at all available.
    // if (!next_packet)
    else
    {
        return kExpand; // return NoPacket();
    }

    // If the expand period was very long, reset NetEQ since it is likely that the
    // sender was restarted.
    if (num_consecutive_expands_ > kReinitAfterExpands)
    {
        *reset_decoder = true;
        return kNormal;
    }

    const uint32_t five_seconds_samples =
        static_cast<uint32_t>(5 * 8000 * fs_mult_);
    // Check if the required packet is available.
    if (target_timestamp == available_timestamp)
    {
        return ExpectedPacketAvailable(prev_mode);
    }
    else if (!IsObsoleteTimestamp(
                 available_timestamp, target_timestamp, five_seconds_samples))
    {
        return FuturePacketAvailable(prev_mode,
                                     available_timestamp, target_timestamp);
    }
    else
    {
        // This implies that available_timestamp < target_timestamp, which can
        // happen when a new stream or codec is received. Signal for a reset.
        return kUndefined;
    }
}
/* // Checks if enough time has elapsed since the last successful timescale
// operation was done (i.e., accelerate or preemptive expand).
bool TimescaleAllowed() const {
  return !timescale_countdown_ || timescale_countdown_->Finished();
}
// The value 5 sets maximum time-stretch rate to about 100 ms/s.
static const int kMinTimescaleInterval = 5;
timescale_countdown_(
          tick_timer_->GetNewCountdown(kMinTimescaleInterval + 1)), */
Operations ExpectedPacketAvailable(Modes prev_mode
                                  )
{
    if (prev_mode != kModeExpand)
    {
        // Check criterion for time-stretching.
        int low_limit, high_limit;
        delay_manager_->BufferLimits(&low_limit, &high_limit);
        if (buffer_level_filter_->filtered_current_level() >= high_limit << 2)
            return kFastAccelerate;
        if (TimescaleAllowed())
        {
            if (buffer_level_filter_->filtered_current_level() >= high_limit)
                return kAccelerate;
            if (buffer_level_filter_->filtered_current_level() < low_limit)
                return kPreemptiveExpand;
        }
    }
    return kNormal;
}
/* bool DecisionLogicNormal::UnderTargetLevel() const {
  return buffer_level_filter_->filtered_current_level() <=
      delay_manager_->TargetLevel();
}
bool DecisionLogicNormal::ReinitAfterExpands(uint32_t timestamp_leap) const {
  return timestamp_leap >=
      static_cast<uint32_t>(output_size_samples_ * kReinitAfterExpands);
}
bool DecisionLogicNormal::PacketTooEarly(uint32_t timestamp_leap) const {
  return timestamp_leap >
      static_cast<uint32_t>(output_size_samples_ * num_consecutive_expands_);
}
bool DecisionLogicNormal::MaxWaitForPacket() const {
  return num_consecutive_expands_ >= kMaxWaitForPacket;
} */
Operations FuturePacketAvailable(
    Modes prev_mode,
    uint32_t available_timestamp,
    uint32_t target_timestamp
)
{
    // Required packet is not available, but a future packet is.
    // Check if we should continue with an ongoing expand because the new packet
    // is too far into the future.
    uint32_t timestamp_leap = available_timestamp - target_timestamp;
    if ((prev_mode == kModeExpand) &&
        !ReinitAfterExpands(timestamp_leap) &&
        !MaxWaitForPacket() &&
        PacketTooEarly(timestamp_leap) &&
        UnderTargetLevel())
    {
        // Nothing to play.
        return kExpand;
    }

    // Do not merge unless we have done an expand before.
    if (prev_mode == kModeExpand)
    {
        return kMerge;
    }
    else
    {
        return kExpand;
    }
}
#endif

int main(void)
{
    WebRtcSpl_Init();

#if 1 /* TEST1 (jitter) buffer_level */
    const size_t packet_length_samples = 320;
    static const int kMaxNumberOfPackets = 10;
    static const int kTimeStepMs = 10;
    static const int kFs = fs_hz_;
    static const int kFrameSizeMs = (1000 * packet_length_samples) / fs_hz_; // 20;
    static const int kTsIncrement = kFrameSizeMs * kFs / 1000;
    static const int kMinTimescaleInterval = 5;
    /* const */ TickTimer *tick_timer_ = new TickTimer;
    std::unique_ptr<TickTimer::Countdown> timescale_countdown_ =
        tick_timer_->GetNewCountdown(kMinTimescaleInterval + 1);
    DelayPeakDetector *delay_peak_detector_ = new DelayPeakDetector(tick_timer_, false);
    std::unique_ptr<DelayManager> delay_manager_ = DelayManager::Create(kMaxNumberOfPackets, delay_peak_detector_, tick_timer_);
    /* const int max_delay_ms = 2000, min_delay_ms = 0;
    delay_manager_->SetMaximumDelay(max_delay_ms);
    delay_manager_->SetMinimumDelay(min_delay_ms); */
    delay_manager_->set_streaming_mode(true/* playout_mode_ == kPlayoutStreaming */);
    BufferLevelFilter *buffer_level_filter_ = new BufferLevelFilter;

    /*
      static const int kOutputSizeMs = 10;
      size_t output_size_samples_ = static_cast<size_t>(kOutputSizeMs * 8 * fs_mult_);
      size_t decoder_frame_length_ = 3 * output_size_samples_; // Initialize to 30ms.
      const uint32_t main_timestamp = packet_list.front().timestamp;
      const uint16_t main_sequence_number = packet_list.front().sequence_number;
      uint32_t timestamp_ = next_packet->timestamp;

      // Update |decoder_frame_length_| with number of samples per channel.
      decoder_frame_length_ =
          result.num_decoded_samples / num_channels_;
      // Calculate the total speech length carried in each packet.
      const size_t buffer_length_before_insert =
          packet_buffer_->NumPacketsInBuffer();
      // ... insert packet
      const size_t buffer_length_after_insert =
          packet_buffer_->NumPacketsInBuffer();

      if (buffer_length_after_insert > buffer_length_before_insert) {
        const size_t packet_length_samples =
            (buffer_length_after_insert - buffer_length_before_insert) *
            decoder_frame_length_;
        if (packet_length_samples != decision_logic_->packet_length_samples()) {
          decision_logic_->set_packet_length_samples(packet_length_samples);
          delay_manager_->SetPacketAudioLength(
              rtc::checked_cast<int>((1000 * packet_length_samples) / fs_hz_));
        }
      }
      // Update statistics.
      if ((int32_t)(main_timestamp - timestamp_) >= 0) {
        // Only update statistics if incoming packet is not older than last played
        // out packet, and if new codec flag is not set.
        delay_manager_->Update(main_sequence_number, main_timestamp, fs_hz_);
      }
    */

    /* const */ uint16_t main_sequence_number = 0x1234;
    /* const */ uint32_t main_timestamp = 0x12345678;
    delay_manager_->SetPacketAudioLength(kFrameSizeMs);
    delay_manager_->Update(main_sequence_number, main_timestamp, fs_hz_);
    /* const */ size_t samples_left =
        0; // sync_buffer.FutureLength() - overlap_samples_/* expand.overlap_length() */;
    const size_t cur_size_samples =
        960; // samples_left + packet_buffer_.NumSamplesInBuffer(decoder_frame_length);
    size_t buffer_size_packets = cur_size_samples / packet_length_samples;
    buffer_level_filter_->SetTargetBufferLevel(delay_manager_->base_target_level());
    buffer_level_filter_->Update(buffer_size_packets, samples_left/* time_stretched_samples */, packet_length_samples);
    int low_limit, high_limit;
    bool prev_time_scale_ = false;

    for (int i = 0; i < 60; i++)
    {
        printf("<%3d>: ", i); // No '\n'
        // SleepMs(100);
        printf("base_target_level(%d) TargetLevel(%d), ", // No '\n'
               delay_manager_->base_target_level(), delay_manager_->TargetLevel());
        // IncreaseTime
        for (int t = 0; t < kFrameSizeMs; t += kTimeStepMs)
        {
            tick_timer_->Increment();
        }

        // |high_limit| will not decrease because is at min (1 << 8) by DelayManager::LimitTargetLevel, so was |kMaxNumberOfPackets|
        delay_manager_->BufferLimits(&low_limit, &high_limit);
        printf("low_limit(%d), high_limit(%d), current_level(%4d): ", // No '\n'
               low_limit, high_limit, buffer_level_filter_->filtered_current_level());

        if (buffer_level_filter_->filtered_current_level() >= high_limit << 2)
        {
            printf("kFastAccelerate\n");
            // DoAccelerate(true)
            // set_sample_memory
            samples_left = packet_length_samples * 3; // is it |samples_removed| ?
            // set_prev_time_scale;
            prev_time_scale_ = true;
        }
        else if (buffer_level_filter_->filtered_current_level() >= high_limit)
        {
            // TimescaleAllowed
            if (!timescale_countdown_ || timescale_countdown_->Finished())
            {
                printf("kAccelerate\n");
                // DoAccelerate(false)
                samples_left = packet_length_samples * 1;
                prev_time_scale_ = true;
            }
            else
            {
                printf("kNormal\n");
                // DoNormal
                samples_left = 0;
                prev_time_scale_ = false;
            }
        }
        else if (buffer_level_filter_->filtered_current_level() < low_limit)
        {
            if (!timescale_countdown_ || timescale_countdown_->Finished())
            {
                printf("kPreemptiveExpand\n");
                // DoPreemptiveExpand
                samples_left = packet_length_samples * -1; // is it |samples_added| ?
                prev_time_scale_ = true;
            }
            else
            {
                printf("kNormal\n");
                samples_left = 0;
                prev_time_scale_ = false;
            }
        }
        else
        {
            printf("kNormal\n");
            samples_left = 0;
            prev_time_scale_ = false;
        }
        // then reduce |buffer_size_packets|
        buffer_size_packets -= 1;

        if (prev_time_scale_)
        {
            timescale_countdown_ =
                tick_timer_->GetNewCountdown(kMinTimescaleInterval);
        }

        // InsertNextPacket
        main_sequence_number += 1;
        main_timestamp += kTsIncrement;
        delay_manager_->Update(main_sequence_number, main_timestamp, fs_hz_);
        buffer_size_packets += 3;
        buffer_level_filter_->SetTargetBufferLevel(delay_manager_->base_target_level());
        buffer_level_filter_->Update(buffer_size_packets, samples_left, packet_length_samples);
    }

    delete buffer_level_filter_;
    // delete delay_manager_;
    delete delay_peak_detector_;
    delete tick_timer_;

#elif 0 /* TEST2 (slow) preemptive-expand */
    const size_t overlap_samples_ = 5 * fs_mult_;
    // BackgroundNoise *background_noise = new BackgroundNoise(num_channels_);
    // background_noise->set_mode(BackgroundNoise::kBgnOn);
    preemptive_expand_ = new PreemptiveExpand(fs_hz_, num_channels_/* , *background_noise */, overlap_samples_);
    /* 160 --> 240 */
    const size_t input_samples = required_samples_, output_samples = input_samples << 1;
    int16_t *input_buffer = new int16_t[input_samples], *output_buffer = new int16_t[output_samples];
    AudioMultiVector *output = new AudioMultiVector(num_channels_);

    FILE *pfile1 = fopen("demo/samples/1_16000_16_1.pcm", "rb");
    if (NULL == pfile1)
    {
        perror("fopen:");
    }
    FILE *pfile2 = fopen("stretch_slow.pcm", "wb");
    if (NULL == pfile2)
    {
        perror("fopen:");
    }

    while (pfile1 != NULL && !feof(pfile1))
    {
        if (pfile1 != NULL)
            fread(input_buffer, 1, input_samples * sizeof(int16_t), pfile1);
        DoPreemptiveExpand(input_buffer, input_samples, output);
        size_t output_length = output->Size();
        output->ReadInterleaved(output_length/* output_samples */, output_buffer);
        output->Clear();
        if (pfile2 != NULL)
            fwrite(output_buffer, 1, output_length * sizeof(int16_t), pfile2);
    }

    if (pfile2 != NULL)
        fclose(pfile2);
    if (pfile1 != NULL)
        fclose(pfile1);
    delete output;
    delete output_buffer, delete input_buffer;
    delete preemptive_expand_;
    // delete background_noise;

#elif 0 /* TEST3 (fast) accelerate */
    // BackgroundNoise *background_noise = new BackgroundNoise(num_channels_);
    // background_noise->set_mode(BackgroundNoise::kBgnOn);
    accelerate_ = new Accelerate(fs_hz_, num_channels_/* , *background_noise */);
    /* 320 --> 240 */
    const size_t input_samples = 320 * fs_mult_, output_samples = input_samples/* required_samples_ */;
    int16_t *input_buffer = new int16_t[input_samples], *output_buffer = new int16_t[output_samples];
    AudioMultiVector *output = new AudioMultiVector(num_channels_);

    FILE *pfile1 = fopen("demo/samples/1_16000_16_1.pcm", "rb");
    if (NULL == pfile1)
    {
        perror("fopen:");
    }
    FILE *pfile2 = fopen("stretch_fast.pcm", "wb");
    if (NULL == pfile2)
    {
        perror("fopen:");
    }

    while (pfile1 != NULL && !feof(pfile1))
    {
        if (pfile1 != NULL)
            fread(input_buffer, 1, input_samples * sizeof(int16_t), pfile1);
        DoAccelerate(input_buffer, input_samples, output);
        size_t output_length = output->Size();
        output->ReadInterleaved(output_length/* output_samples */, output_buffer);
        output->Clear();
        if (pfile2 != NULL)
            fwrite(output_buffer, 1, output_length * sizeof(int16_t), pfile2);
    }

    if (pfile2 != NULL)
        fclose(pfile2);
    if (pfile1 != NULL)
        fclose(pfile1);
    delete output;
    delete output_buffer, delete input_buffer;
    delete accelerate_;
    // delete background_noise;

#elif 1 /* TEST4 (normal) normal */
    normal_ = new Normal(fs_hz_);
    /* 240 --> 240 */
    const size_t input_samples = required_samples_, output_samples = required_samples_;
    int16_t *input_buffer = new int16_t[input_samples], *output_buffer = new int16_t[output_samples];
    AudioMultiVector *output = new AudioMultiVector(num_channels_);

    FILE *pfile1 = fopen("demo/samples/1_16000_16_1.pcm", "rb");
    if (NULL == pfile1)
    {
        perror("fopen:");
    }
    FILE *pfile2 = fopen("stretch_normal.pcm", "wb");
    if (NULL == pfile2)
    {
        perror("fopen:");
    }

    while (pfile1 != NULL && !feof(pfile1))
    {
        if (pfile1 != NULL)
            fread(input_buffer, 1, input_samples * sizeof(int16_t), pfile1);
        DoNormal(input_buffer, input_samples, output);
        size_t output_length = output->Size();
        output->ReadInterleaved(output_length/* output_samples */, output_buffer);
        output->Clear();
        if (pfile2 != NULL)
            fwrite(output_buffer, 1, output_length * sizeof(int16_t), pfile2);
    }

    if (pfile2 != NULL)
        fclose(pfile2);
    if (pfile1 != NULL)
        fclose(pfile1);
    delete output;
    delete output_buffer, delete input_buffer;
    delete normal_;
#endif

    return 0;
}

