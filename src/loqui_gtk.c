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

static void make_accel_map_entries_for_channel_shortcutkeys(void);

/* temporary implementation */
static void
make_accel_map_entries_for_channel_shortcutkeys(void)
{
	int i;
	gchar *path;
	guint key = 0;
	GdkModifierType mods = 0;
	
	for (i = 0; i <= MAX_SHORTCUT_CHANNEL_NUMBER; i++) {
		if (i < 10) {
			key = GDK_0 + i;
			mods = GDK_CONTROL_MASK;
		} else if (i < 20) {
			key = GDK_0 + i - 10;
			mods = GDK_MOD1_MASK;
		} else {
			g_assert_not_reached();
		}
		path = g_strdup_printf(SHORTCUT_CHANNEL_ACCEL_MAP_PREFIX "%d", i);
		gtk_accel_map_add_entry(path, key, mods);
		g_free(path);
	}
}

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

	make_accel_map_entries_for_channel_shortcutkeys();
}
void
loqui_gtk_start_main_loop(void)
{
	AccountManager *account_manager;
	LoquiApp *app;

	account_manager = account_manager_new();
	app = LOQUI_APP(loqui_app_new(account_manager));
	account_manager_load_accounts(account_manager);
	if(prefs_general.connect_startup)
		account_manager_connect_all_default(account_manager);
	g_object_unref(account_manager); /* app has reference */

	gtk_main();
}
