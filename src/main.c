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
#include "codeconv.h"
#include "command_table.h"
#include "account_manager.h"
#include "prefs_general.h"
#include "utils.h"
#include "loqui_stock.h"

#include "loqui_gtk.h"

#include <string.h>
#include <stdlib.h>

#include <gnet.h>

#include "intl.h"

int debug_mode;
int show_msg_mode;
int send_status_commands_mode;

static void make_program_dir(void);

static void make_program_dir(void)
{
	gchar *dirname;
	gchar *log_dirname;
	
	dirname = g_build_filename(g_get_home_dir(), PREFS_DIR, NULL);
	if(!g_file_test(dirname, G_FILE_TEST_EXISTS)) {
		make_dir(dirname);
	}
	
	if(!g_file_test(dirname, G_FILE_TEST_IS_DIR)) {
		g_error(_("Invalid \"%s\""), dirname);
	}
	g_free(dirname);

	log_dirname = g_build_filename(g_get_home_dir(), PREFS_DIR, LOG_DIR, NULL);
	if(!g_file_test(log_dirname, G_FILE_TEST_EXISTS))
		make_dir(log_dirname);
	
	g_free(log_dirname);
}

int
main(int argc, char *argv[])
{
	int i;

        bindtextdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
#ifdef HAVE_BIND_TEXTDOMAIN_CODESET
        bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
#endif
        textdomain(GETTEXT_PACKAGE);

	make_program_dir();

	if(!g_threads_got_initialized)
		g_thread_init (NULL);

	gnet_init();

	command_table_init();
	loqui_gtk_init(&argc, &argv);

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
	loqui_gtk_start_main_loop();
	prefs_general_save();

	return 0;
}
