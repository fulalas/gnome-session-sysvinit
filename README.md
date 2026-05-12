# About

This is a port of [gnome-session-openrc](https://github.com/swagtoy/gnome-session-openrc/) to sysvinit systems (e.g. PorteuX/Slackware).

It uses simple shell scripts (`gnome-session-start` and `gnome-session-stop`) to manage session components. The session leader spawns the start script and monitors a FIFO for shutdown signaling.

# Building/Installing

First, this requires GNOME Session with [swagtoy's no-systemd patch](https://gitlab.gnome.org/swagtoy/gnome-session/-/commit/419191d3897957bd8cd325f2167f3c8663969a13), [Dudemanguy's elogind patch](https://gitlab.gnome.org/GNOME/gnome-session/-/merge_requests/106), or any other means of disabling the official `gnome-session-ctl`.

This project can be built with the standard `meson` workflow, ensuring the main GNOME session and this project are built with the same `--prefix` setting. Once it is installed, launching `gnome-session` will work.

# How it works

- **gnome-session-start**: Shell script that starts gnome-session-service, gnome-session-ctl --monitor, gnome-shell, all GSD daemons, and signals init complete
- **gnome-session-stop**: Shell script that kills all session processes by PID file
- **leader-sysvinit.c**: Session leader process started by GDM. Spawns the start script, creates a FIFO, and waits. On SIGTERM from GDM, writes to the FIFO to trigger the monitor's shutdown path
- **gnome-session-ctl**: Handles `--monitor` (watches the FIFO), `--shutdown` (calls gnome-session-stop), and `--signal-init` (tells gnome-session-service that init is done)
