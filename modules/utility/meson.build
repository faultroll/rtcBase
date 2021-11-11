# utility_headers = [
#   'include/audio_frame_operations.h',
# ]

# install_headers(utility_headers,
#     subdir: 'webrtc_audio/modules/include'
# )

utility_sources = [
  'include/audio_frame_operations.cc',
]

# libutility = static_library('utility',
#     utility_sources,
#     dependencies: common_deps,
#     include_directories: webrtc_inc,
#     c_args: common_cflags,
#     cpp_args: common_cxxflags
# )

utility_dep = declare_dependency(
    # link_with: libutility
    sources: utility_sources,
    include_directories: webrtc_inc
)
