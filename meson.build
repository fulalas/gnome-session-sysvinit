project(
  'gnome-session-sysvinit', 'c',
  version: '50.0',
  license: 'GPL2+',
  meson_version: '>= 0.61.0'
)

session_prefix = get_option('prefix')
session_libexecdir = join_paths(session_prefix, get_option('libexecdir'))

glib_req_version = '>= 2.82.0'

session_bin_deps = [
  dependency('glib-2.0', version: glib_req_version),
  dependency('gio-2.0', version: glib_req_version),
]

# Nothing here links against elogind, but the session needs it at runtime
# (gnome-session-service uses it), so fail at build time instead of at login.
dependency('libelogind', version: '>= 209', required: true)

top_inc = include_directories('.')

# config.h needs the path quoted for #define, the shell script needs it bare,
# so the two cannot share one configuration_data.
config_h_data = configuration_data()
config_h_data.set_quoted('LIBEXECDIR', session_libexecdir)

configure_file(output: 'config.h', configuration: config_h_data)

script_data = configuration_data()
script_data.set('LIBEXECDIR', session_libexecdir)

subdir('src')

summary(
  {
    'prefix': session_prefix,
    'libexecdir': session_libexecdir,
  },
  section: 'Directories',
)
