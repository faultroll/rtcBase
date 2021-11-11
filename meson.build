project('webrtc-audio', 'c', 'cpp',
  version : '0.4.0',
  meson_version : '>= 0.47',
  default_options : [ 'warning_level=1',
                      'buildtype=debugoptimized' ])

soversion = 0

cc = meson.get_compiler('c')

host_system = host_machine.system()

platform_cflags = []
os_cflags = []
os_deps = []
have_posix = false
have_win = false

audio_conf = configuration_data()

if ['darwin', 'ios'].contains(host_system)
  os_cflags = ['-DWEBRTC_MAC', '-DWEBRTC_THREAD_RR', '-DWEBRTC_CLOCK_TYPE_REALTIME']
  if host_system == 'ios'
    os_cflags += ['-DWEBRTC_IOS']
    audio_conf.set('WEBRTC_IOS', 1)
  endif
  have_posix = true
  audio_conf.set('WEBRTC_MAC', 1)
  audio_conf.set('WEBRTC_THREAD_RR', 1)
  audio_conf.set('WEBRTC_CLOCK_TYPE_REALTIME', 1)
elif host_system == 'android'
  os_cflags += ['-DWEBRTC_ANDROID', '-DWEBRTC_LINUX', '-DWEBRTC_THREAD_RR', '-DWEBRTC_CLOCK_TYPE_REALTIME']
  os_deps += [cc.find_library('log')]
  os_deps += [dependency('gnustl', required : get_option('gnustl'))]
  have_posix = true
  audio_conf.set('WEBRTC_ANDROID', 1)
  audio_conf.set('WEBRTC_LINUX', 1)
  audio_conf.set('WEBRTC_THREAD_RR', 1)
  audio_conf.set('WEBRTC_CLOCK_TYPE_REALTIME', 1)
elif host_system == 'linux'
  os_cflags += ['-DWEBRTC_LINUX', '-DWEBRTC_THREAD_RR']
  os_deps += [cc.find_library('rt', required : false)]
  os_deps += [dependency('threads')]
  have_posix = true
  audio_conf.set('WEBRTC_LINUX', 1)
  audio_conf.set('WEBRTC_THREAD_RR', 1)
elif host_system == 'windows'
  platform_cflags += ['-DWEBRTC_WIN', '-D_WIN32', '-U__STRICT_ANSI__']
  os_deps += [cc.find_library('winmm')]
  have_win = true
  audio_conf.set('WEBRTC_WIN', 1)
  audio_conf.set('_WIN32', 1)
  audio_conf.set('__STRICT_ANSI__', 0)
endif

if have_posix
  platform_cflags += ['-DWEBRTC_POSIX']
  audio_conf.set('WEBRTC_POSIX', 1)
endif

arch_cflags = []
have_arm = false
have_armv7 = false
have_neon = false
have_x86 = false
if ['arm', 'armv7'].contains(host_machine.cpu_family())
  if cc.compiles('''#ifdef __ARM_ARCH_ISA_ARM
#error no arm arch
#endif''')
    have_arm = true
    arch_cflags += ['-DWEBRTC_ARCH_ARM']
    audio_conf.set('WEBRTC_ARCH_ARM', 1)
  endif
  if cc.compiles('''#ifndef __ARM_ARCH_7A__
#error no armv7 arch
#endif''')
    have_armv7 = true
    arch_cflags += ['-DWEBRTC_ARCH_ARM_V7']
    audio_conf.set('WEBRTC_ARCH_ARM_V7', 1)
  endif
endif
if cc.compiles('''#ifndef __aarch64__
#error no aarch64 arch
#endif''')
  have_neon = true
  arch_cflags += ['-DWEBRTC_ARCH_ARM64', '-DWEBRTC_HAS_NEON']
  audio_conf.set('WEBRTC_ARCH_ARM64', 1)
  audio_conf.set('WEBRTC_HAS_NEON', 1)
endif
if ['x86', 'x86_64'].contains(host_machine.cpu_family())
  have_x86 = true
endif

neon_opt = get_option('neon')
if neon_opt != 'no'
  if neon_opt != 'runtime'
    if cc.compiles('#include <arm_neon.h>', args : '-mfpu=neon')
      arch_cflags += ['-mfpu=neon', '-DWEBRTC_HAS_NEON']
      have_neon = true
      audio_conf.set('WEBRTC_HAS_NEON', 1)
    endif
  else
    neon_opt += ['-DWEBRTC_DETECT_NEON', '-mfpu=neon']
    have_neon = true
    audio_conf.set('WEBRTC_DETECT_NEON', 1)
  endif
endif

noise_cflags = []
if get_option('ns_mode') == 'float'
  noise_cflags += ['-DWEBRTC_NS_FLOAT=1']
  audio_conf.set('WEBRTC_NS_FLOAT', 1)
else
  noise_cflags += ['-DWEBRTC_NS_FIXED=1']
  audio_conf.set('WEBRTC_NS_FIXED', 1)
endif

common_cflags = ['-DNDEBUG', '-DWEBRTC_AUDIO_PROCESSING_ONLY_BUILD', '-DWEBRTC_RESTRICT_LOGGING'] + platform_cflags + os_cflags + arch_cflags + noise_cflags
common_cxxflags = ['-std=c++11'] + ['-fpermissive'] + common_cflags
common_deps = os_deps
webrtc_inc = include_directories('.')
audio_conf.set('WEBRTC_AUDIO_PROCESSING_ONLY_BUILD', 1)
audio_conf.set('NDEBUG', 1)

configure_file(output : 'config.h', configuration : audio_conf)

webrtc_sources = [
  # 'common_types.cc'
]

webrtc_headers = [
  # 'common.h',
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

subdir('rtc_base')
subdir('system_wrappers')
subdir('common_audio')
subdir('modules')

libwebrtc_audio = static_library('webrtc_audio',
    dependencies: [base_dep, system_wrappers_dep, utility_dep, webrtc_dep, 
            common_audio_dep, audio_coding_dep, webrtc_audio_processing_dep, 
            webrtc_audio_conference_mixer_dep, webrtc_audio_mixer_dep] + common_deps,
    include_directories: webrtc_inc,
    c_args: common_cflags,
    cpp_args: common_cxxflags
)

pkgconfig = import('pkgconfig')

pkgconfig.generate(
    name: 'webrtc-audio',
    description: 'WebRTC Audio library',
    version: meson.project_version(),
    filebase: 'webrtc-audio',
    subdirs: 'webrtc_audio',
    extra_cflags: [
      '-DWEBRTC_AUDIO_PROCESSING_ONLY_BUILD',
    ] + platform_cflags,
    # XXX: passing the libwebrtc_audio object result in adding not-installed libraries to Libs.private
    libraries: '-lwebrtc_audio',
    libraries_private: common_deps,
)
