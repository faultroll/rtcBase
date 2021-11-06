base_sources = [
  'criticalsection.cc',
  'checks.cc',
  'event.cc',
  'platform_thread.cc',
  'platform_file.cc',
  'stringutils.cc',
  'thread_checker_impl.cc',
  'race_checker.cc',
  'timeutils.cc',
]

base_headers = [
  'arraysize.h',
  'checks.h',
  'constructormagic.h',
  'basictypes.h',
  'maybe.h',
  'platform_file.h',
  'refcount.h',
  'refcountedobject.h',
  'scoped_ref_ptr.h',
  'race_checker.h',
]

install_headers(base_headers,
    subdir: 'webrtc_audio/webrtc/base'
)

# libbase = static_library('libbase',
#     base_sources,
#     dependencies: common_deps,
#     include_directories: webrtc_inc,
#     cpp_args : common_cxxflags
# )

base_dep = declare_dependency(
    # link_with: libbase
    sources: base_sources,
    include_directories: webrtc_inc
)

