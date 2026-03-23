# About

This is a port of [gnome-session-openrc](https://github.com/swagtoy/gnome-session-openrc/) to sysvinit systems (e.g. PorteuX/Slackware).

It uses simple shell scripts (`gnome-session-start` and `gnome-session-stop`) to manage session components. The session leader spawns the start script and monitors a FIFO for shutdown signaling.

This requires **elogind** for session/seat management.

# Applying the changes

Simply clone this repository and copy/replace its content to `gnome-session` source code, and build it normally.

# How it works

- **gnome-session-start**: Shell script that starts gnome-session-service, gnome-session-ctl --monitor, gnome-shell, all GSD daemons, and signals init complete
- **gnome-session-stop**: Shell script that kills all session processes by PID file
- **leader-sysvinit.c**: Session leader process started by GDM. Spawns the start script, creates a FIFO, and waits. On SIGTERM from GDM, writes to the FIFO to trigger the monitor's shutdown path
- **gnome-session-ctl**: Handles `--monitor` (watches the FIFO), `--shutdown` (calls gnome-session-stop), and `--signal-init` (tells gnome-session-service that init is done)
