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
#include "prefs_general.h"
#include "utils.h"
#include "main.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "intl.h"

PrefsGeneral *prefs_general = NULL;
#define RC_FILENAME "loquirc.xml"

gint in_prefs = 0;

static void
start_element_handler  (GMarkupParseContext *context,
                        const gchar         *element_name,
                        const gchar        **attribute_names,
                        const gchar        **attribute_values,
                        gpointer             user_data,
                        GError             **error);
static void
end_element_handler    (GMarkupParseContext *context,
                        const gchar         *element_name,
                        gpointer             user_data,
                        GError             **error);

static GMarkupParser parser = {
	start_element_handler,
	end_element_handler,
	NULL,
	NULL,
	NULL
};

static void
start_element_handler  (GMarkupParseContext *context,
                        const gchar         *element_name,
                        const gchar        **attribute_names,
                        const gchar        **attribute_values,
                        gpointer             user_data,
                        GError             **error)
{
	int i;
	const gchar *name = NULL, *value = NULL;

	if(g_ascii_strcasecmp(element_name, "prefs") == 0) {
		in_prefs++;
		return;
	}

	if(in_prefs < 1) {
		return;
	}

	if(g_strcasecmp(element_name, "entry") != 0) {
		g_warning(_("Invalid element: %s"), element_name);
		return;
	}

	for(i = 0; attribute_names[i] != NULL; i++) {
		if(g_strcasecmp(attribute_names[i], "name") == 0)
			name = attribute_values[i];
		else if(g_strcasecmp(attribute_names[i], "value") == 0)
			value = attribute_values[i];
		else
			g_warning(_("prefs_general: Invalid attribute for entry: %s"), attribute_names[i]);
	}

	if(name == NULL || value == NULL) {
		g_warning(_("The entry lacks name or value"));
		return;
	}

#define SET_VALUE_INT(pref_name, dest) { \
   if(g_strcasecmp(name, pref_name) == 0) { \
       dest = (gint) g_ascii_strtoull(value, NULL, 10); \
       debug_puts("prefs_general: %s => %d", pref_name, dest); \
       return; \
  } \
}
#define SET_VALUE_STR(pref_name, dest) { \
   if(g_strcasecmp(name, pref_name) == 0) { \
       dest = g_strdup(value); \
       debug_puts("prefs_general: %s => \"%s\"", pref_name, dest); \
       return; \
  } \
}
	SET_VALUE_INT("codeconv", prefs_general->codeconv);
	SET_VALUE_STR("codeset", prefs_general->codeset);
	SET_VALUE_INT("save_size", prefs_general->save_size);
	SET_VALUE_INT("window_width", prefs_general->window_width);
	SET_VALUE_INT("window_height", prefs_general->window_height);
	SET_VALUE_INT("common_buffer_height", prefs_general->common_buffer_height);
	SET_VALUE_INT("channel_tree_width", prefs_general->channel_tree_width);
	SET_VALUE_INT("channel_tree_height", prefs_general->channel_tree_height);
	SET_VALUE_STR("away_message", prefs_general->away_message);

#undef SET_VALUE_INT
#undef SET_VALUE_STR
}

static void
end_element_handler    (GMarkupParseContext *context,
                        const gchar         *element_name,
                        gpointer             user_data,
                        GError             **error)
{
	if(g_ascii_strcasecmp(element_name, "prefs") == 0) {
		in_prefs--;
	}
}

void prefs_general_set_default(void)
{
	prefs_general->codeconv = 0;
	prefs_general->codeset = g_strdup("");
	prefs_general->save_size = 1;
	prefs_general->window_height = 400;
	prefs_general->window_width = 480;

	prefs_general->channel_tree_height = 180;
	prefs_general->channel_tree_width = 100;

	prefs_general->common_buffer_height = 150;
	prefs_general->away_message = "Gone.";
}

void prefs_general_load(void)
{
	gchar *contents;
	gchar *path;
	gsize len;
	GError *error = NULL;
	GMarkupParseContext *context;

	debug_puts("Loading prefs_general...");

	if(!prefs_general) {
		prefs_general = g_new0(PrefsGeneral, 1);
	} else {
		G_FREE_UNLESS_NULL(prefs_general->codeset);
	}
	prefs_general_set_default();
	
	path = g_build_filename(g_get_home_dir(), PREFS_DIR, RC_FILENAME, NULL);

	if(!g_file_get_contents(path, &contents, &len, &error)) {
		g_warning(_("Can't open %s: %s"), RC_FILENAME, error->message);
		g_error_free(error);
		return;
	}
	g_free(path);
	error = NULL;

	context = g_markup_parse_context_new(&parser, 0, NULL, NULL);
	
	if(!g_markup_parse_context_parse(context, contents, len, &error)) {
		g_warning(_("Loquirc parse error: %s"), error->message);
		g_error_free(error);
		g_markup_parse_context_free(context);
	}
	g_markup_parse_context_free(context);

	debug_puts("Done.");
	return;
}

void prefs_general_save(void)
{
	gchar *path;
	gchar *escaped;
	FILE *fp;

	debug_puts("Saving prefs_general...");

	path = g_build_filename(g_get_home_dir(), PREFS_DIR, RC_FILENAME, NULL);
	if((fp = fopen(path, "w")) == NULL) {
		g_warning(_("Can't open %s: %s"), RC_FILENAME, strerror(errno));
		return;
	}

	/* FIXME: need error checking? */
	fputs("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n", fp);
	fputs("<prefs>\n", fp);

#define WRITE_ENTRY_STR(name, value) { \
   escaped = g_markup_escape_text(value, -1); \
   fprintf(fp, "  <entry name=\"%s\" value=\"%s\" />\n", name, escaped); \
   g_free(escaped); \
}
#define WRITE_ENTRY_INT(name, value) { \
   fprintf(fp, "  <entry name=\"%s\" value=\"%d\" />\n", name, value); \
}
	WRITE_ENTRY_INT("codeconv", prefs_general->codeconv);
	WRITE_ENTRY_STR("codeset", prefs_general->codeset);
	WRITE_ENTRY_INT("save_size", prefs_general->save_size);
	WRITE_ENTRY_INT("window_width", prefs_general->window_width);
	WRITE_ENTRY_INT("window_height", prefs_general->window_height);
	WRITE_ENTRY_INT("common_buffer_height", prefs_general->common_buffer_height);
	WRITE_ENTRY_INT("channel_tree_width", prefs_general->channel_tree_width);
	WRITE_ENTRY_INT("channel_tree_height", prefs_general->channel_tree_height);
	WRITE_ENTRY_STR("away_message", prefs_general->away_message);

	fputs("</prefs>", fp);
	fclose(fp);

	debug_puts("Done.");

#undef WRITE_ENTRY_STR
#undef WRITE_ENTRY_INT

}
