/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui -- IRC client for Gtk2 <http://loqui.good-day.net/>
 * Copyright (C) 2002-2003 Yoichi Imai <yoichi@silver-forest.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#include "config.h"
#include "main.h"
#include "loqui_app.h"
#include "codeconv.h"
#include "command_table.h"
#include "account_manager.h"
#include "prefs_general.h"
#include "utils.h"

#include "icons/pixbufs.h"

#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>

#include <gnet.h>

#include "intl.h"

int debug_mode;
int show_msg_mode;
int send_status_commands_mode;

struct icon_pair {
	const gchar *id;
	const guint8 *inline_pixbuf;
};
static struct icon_pair icon_list[] = { { "loqui-console", console_pixbuf },
					{ "loqui-online", online_pixbuf },
					{ "loqui-offline", offline_pixbuf },
					{ "loqui-away", away_pixbuf },
					{ "loqui-busy", busy_pixbuf },
					{ NULL, NULL } };

static void make_program_dir(void);
static void make_icons(void);

static void make_program_dir(void)
{
	gchar *dirname;

	dirname = g_build_filename(g_get_home_dir(), PREFS_DIR, NULL);
	if(!g_file_test(dirname, G_FILE_TEST_EXISTS)) {
		make_dir(dirname);
	}

	if(!g_file_test(dirname, G_FILE_TEST_IS_DIR)) {
		g_error(_("Invalid \"%s\""), dirname);
	}
	g_free(dirname);
}

static void
make_icons(void)
{
	GdkPixbuf *pixbuf;
	GtkIconSet *icon_set;
	GtkIconFactory *icon_factory;
	int i;

	icon_factory = gtk_icon_factory_new();
	gtk_icon_factory_add_default(icon_factory);

	for(i = 0; icon_list[i].id != NULL; i++) {
		pixbuf = gdk_pixbuf_new_from_inline(-1, icon_list[i].inline_pixbuf, FALSE, NULL);
		if(pixbuf == NULL)
			continue;
		icon_set = gtk_icon_set_new_from_pixbuf(pixbuf);
		g_object_unref(pixbuf);
		
		gtk_icon_factory_add(icon_factory, icon_list[i].id, icon_set);
	}
	g_object_unref(icon_factory);
}

int
main(int argc, char *argv[])
{
	AccountManager *account_manager;
	int i;
	gchar *path;

        bindtextdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
#ifdef HAVE_BIND_TEXTDOMAIN_CODESET
        bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
#endif
        textdomain(GETTEXT_PACKAGE);

	if(!g_threads_got_initialized)
		g_thread_init (NULL);
	gdk_threads_init();

	make_program_dir();
	
	gnet_init();
	gtk_init(&argc, &argv);

	make_icons();

	path = g_build_filename(g_get_home_dir(), PREFS_DIR, "gtkrc-2.0", NULL);
	gtk_rc_parse(path);
	g_free(path);

	send_status_commands_mode = 1;
	
	for(i = 0; i < argc; i++) {
		if(strcmp(argv[i], "--debug") == 0) {
			debug_mode = 1;
			g_print("Start debug mode.\n");
			continue;
		}
		if(strcmp(argv[i], "--show-msg") == 0) {
			show_msg_mode = 1;
			g_print("Start show msg mode\n");
			continue;
		}
		if(strcmp(argv[i], "--no-send-status-commands") == 0) {
			send_status_commands_mode = 0;
			g_print("Loqui doesn't send commands to get status like MODE, WHO in this session\n");
			continue;
		}
		if(strcmp(argv[i], "--help") == 0) {
			g_print("Loqui %s\n", VERSION);
			g_print(_("  --no-send-status-commands don't send commands to get status (like MODE, WHO)"));
			g_print(_("  --debug                   debug mode\n"));
			g_print(_("  --show-msg                show message mode\n"));
			g_print(_("  --help                    show this help\n"));
			exit(0);
		}
	}
	prefs_general_load();

	command_table_init();

	account_manager = account_manager_get();
	account_manager_load_accounts(account_manager);
	account_manager_set_whether_scrolling(account_manager, TRUE);
	if(prefs_general.connect_startup)
		account_manager_connect_all_default(account_manager);

	gtk_main();
	prefs_general_save();

	return 0;
}
