/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui -- IRC client for Gtk2 <http://loqui.good-day.net/>
 * Copyright (C) 2003 Yoichi Imai <yoichi@silver-forest.com>
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
#include "prefs_highlight.h"
#include "main.h"
#include "intl.h"
#include "utils.h"

#include <string.h>

PrefsHighlightWords prefs_highlight;
gboolean prefs_highlight_initialized = FALSE;

#define HIGHLIGHT_ALLOW_FILENAME "highlight.allow.txt"

void prefs_highlight_init(void)
{
	if(!prefs_highlight_initialized) {
		prefs_highlight.allow_list = NULL;
	}

	if(prefs_highlight.allow_list) {
		g_slist_foreach(prefs_highlight.allow_list, (GFunc) g_free, NULL);
		g_slist_free(prefs_highlight.allow_list);
		prefs_highlight.allow_list = NULL;
	}

	prefs_highlight_initialized = TRUE;
}
void prefs_highlight_load(void)
{
	gchar *path;
	GError *error = NULL;
	gchar *contents;

	debug_puts("Loading highlighting words...");

	prefs_highlight_init();
	
        path = g_build_filename(g_get_home_dir(), PREFS_DIR, HIGHLIGHT_ALLOW_FILENAME, NULL);
	if(!g_file_get_contents(path, &contents, NULL, &error)) {
		if(error->code != G_FILE_ERROR_NOENT)
			g_warning("%s", error->message);
		g_error_free(error);
		return;
	}
	
	prefs_highlight.allow_list = utils_line_separated_text_to_slist(contents);

	debug_puts("Done.");
}
void prefs_highlight_save(void)
{
	gchar *path;
	GError *error = NULL;
	gchar *buf;
	GIOChannel *ioch;
	gsize len;

	if(prefs_highlight.allow_list == NULL)
		return;

	debug_puts("Saving highlighting words...");
	
	path = g_build_filename(g_get_home_dir(), PREFS_DIR, HIGHLIGHT_ALLOW_FILENAME, NULL);
	if((ioch = g_io_channel_new_file(path, "w", &error)) == NULL) {
		g_warning("%s", error->message);
		g_error_free(error);
		return;
	}

	buf = utils_line_separated_text_from_slist(prefs_highlight.allow_list);

	if(g_io_channel_write_chars(ioch, buf, -1, &len, &error) == G_IO_STATUS_ERROR) {
		g_warning("%s", error->message);
		g_error_free(error);
		return;
	}
	g_io_channel_unref(ioch);
	g_free(buf);

	debug_puts("Done.");
}
