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

#include <gnome.h>
int debug_mode;

static const struct poptOption options[] =
{
	{"debug", '\0', POPT_ARG_NONE, &debug_mode, 0, N_("Enable debugging"), NULL},
        {NULL, '\0', 0, NULL, 0}
};

int
main(int argc, char *argv[])
{
        GnomeProgram *program;
        LoquiApp *app;

        bindtextdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
#ifdef HAVE_BIND_TEXTDOMAIN_CODESET
        bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
#endif
        textdomain(GETTEXT_PACKAGE);

	if(!g_threads_got_initialized)
		g_thread_init (NULL);
	gdk_threads_init();

        program = gnome_program_init(PACKAGE, VERSION,
				     LIBGNOMEUI_MODULE, argc, argv,
				     GNOME_PARAM_POPT_TABLE, options,
				     GNOME_PARAM_HUMAN_READABLE_NAME,
				     _("Loqui IRC client"),
				     GNOME_PARAM_APP_DATADIR, DATADIR,
				     NULL);

        eel_gconf_client_get_global();
	codeconv_init();
	command_table_init();

	if(debug_mode)
		g_print("Start debug mode.\n");

	
	app = loqui_app_get_main_app();
	gtk_widget_show_all(GTK_WIDGET(app));

	gtk_main();

	return 0;
}
