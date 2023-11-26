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

if get_option('buildtype').startswith('debug')
  message('Debug build enabled')
endif

src_files = [ 'src/backend/xcb.c', 'src/renderer/x11/xcb-render.c', 'src/interfaces/kwl-compositor.c', 'src/log/kwl-log.c' ]
deps = [ dependency('wayland-server'), dependency('xcb'), dependency('xcb-errors') ]
includes = include_directories('includes')

shared_library(meson.project_name(), src_files, dependencies: deps, 
  include_directories: includes, soversion: '0', version: meson.project_version())