/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui -- IRC client for GNOME2
 * Copyright (C) 2002 Yoichi Imai <yoichi@silver-forest.com>
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
#include "loqui_gconf.h"
#include "loqui_app.h"
#include "codeconv.h"
#include "command_table.h"
#include "account_manager.h"
#include "prefs_general.h"
#include "utils.h"

#include <gnome.h>

int debug_mode;
int show_msg_mode;

static void make_program_dir(void);

static const struct poptOption options[] =
{
	{"debug", '\0', POPT_ARG_NONE, &debug_mode, 0, N_("Enable debugging"), NULL},
	{"show-msg", '\0', POPT_ARG_NONE, &show_msg_mode, 0, N_("Print messages to console"), NULL},
        {NULL, '\0', 0, NULL, 0}
};

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

int
main(int argc, char *argv[])
{
        GnomeProgram *program;
	AccountManager *account_manager;

        bindtextdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
#ifdef HAVE_BIND_TEXTDOMAIN_CODESET
        bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
#endif
        textdomain(GETTEXT_PACKAGE);

	if(!g_threads_got_initialized)
		g_thread_init (NULL);
	gdk_threads_init();

	make_program_dir();

        program = gnome_program_init(PACKAGE, VERSION,
				     LIBGNOMEUI_MODULE, argc, argv,
				     GNOME_PARAM_POPT_TABLE, options,
				     GNOME_PARAM_HUMAN_READABLE_NAME,
				     _("Loqui IRC client"),
				     GNOME_PARAM_APP_DATADIR, DATADIR,
				     NULL);

	prefs_general_load();

        eel_gconf_client_get_global();
	codeconv_init();
	command_table_init();

	if(debug_mode)
		g_print("Start debug mode.\n");
	if(show_msg_mode)
		g_print("Start show msg mode\n");

	account_manager = account_manager_get();
	account_manager_load_accounts(account_manager);

	gtk_main();

	account_manager_save_accounts(account_manager);
	prefs_general_save();

	return 0;
}
