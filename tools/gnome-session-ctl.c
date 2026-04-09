/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*-
 * gnome-session-ctl.c - Small utility program to manage gnome session.

   Copyright (C) 1998 Tom Tromey
   Copyright (C) 2008,2019 Red Hat, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, see <http://www.gnu.org/licenses/>.
*/

#include <config.h>

#include <locale.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <glib.h>
#include <glib-unix.h>
#include <glib/gstdio.h>
#include <glib/gi18n.h>
#include <gio/gio.h>

#define GSM_SERVICE_DBUS   "org.gnome.SessionManager"
#define GSM_PATH_DBUS      "/org/gnome/SessionManager"
#define GSM_INTERFACE_DBUS "org.gnome.SessionManager"

static gboolean
async_run_cmd (gchar **argv, GError **error)
{
        return g_spawn_async (NULL, argv, NULL,
                              G_SPAWN_DEFAULT,
                              NULL, NULL, NULL, error);
}

static gboolean
sysvinit_run_session_stop (GError **error)
{
        g_autofree char *stop_script = g_build_filename ("/usr/libexec",
                                                         "gnome-session-stop",
                                                         NULL);
        gchar *argv[] = { stop_script, NULL };
        return async_run_cmd (argv, error);
}

static GDBusConnection *
get_session_bus (void)
{
        g_autoptr(GError) error = NULL;
        GDBusConnection *bus;

        bus = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, &error);

        if (bus == NULL)
                g_warning ("Couldn't connect to session bus: %s", error->message);

        return bus;
}

static void
do_signal_init (void)
{
        g_autoptr(GDBusConnection) connection = NULL;
        g_autoptr(GVariant) reply = NULL;
        g_autoptr(GError) error = NULL;

        connection = get_session_bus ();
        if (connection == NULL)
                return;

        reply = g_dbus_connection_call_sync (connection,
                                             GSM_SERVICE_DBUS,
                                             GSM_PATH_DBUS,
                                             GSM_INTERFACE_DBUS,
                                             "Initialized",
                                             NULL,
                                             NULL,
                                             G_DBUS_CALL_FLAGS_NO_AUTO_START,
                                             -1, NULL, &error);

        if (error != NULL)
                g_warning ("Failed to call signal initialization: %s",
                           error->message);
}

typedef struct {
        GMainLoop *loop;
        gint fifo_fd;
} MonitorLeader;

static gboolean
leader_term_or_int_signal_cb (gpointer user_data)
{
        MonitorLeader *data = (MonitorLeader*) user_data;

        g_main_loop_quit (data->loop);

        return G_SOURCE_REMOVE;
}

static gboolean
leader_fifo_io_cb (gint fd,
                   GIOCondition condition,
                   gpointer user_data)
{
        MonitorLeader *data = (MonitorLeader*) user_data;

        if (condition & G_IO_IN) {
                char buf[1];
                read (data->fifo_fd, buf, 1);
                g_main_loop_quit (data->loop);
        }

        if (condition & G_IO_HUP) {
                g_main_loop_quit (data->loop);
        }

        return G_SOURCE_CONTINUE;
}

/**
 * do_monitor_leader:
 *
 * Function to monitor the leader to ensure clean session shutdown and
 * propagation of this information to/from GDM.
 * See leader-sysvinit.c for more information.
 */
static void
do_monitor_leader (void)
{
        MonitorLeader data;
        g_autofree char *fifo_name = NULL;
        int res;

        data.loop = g_main_loop_new (NULL, TRUE);

        const char *session_rundir = g_getenv ("GNOME_SESSION_RUNDIR");
        if (!session_rundir)
                session_rundir = g_get_user_runtime_dir ();
        fifo_name = g_strdup_printf ("%s/gnome-session-leader-fifo", session_rundir);
        res = mkfifo (fifo_name, 0666);
        if (res < 0 && errno != EEXIST)
                g_warning ("Error creating FIFO: %m");

        data.fifo_fd = g_open (fifo_name, O_RDONLY | O_CLOEXEC, 0666);
        if (data.fifo_fd >= 0) {
                struct stat buf;

                res = fstat (data.fifo_fd, &buf);
                if (res < 0) {
                        g_warning ("Unable to monitor session leader: stat failed with error %m");
                        close (data.fifo_fd);
                        data.fifo_fd = -1;
                } else if (!(buf.st_mode & S_IFIFO)) {
                        g_warning ("Unable to monitor session leader: FD is not a FIFO");
                        close (data.fifo_fd);
                        data.fifo_fd = -1;
                } else {
                        g_debug ("Watching session leader");
                        g_unix_fd_add (data.fifo_fd, G_IO_HUP | G_IO_IN, leader_fifo_io_cb, &data);
                }
        } else {
                g_warning ("Unable to monitor session leader: Opening FIFO failed with %m");
        }

        g_unix_signal_add (SIGTERM, leader_term_or_int_signal_cb, &data);
        g_unix_signal_add (SIGINT, leader_term_or_int_signal_cb, &data);

        g_main_loop_run (data.loop);

        g_main_loop_unref (data.loop);
        /* FD is closed with the application. */
}

int
main (int argc, char *argv[])
{
        g_autoptr(GError) error = NULL;
        static gboolean   opt_shutdown;
        static gboolean   opt_monitor;
        static gboolean   opt_signal_init;
        int     conflicting_options;
        GOptionContext *ctx;
        static const GOptionEntry options[] = {
                { "shutdown", '\0', 0, G_OPTION_ARG_NONE, &opt_shutdown, N_("Run gnome-session-stop to shut down the session"), NULL },
                { "monitor", '\0', 0, G_OPTION_ARG_NONE, &opt_monitor, N_("Monitor the session leader FIFO and shut down on EOF or a single byte"), NULL },
                { "signal-init", '\0', 0, G_OPTION_ARG_NONE, &opt_signal_init, N_("Signal initialization done to gnome-session"), NULL },
                { NULL },
        };

        /* Initialize the i18n stuff */
        setlocale (LC_ALL, "");
        bindtextdomain (GETTEXT_PACKAGE, LOCALE_DIR);
        bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
        textdomain (GETTEXT_PACKAGE);

        ctx = g_option_context_new ("");
        g_option_context_add_main_entries (ctx, options, GETTEXT_PACKAGE);
        if (! g_option_context_parse (ctx, &argc, &argv, &error)) {
                g_warning ("Unable to start: %s", error->message);
                exit (1);
        }
        g_option_context_free (ctx);

        conflicting_options = 0;
        if (opt_shutdown)
                conflicting_options++;
        if (opt_monitor)
                conflicting_options++;
        if (opt_signal_init)
                conflicting_options++;
        if (conflicting_options != 1) {
                g_printerr (_("Program needs exactly one parameter"));
                exit (1);
        }

        if (opt_signal_init) {
                do_signal_init ();
        } else if (opt_shutdown) {
                if (!sysvinit_run_session_stop (&error))
                        g_error ("Failed to run session stop: %s",
                                 error ? error->message : "(no message)");
        } else if (opt_monitor) {
                do_monitor_leader ();
                if (!sysvinit_run_session_stop (&error))
                        g_warning ("Failed to run session stop: %s",
                                   error ? error->message : "(no message)");
        } else {
                g_assert_not_reached ();
        }

        return 0;
}
