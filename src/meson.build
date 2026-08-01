executable(
  'gnome-session-init-worker',
  files('leader-sysvinit.c'),
  include_directories: top_inc,
  dependencies: session_bin_deps,
  install: true,
  install_dir: session_libexecdir
)

executable(
  'gnome-session-ctl',
  files('gnome-session-ctl.c'),
  include_directories: top_inc,
  dependencies: session_bin_deps,
  install: true,
  install_dir: session_libexecdir
)

gnome_session_start = configure_file(
  input: 'sysvinit/gnome-session-start.in',
  output: 'gnome-session-start',
  configuration: script_data,
)

install_data(
  gnome_session_start,
  'sysvinit/gnome-session-stop',
  install_dir: session_libexecdir,
  install_mode: 'rwxr-xr-x',
)
