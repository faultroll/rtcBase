system_wrappers_sources = [
  'source/aligned_malloc.cc',
  'source/cpu_features.cc',
  'source/event.cc',
  'source/file_impl.cc',
  'source/metrics_default.cc',
  'source/rw_lock.cc',
  'source/sleep.cc',
  'source/trace_impl.cc',
]

if have_posix
  system_wrappers_sources += [
    'source/event_timer_posix.cc',
    'source/rw_lock_posix.cc',
    'source/trace_posix.cc',
  ]
endif

if have_win
  system_wrappers_sources += [
    'source/event_timer_win.cc',
    'source/rw_lock_win.cc',
    'source/trace_win.cc',
  ]
endif

system_headers = [
  'include/trace.h',
]

install_headers(system_headers,
    subdir: 'webrtc_audio/system_wrappers/include'
)

# libsystem_wrappers = static_library('system_wrappers',
#     system_wrappers_sources,
#     dependencies: common_deps,
#     include_directories: webrtc_inc,
#     c_args : common_cflags,
#     cpp_args : common_cxxflags
# )

system_wrappers_dep = declare_dependency(
    # link_with: libsystem_wrappers
    sources: system_wrappers_sources,
    include_directories: webrtc_inc
)
