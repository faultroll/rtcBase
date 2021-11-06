webrtc_audio_mixer_sources = [
    'audio_frame_manipulator.cc',
    'audio_mixer_impl.cc',
    'default_output_rate_calculator.cc',
]

webrtc_audio_mixer_include_headers = [
    'include/audio_mixer.h',
]

install_headers(webrtc_audio_mixer_include_headers,
    subdir: 'webrtc_audio/webrtc/modules/audio_mixer/include'
)

# libwebrtc_audio_mixer = library('webrtc_audio_mixer',
#     webrtc_audio_mixer_sources,
#     dependencies: [base_dep, audio_coding_dep, system_wrappers_dep, common_audio_dep, utility_dep, webrtc_dep, webrtc_audio_processing_dep] + common_deps,
#     # link_with: extra_libs,
#     include_directories: webrtc_inc,
#     c_args: common_cflags,
#     cpp_args: common_cxxflags,
#     soversion: soversion,
#     install: true
# )

webrtc_audio_mixer_dep = declare_dependency(
    # link_with: libwebrtc_audio_mixer,
    # include_directories: webrtc_inc,
    # version: meson.project_version()
    sources: webrtc_audio_mixer_sources,
    include_directories: webrtc_inc
)
