audio_coding_sources = [
    # 'codecs/isac/main/source/arith_routines.c',
    # 'codecs/isac/main/source/arith_routines_hist.c',
    # 'codecs/isac/main/source/arith_routines_logist.c',
    # 'codecs/isac/main/source/encode_lpc_swb.c',
    # 'codecs/isac/main/source/entropy_coding.c',
    # 'codecs/isac/main/source/filter_functions.c',
    # 'codecs/isac/main/source/filterbanks.c',
    # 'codecs/isac/main/source/filterbank_tables.c',
    # 'codecs/isac/main/source/intialize.c',
    # 'codecs/isac/main/source/lpc_analysis.c',
    # 'codecs/isac/main/source/lpc_gain_swb_tables.c',
    # 'codecs/isac/main/source/lpc_shape_swb12_tables.c',
    # 'codecs/isac/main/source/lpc_shape_swb16_tables.c',
    # 'codecs/isac/main/source/lpc_tables.c',
    # 'codecs/isac/main/source/pitch_estimator.c',
    # 'codecs/isac/main/source/pitch_filter.c',
    # 'codecs/isac/main/source/pitch_gain_tables.c',
    # 'codecs/isac/main/source/pitch_lag_tables.c',
    # 'codecs/isac/main/source/spectrum_ar_model_tables.c',
]

audio_coding_sources += [
    'neteq/dsp_helper.cc',
    'neteq/audio_multi_vector.cc',
    'neteq/audio_vector.cc',
    'neteq/cross_correlation.cc',
    'neteq/time_stretch.cc',
    'neteq/preemptive_expand.cc',
    'neteq/accelerate.cc',

    # DONE use m75 delay_manager
    'neteq/tick_timer.cc',
    'neteq/delay_peak_detector.cc',
    'neteq/histogram.cc',
    'neteq/delay_manager.cc',
    'neteq/buffer_level_filter.cc',

    # DONE remove below
    # 'neteq/background_noise.cc',
    # 'neteq/post_decode_vad.cc',
    
    'neteq/normal.cc',

    # useless
    # 'neteq/audio_decoder_impl.cc',
    # 'neteq/comfort_noise.cc',
    # 'neteq/decision_logic.cc',
    # 'neteq/decision_logic_fax.cc',
    # 'neteq/decision_logic_normal.cc',
    # 'neteq/decoder_database.cc',
    # 'neteq/dtmf_buffer.cc',
    # 'neteq/dtmf_tone_generator.cc',
    # 'neteq/expand.cc',
    # 'neteq/merge.cc',
    # 'neteq/nack_tracker.cc',
    # 'neteq/neteq.cc',
    # 'neteq/neteq_impl.cc',
    # 'neteq/packet_buffer.cc',
    # 'neteq/packet.cc',
    # 'neteq/random_vector.cc',
    # 'neteq/red_payload_splitter.cc',
    # 'neteq/rtcp.cc',
    # 'neteq/statistics_calculator.cc',
    # 'neteq/sync_buffer.cc',
    # 'neteq/timestamp_scaler.cc',
]

# libaudio_coding = static_library('audio_coding',
#     audio_coding_sources,
#     dependencies: common_deps,
#     include_directories: webrtc_inc,
#     c_args: common_cflags,
#     cpp_args: common_cxxflags
# )

audio_coding_dep = declare_dependency(
    # link_with: libaudio_coding
    sources: audio_coding_sources,
    include_directories: webrtc_inc
)
