# Install the .desktop files that tell GDM about the GNOME session

wayland_desktops = [
  'gnome',
]
foreach name : wayland_desktops
  desktop = name + '.desktop'
  desktop_in = configure_file(
    input: desktop + '.in.in',
    output: desktop + '.in',
    configuration: {
      'bindir': session_bindir,
      'canrunheadless': 'true',
    },
  )
  desktop_target = i18n.merge_file(
    type: 'desktop',
    input: desktop_in,
    output: desktop,
    po_dir: po_dir,
    install: true,
    install_dir: session_datadir / 'wayland-sessions',
  )
endforeach

# Install sysvinit session helper scripts

install_data(
'sysvinit/gnome-session-start',
'sysvinit/gnome-session-stop',
install_dir: session_libexecdir,
install_mode: 'rwxr-xr-x',
)

# Install some other misc. configuration files

i18n.merge_file(
  type: 'desktop',
  input: 'gnome.session.desktop',
  output: 'gnome.session',
  po_dir: po_dir,
  install: true,
  install_dir: join_paths(session_pkgdatadir, 'sessions')
)

install_data(
  'org.gnome.SessionManager.gschema.xml',
  install_dir: join_paths(session_datadir, 'glib-2.0', 'schemas'),
)

install_data(
  'gnome-portals.conf',
  install_dir: session_datadir / 'xdg-desktop-portal',
)

if get_option('mimeapps')
  install_data(
    'gnome-mimeapps.list',
    install_dir: session_datadir / 'applications',
  )
endif
