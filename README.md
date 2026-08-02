# About

This is a port of [gnome-session-openrc](https://github.com/swagtoy/gnome-session-openrc/) that lets sysvinit systems (without systemd) boot into the GNOME desktop version 50+.

It uses simple shell scripts (`gnome-session-start` and `gnome-session-stop`) to manage session components. The session leader spawns the start script and monitors a FIFO for shutdown signaling.

**Important**: it doesn't replace the official GNOME Session, so both must be installed for a working GNOME desktop.

# Building

It requires **elogind** for session/seat management and can be built with the `meson` command below (make sure the official GNOME Session and this project are built with the same `--prefix`):

	meson setup \
	--prefix=/usr \
	--buildtype=release \
	--libexecdir=/usr/libexec

It's recommended to patch the official GNOME Session with [no gnome-session-ctl/systemd](https://raw.githubusercontent.com/porteux/porteux/refs/heads/2.8/003-gnome/gnome/gnome-session/remove-systemd.patch) patch. Building without this patch might work, but make sure both `gnome-session-ctl` and `gnome-session-init-worker` from this project replace the ones installed by the official GNOME Session in `/usr/libexec`.

# Usage

Once this project and the official GNOME Session are installed, no extra configuration is needed to boot into the GNOME desktop on a sysvinit system.

# How it works

- **gnome-session-start**: Shell script that starts gnome-session-service, gnome-session-ctl --monitor, gnome-shell and all GSD daemons, then signals that init is complete
- **gnome-session-stop**: Shell script that kills all session processes by PID file
- **leader-sysvinit.c**: Session leader process started by GDM. Spawns the start script, creates a FIFO, and waits. On SIGTERM from GDM, writes to the FIFO to trigger the monitor's shutdown path
- **gnome-session-ctl**: Handles `--monitor` (watches the FIFO), `--shutdown` (calls gnome-session-stop), and `--signal-init` (tells gnome-session-service that init is done)
