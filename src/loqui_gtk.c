/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui -- Chat client for Gtk2 <http://loqui.good-day.net/>
 * Copyright (C) 2004 Yoichi Imai <yoichi@silver-forest.com>
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
#include "loqui_gtk.h"
#include "loqui_app.h"

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <gtk24backports.h>

#include "main.h"
#include "loqui_stock.h"
#include "prefs_general.h"

#define ACCEL_MAP_FILE "accelmaprc"

static LoquiApp *app;

void
loqui_gtk_init(int *argc, char **argv[])
{
	gchar *path;

	gdk_threads_init();

	gtk_init(argc, argv);
	gtk24backports_init();

	loqui_stock_init();

	path = g_build_filename(g_get_home_dir(), PREFS_DIR, "gtkrc-2.0", NULL);
	gtk_rc_parse(path);
	g_free(path);
}
void
loqui_gtk_start_main_loop(LoquiProtocolManager *pmanag)
{
	LoquiAccountManager *account_manager;
	gchar *path;
	
	account_manager = loqui_account_manager_new(pmanag);
	app = LOQUI_APP(loqui_app_new(account_manager));
	
	path = g_build_filename(g_get_home_dir(), PREFS_DIR, ACCEL_MAP_FILE, NULL);

	gtk_accel_map_add_filter(SHORTCUT_CHANNEL_ENTRY_ACCEL_MAP_PREFIX "*");
	gtk_accel_map_load(path);

	loqui_account_manager_load_accounts(account_manager);
	if(prefs_general.connect_startup)
		loqui_account_manager_connect_all_default(account_manager);
	g_object_unref(account_manager); /* app has reference */

	gtk_main();

	gtk_accel_map_save(path);
}
