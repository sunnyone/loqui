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
#include <utils.h>

#include "loqui_stock.h"

#include "loqui-core-gtk.h"

#include "intl.h"
#include "loqui-core-gtk.h"

#include <string.h>
#include <loqui.h>
#include <stdlib.h>

int
main(int argc, char *argv[])
{
	int i;
	LoquiCoreGtk *core;

        bindtextdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
#ifdef HAVE_BIND_TEXTDOMAIN_CODESET
        bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
#endif
        textdomain(GETTEXT_PACKAGE);

	g_type_init();

	core = loqui_core_gtk_new();
	loqui_init(LOQUI_CORE(core));
	loqui_core_gtk_initialize_with_args(core, &argc, &argv);

	for(i = 0; i < argc; i++) {
		if(strcmp(argv[i], "--debug") == 0) {
			loqui_core_set_debug_mode(loqui_get_core(), TRUE);
			g_print("Start debug mode.\n");
			continue;
		}
		if(strcmp(argv[i], "--show-msg") == 0) {
			loqui_core_set_show_msg_mode(loqui_get_core(), TRUE);
			g_print("Start show msg mode\n");
			continue;
		}
		if(strcmp(argv[i], "--no-send-status-commands") == 0) {
			loqui_core_set_send_status_commands_mode(loqui_get_core(), FALSE);
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
	loqui_core_gtk_run(core);

	return 0;
}
