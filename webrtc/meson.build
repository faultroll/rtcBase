webrtc_sources = [
  'common_types.cc'
]

webrtc_headers = [
  'common.h',
  'common_types.h',
  'typedefs.h',
]

install_headers(webrtc_headers,
    subdir: 'webrtc_audio/webrtc'
)

# libwebrtc = static_library('webrtc',
#     webrtc_sources,
#     dependencies: common_deps,
#     include_directories: webrtc_inc,
#     c_args: common_cflags,
#     cpp_args: common_cxxflags
# )

webrtc_dep = declare_dependency(
    # link_with: libwebrtc
    sources: webrtc_sources,
    include_directories: webrtc_inc
)

subdir('base')
subdir('common_audio')
subdir('system_wrappers')

subdir('modules')
