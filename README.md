# About

This is a port of [gnome-session-openrc](https://github.com/swagtoy/gnome-session-openrc/) to sysvinit systems (e.g. PorteuX/Slackware).

It uses simple shell scripts (`gnome-session-start` and `gnome-session-stop`) to manage session components. The session leader spawns the start script and monitors a FIFO for shutdown signaling.

This requires **elogind** for session/seat management.

The version number tracks the GNOME Session release this works with.

# Building/Installing

This project can be built with the standard `meson` workflow, ensuring the main GNOME session and this project are built with the same `--prefix` setting. Once it is installed, launching `gnome-session` will work.

It's recommended to patch the official GNOME Session with [no gnome-session-ctl/systemd patch](https://raw.githubusercontent.com/porteux/porteux/refs/heads/2.8/003-gnome/gnome/gnome-session/remove-systemd.patch).

Building the official GNOME Session without the patch above might work, but make sure both `gnome-session-ctl` and `gnome-session-init-worker` from this project replace the ones installed in `/usr/libexec`.

# How it works

- **gnome-session-start**: Shell script that starts gnome-session-service, gnome-session-ctl --monitor, gnome-shell, all GSD daemons, and signals init complete
- **gnome-session-stop**: Shell script that kills all session processes by PID file
- **leader-sysvinit.c**: Session leader process started by GDM. Spawns the start script, creates a FIFO, and waits. On SIGTERM from GDM, writes to the FIFO to trigger the monitor's shutdown path
- **gnome-session-ctl**: Handles `--monitor` (watches the FIFO), `--shutdown` (calls gnome-session-stop), and `--signal-init` (tells gnome-session-service that init is done)
