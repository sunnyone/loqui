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
#include "utils.h"

PrefsGeneral prefs_general;

PrefElement prefs_general_defs[] = {
	{"save_size", "true", PREF_TYPE_BOOLEAN, &prefs_general.save_size },
	{"window_height","400", PREF_TYPE_UINT, &prefs_general.window_height},
	{"window_width", "480", PREF_TYPE_UINT, &prefs_general.window_width},
	{"channel_tree_height", "180", PREF_TYPE_UINT, &prefs_general.channel_tree_height},
	{"channel_tree_width",  "100", PREF_TYPE_UINT, &prefs_general.channel_tree_width},
	{"common_buffer_height", "150", PREF_TYPE_UINT, &prefs_general.common_buffer_height},

	{"show_statusbar", "true", PREF_TYPE_BOOLEAN, &prefs_general.show_statusbar},
	{"show_channelbar", "true", PREF_TYPE_BOOLEAN, &prefs_general.show_channelbar},

	{"away_message", "Gone.", PREF_TYPE_STRING, &prefs_general.away_message},

	{"browser_command", "mozilla %s", PREF_TYPE_STRING, &prefs_general.browser_command},

	{"use_notification", "true", PREF_TYPE_BOOLEAN, &prefs_general.use_notification },
	{"notification_command", "esdplay /usr/share/sounds/email.wav", PREF_TYPE_STRING, &prefs_general.notification_command},

	{"auto_switch_scrolling", "true", PREF_TYPE_BOOLEAN, &prefs_general.auto_switch_scrolling },

	{"parse_plum_recent", "false", PREF_TYPE_BOOLEAN, &prefs_general.parse_plum_recent },
	
	{"highlight_list", "", PREF_TYPE_STRING_LIST, &prefs_general.highlight_list },

	{"remark_history_number", "10", PREF_TYPE_UINT, &prefs_general.remark_history_number },

	{"common_buffer_max_line_number", "500", PREF_TYPE_UINT, &prefs_general.common_buffer_max_line_number },
	{"channel_buffer_max_line_number", "2000", PREF_TYPE_UINT, &prefs_general.channel_buffer_max_line_number },
	
	{"auto_reconnect", "true", PREF_TYPE_BOOLEAN, &prefs_general.auto_reconnect },
	
	{"use_transparent_ignore", "true", PREF_TYPE_BOOLEAN, &prefs_general.use_transparent_ignore },
	{"transparent_ignore_list", "", PREF_TYPE_STRING_LIST, &prefs_general.transparent_ignore_list },

	{"connect_startup", "false", PREF_TYPE_BOOLEAN, &prefs_general.connect_startup },

	{"time_format", "%H:%M ", PREF_TYPE_STRING, &prefs_general.time_format },

	{"auto_command_mode", "true", PREF_TYPE_BOOLEAN, &prefs_general.auto_command_mode },
	{"command_prefix", "/", PREF_TYPE_STRING, &prefs_general.command_prefix },
	
	{NULL, NULL, 0, NULL}
};

#define RC_FILENAME "loquirc.xml"

static gint in_prefs = 0;
static gboolean prefs_general_initialized = FALSE;
static PrefElement *current_pref_elem = NULL;
static gboolean in_li = FALSE;

static void prefs_general_initialize(void);
static void prefs_general_set_default(void);
static void prefs_general_set_string(PrefElement *elem, const gchar *str);
static gchar* prefs_general_get_string(PrefElement *elem);

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
static void
text_handler           (GMarkupParseContext *context,
                        const gchar         *text,
                        gsize                text_len,
                        gpointer             user_data,
                        GError             **error);

static GMarkupParser parser = {
	start_element_handler,
	end_element_handler,
	text_handler,
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
	const gchar *key = NULL;
	PrefElement *elem;

	if(g_ascii_strcasecmp(element_name, "prefs") == 0) {
		in_prefs++;
		return;
	}

	if(in_prefs < 1) {
		return;
	}

	if(g_strcasecmp(element_name, "li") == 0) {
		in_li = TRUE;
		return;
	}

	current_pref_elem = NULL;

	if(g_strcasecmp(element_name, "entry") != 0) {
		g_warning(_("Invalid element: %s"), element_name);
		return;
	}

	for(i = 0; attribute_names[i] != NULL; i++) {
		if(g_strcasecmp(attribute_names[i], "key") == 0) {
			key = attribute_values[i];
			break;
		} else {
			g_warning(_("prefs_general: Invalid attribute for entry: %s"), attribute_names[i]);
			return;
		}
	}
	elem = g_dataset_get_data(&prefs_general, key);
	if(!elem) {
		g_warning(_("Unknown key: %s\n"), key);
		return;
	}

	current_pref_elem = elem;
}

static void
end_element_handler    (GMarkupParseContext *context,
                        const gchar         *element_name,
                        gpointer             user_data,
                        GError             **error)
{
	if(g_ascii_strcasecmp(element_name, "prefs") == 0) {
		in_prefs--;
	} else if(g_ascii_strcasecmp(element_name, "entry") == 0) {
		current_pref_elem = NULL;
	} else if(g_ascii_strcasecmp(element_name, "li") == 0) {
		in_li = FALSE;
	}
}

static void
text_handler           (GMarkupParseContext *context,
                        const gchar         *text,
                        gsize                text_len,
                        gpointer             user_data,
                        GError             **error)
{
	GList **list_ptr;

	if(!current_pref_elem)
		return;
	if(current_pref_elem->type == PREF_TYPE_STRING_LIST) {
		if(!in_li)
			return;

		list_ptr = (GList **) current_pref_elem->ptr;
		*list_ptr = g_list_append(*list_ptr, g_strdup(text));
	} else {
		prefs_general_set_string(current_pref_elem, text);
	}
}

static void
prefs_general_set_string(PrefElement *elem, const gchar *str)
{
	switch(elem->type) {
	case PREF_TYPE_BOOLEAN:
		if(g_ascii_strcasecmp(str, "true") == 0)
			*((gboolean *) elem->ptr) = TRUE;
		else
			*((gboolean *) elem->ptr) = FALSE;
		break;
	case PREF_TYPE_UINT:
		*((guint *) elem->ptr) = (guint) g_ascii_strtoull(str, NULL, 10);
		break;
	case PREF_TYPE_STRING:
		G_FREE_UNLESS_NULL(*((gchar **) elem->ptr));
		*((gchar **) elem->ptr) = g_strdup(str);
		break;
	case PREF_TYPE_STRING_LIST:
		G_LIST_FREE_WITH_ELEMENT_FREE_UNLESS_NULL(*((GList **) elem->ptr));
		break;
	default:
		g_warning(_("Unsupported pref type!"));
		break;
	}
}
static gchar *
prefs_general_get_string(PrefElement *elem)
{
	gchar *str;

	switch(elem->type) {
	case PREF_TYPE_BOOLEAN:
		if(*((gboolean *)elem->ptr))
			str = g_strdup("true");
		else
			str = g_strdup("false");
		break;
	case PREF_TYPE_UINT:
		str = g_strdup_printf("%d", *((guint *) elem->ptr));
		break;
	case PREF_TYPE_STRING:
		str = g_strdup(*((gchar **) elem->ptr));
		break;
	case PREF_TYPE_STRING_LIST:
		str = NULL;
		break;
	default:
		g_warning(_("Unsupported pref type!"));
		str = g_strdup("UNSUPPORTED TYPE");
		break;
	}

	return str;
}
static
void prefs_general_set_default(void)
{
	gint i;

	for(i = 0; prefs_general_defs[i].name != NULL; i++)
		prefs_general_set_string(&prefs_general_defs[i], prefs_general_defs[i].default_value);

}
static void prefs_general_initialize(void)
{
	gint i;

	if(prefs_general_initialized)
		return;

	memset(&prefs_general, 0, sizeof(prefs_general));

	for(i = 0; prefs_general_defs[i].name != NULL; i++) {
		g_dataset_id_set_data(&prefs_general,
				      g_quark_from_static_string(prefs_general_defs[i].name),
				      &prefs_general_defs[i]);
	}

	prefs_general_initialized = TRUE;
}
void prefs_general_load(void)
{
	gchar *contents;
	gchar *path;
	gsize len;
	GError *error = NULL;
	GMarkupParseContext *context;

	debug_puts("Loading prefs_general...");

	prefs_general_initialize();
	prefs_general_set_default();
	
	path = g_build_filename(g_get_home_dir(), PREFS_DIR, RC_FILENAME, NULL);

	if(!g_file_get_contents(path, &contents, &len, &error)) {
		if(error->code != G_FILE_ERROR_NOENT)
			g_warning("%s", error->message);
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
	gchar *tmp, *escaped;
	GList *cur;
	FILE *fp;
	gint i;

	debug_puts("Saving prefs_general...");

	path = g_build_filename(g_get_home_dir(), PREFS_DIR, RC_FILENAME, NULL);
	if((fp = fopen(path, "w")) == NULL) {
		g_warning(_("Can't open %s: %s"), RC_FILENAME, strerror(errno));
		return;
	}

	/* FIXME: need error checking? */
	fputs("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n", fp);
	fputs("<prefs>\n", fp);

	for(i = 0; prefs_general_defs[i].name != NULL; i++) {
		if(prefs_general_defs[i].type == PREF_TYPE_STRING_LIST) {
			fprintf(fp, "  <entry key=\"%s\">\n", prefs_general_defs[i].name);
			for(cur = *((GList **) prefs_general_defs[i].ptr); cur != NULL; cur = cur->next) {
				escaped = g_markup_escape_text((gchar *) cur->data, -1);
				fprintf(fp, "    <li>%s</li>\n", escaped);
				g_free(escaped);
			}
			fprintf(fp, "  </entry>\n");
		} else {
			tmp = prefs_general_get_string(&prefs_general_defs[i]);
			escaped = g_markup_escape_text(tmp, -1);
			g_free(tmp);
			
			fprintf(fp, "  <entry key=\"%s\">%s</entry>\n", 
				prefs_general_defs[i].name, escaped);
			g_free(escaped);
		}
	}

	fputs("</prefs>", fp);
	fclose(fp);

	debug_puts("Done.");
}
