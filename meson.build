project('katzenwl', 'c', 
  version: '0.1.0',
  license: 'The 3-Clause BSD License',
  meson_version: '>=0.58.1',
  default_options: [
    'c_std=c11',
    'warning_level=1',
    'werror=true'
  ],
)

# Src Files
log_files = 'src/log/kwl-log.c'
allocator_files = [ 'src/renderer/allocators/allocator.c', 
  'src/renderer/allocators/shm.c' ]
interfaces = [ 'src/interfaces/kwl-seat.c', 'src/interfaces/kwl-buffer.c', 
  'src/interfaces/kwl-compositor.c', 'src/interfaces/kwl-output.c' ]
renderers = [ 'src/renderer/software/renderer.c' ]

src_files = [ 
  'src/backend/backend.c',
  interfaces,
  log_files, 
  allocator_files,
  renderers
  ]

# Dependencies
wayland_deps = dependency('wayland-server')
libseat = dependency('libseat')

deps = [ wayland_deps, libseat ]

# Includes Directory
includes = include_directories('includes')


if get_option('buildtype').startswith('debug')
  message('Debug build enabled')
endif

if get_option('disable-xcb-backend')
  message('Libxcb backend disabled kwl will be unable to run under X')
  add_project_arguments('-D_KWL_NO_XCB_', language: 'c')
else
  xcb_deps = [ dependency('xcb'), dependency('xcb-errors') ]
  xcb_files = [ 'src/backend/xcb.c' ]

  deps += xcb_deps
  src_files += xcb_files
endif


if get_option('disable-drm-backend')
  message('Libdrm backend disabled kwl will be unable to run under DRM/KMS')
  add_project_arguments('-D_KWL_NO_DRM_', language: 'c')
else
  drm_deps = [ dependency('libdrm') ]
  drm_files = [ 'src/backend/drm.c' ]

  deps += drm_deps
  src_files += drm_files
endif


shared_library(meson.project_name(), src_files, dependencies: deps, 
  include_directories: includes, soversion: '0', version: meson.project_version())
