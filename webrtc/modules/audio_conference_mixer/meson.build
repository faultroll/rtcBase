webrtc_audio_conference_mixer_sources = [
    'source/audio_conference_mixer_impl.cc',
    'source/audio_frame_manipulator.cc',
    'source/time_scheduler.cc',
]

webrtc_audio_conference_mixer_include_headers = [
    'include/audio_conference_mixer_defines.h',
    'include/audio_conference_mixer.h',
]

install_headers(webrtc_audio_conference_mixer_include_headers,
    subdir: 'webrtc_audio/webrtc/modules/audio_conference_mixer/include'
)

# libwebrtc_audio_conference_mixer = library('webrtc_audio_conference_mixer',
#     webrtc_audio_conference_mixer_sources,
#     dependencies: [base_dep, audio_coding_dep, system_wrappers_dep, common_audio_dep, utility_dep, webrtc_dep, webrtc_audio_processing_dep] + common_deps,
#     # link_with: extra_libs,
#     include_directories: webrtc_inc,
#     c_args: common_cflags,
#     cpp_args: common_cxxflags,
#     soversion: soversion,
#     install: true
# )

webrtc_audio_conference_mixer_dep = declare_dependency(
    # link_with: libwebrtc_audio_conference_mixer,
    # include_directories: webrtc_inc,
    # version: meson.project_version()
    sources: webrtc_audio_conference_mixer_sources,
    include_directories: webrtc_inc
)
