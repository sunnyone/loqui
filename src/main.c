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
#include <loqui_account_manager.h>
#include <prefs_general.h>
#include <utils.h>
#include <loqui_protocol_manager.h>
#include <loqui_protocol_irc.h>
#include <loqui_protocol_ipmsg.h>
#include <loqui_protocol_msn.h>

#include "loqui_stock.h"

#include "loqui_gtk.h"


#include <string.h>
#include <stdlib.h>

#include <gnet.h>
#include <loqui.h>

#include "intl.h"

static void make_program_dir(void);

static void make_program_dir(void)
{
	const gchar *dirname;
	gchar *log_dirname;
	
	dirname = loqui_get_user_dir();
	if(!g_file_test(dirname, G_FILE_TEST_EXISTS)) {
		loqui_utils_mkdir_and_chmod(dirname);
	}
	
	if(!g_file_test(dirname, G_FILE_TEST_IS_DIR)) {
		g_error(_("Invalid \"%s\""), dirname);
	}

	log_dirname = g_build_filename(dirname, LOG_DIR, NULL);
	if(!g_file_test(log_dirname, G_FILE_TEST_EXISTS))
		loqui_utils_mkdir_and_chmod(log_dirname);
	
	g_free(log_dirname);
}

int
main(int argc, char *argv[])
{
	int i;
	LoquiProtocolManager *pmanag;

        bindtextdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
#ifdef HAVE_BIND_TEXTDOMAIN_CODESET
        bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
#endif
        textdomain(GETTEXT_PACKAGE);
	make_program_dir();

	if(!g_threads_got_initialized)
		g_thread_init (NULL);

	gnet_init();

	loqui_gtk_init(&argc, &argv);

	for(i = 0; i < argc; i++) {
		if(strcmp(argv[i], "--debug") == 0) {
			loqui_set_debug_mode(TRUE);
			g_print("Start debug mode.\n");
			continue;
		}
		if(strcmp(argv[i], "--show-msg") == 0) {
			loqui_set_show_msg_mode(TRUE);
			g_print("Start show msg mode\n");
			continue;
		}
		if(strcmp(argv[i], "--no-send-status-commands") == 0) {
			loqui_set_send_status_commands_mode(FALSE);
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

	pmanag = loqui_protocol_manager_new();

	loqui_protocol_manager_register(pmanag, loqui_protocol_irc_get());
	loqui_protocol_manager_register(pmanag, loqui_protocol_ipmsg_get());
	loqui_protocol_manager_register(pmanag, loqui_protocol_msn_get());

	prefs_general_load();
	loqui_gtk_start_main_loop(pmanag);
	prefs_general_save();

	g_object_unref(pmanag);

	return 0;
}
