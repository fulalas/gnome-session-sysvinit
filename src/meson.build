sources = files(
	'leader-sysvinit.c',
)

executable(
	'gnome-session-init-worker',
	sources,
	include_directories: top_inc,
	dependencies: session_bin_deps,
	install: true,
	install_dir: session_libexecdir
)

sources = files(
	'gnome-session-ctl.c',
)

executable(
	'gnome-session-ctl',
	sources,
	include_directories: top_inc,
	dependencies: session_bin_deps,
	install: true,
	install_dir: session_libexecdir
)

g_s_start = configure_file(input: 'sysvinit/gnome-session-start.in', output: 'gnome-session-start', configuration: conf_data)

install_data(
  g_s_start,
  'sysvinit/gnome-session-stop',
  install_dir: session_libexecdir,
  install_mode: 'rwxr-xr-x',
)

