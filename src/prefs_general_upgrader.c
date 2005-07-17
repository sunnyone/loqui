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
#include "prefs_general_upgrader.h"
#include <libloqui/loqui-utils.h>

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <glib/gi18n.h>
#include <libloqui/loqui-utils.h>
#include "loqui.h"

#include "loqui-core-gtk.h"
#include "loqui-general-pref-gtk-groups.h"
#include "loqui-general-pref-gtk-default.h"

typedef struct _PrefElementForUpgrade PrefElementForUpgrade;

typedef enum {
	PREF_TYPE_FOR_UPGRADE_NONE,
	PREF_TYPE_FOR_UPGRADE_BOOLEAN,
	PREF_TYPE_FOR_UPGRADE_INT,
	PREF_TYPE_FOR_UPGRADE_UINT,
	PREF_TYPE_FOR_UPGRADE_STRING,
	PREF_TYPE_FOR_UPGRADE_STRING_LIST = 101
} PrefTypeForUpgrade;

#define PREF_TYPE_FOR_UPGRADE_IS_LIST(type) (type >= 100)

struct _PrefElementForUpgrade {
	gchar *name;
	PrefTypeForUpgrade type;
	gchar *group;
	gchar *key;
};

PrefElementForUpgrade prefs_general_defs[] = {
	{"away_message", PREF_TYPE_FOR_UPGRADE_STRING, "Messages", "AwayMessage"},

	{"auto_reconnect", PREF_TYPE_FOR_UPGRADE_BOOLEAN, "Account", "AutoReconnect"},

	{"use_notification", PREF_TYPE_FOR_UPGRADE_BOOLEAN, "Notification", "UseNotification"},
	{"exec_notification_by_notice", PREF_TYPE_FOR_UPGRADE_BOOLEAN, "Notification", "ExecNotificationByNotice"},
	{"highlight_list", PREF_TYPE_FOR_UPGRADE_STRING_LIST, "Notification", "HighlightList"},

	{"use_transparent_ignore", PREF_TYPE_FOR_UPGRADE_BOOLEAN, "Ignore", "UseTransparentIgnore"},
	{"transparent_ignore_list", PREF_TYPE_FOR_UPGRADE_STRING_LIST, "Ignore", "TransparentIgnoreList"},

	{"save_size", PREF_TYPE_FOR_UPGRADE_BOOLEAN, "Gtk.Size", "SaveSize"},
	{"window_height", PREF_TYPE_FOR_UPGRADE_UINT, "Gtk.Size", "WindowHeight"},
	{"window_width", PREF_TYPE_FOR_UPGRADE_UINT, "Gtk.Size", "WindowWidth"},
	{"channel_tree_height", PREF_TYPE_FOR_UPGRADE_UINT, "Gtk.Size", "ChannelTreeHeight"},
	{"channel_tree_width", PREF_TYPE_FOR_UPGRADE_UINT, "Gtk.Size", "ChannelTreeWidth"},
	{"common_buffer_height", PREF_TYPE_FOR_UPGRADE_UINT, "Gtk.Size", "CommonBufferHeight"},

	{"show_statusbar", PREF_TYPE_FOR_UPGRADE_BOOLEAN, "Gtk.Visibility", "ShowStatusbar"},
	{"show_channelbar", PREF_TYPE_FOR_UPGRADE_BOOLEAN, "Gtk.Visibility", "ShowChannelbar"},

	{"title_format_title", PREF_TYPE_FOR_UPGRADE_STRING, "Gtk.TitleFormat", "TitleFormatTitle"},
	{"title_format_statusbar", PREF_TYPE_FOR_UPGRADE_STRING,"Gtk.TitleFormat" , "TitleFormatStatusbar"},

	{"browser_command", PREF_TYPE_FOR_UPGRADE_STRING, "Gtk.Commands", "BrowserCommand"},
	{"notification_command", PREF_TYPE_FOR_UPGRADE_STRING, "Gtk.Commands", "NotificationCommand"},

	{"auto_switch_scrolling", PREF_TYPE_FOR_UPGRADE_BOOLEAN, "Gtk.General", "AutoSwitchScrolling"},
	{"auto_switch_scrolling_common_buffer", PREF_TYPE_FOR_UPGRADE_BOOLEAN, "Gtk.General", "AutoSwitchScrollingCommonBuffer"},

	{"parse_plum_recent", PREF_TYPE_FOR_UPGRADE_BOOLEAN, "Proxy", "ParsePlumRecent"},

	{"remark_history_number", PREF_TYPE_FOR_UPGRADE_UINT, "Gtk.General", "RemarkHistoryNumber"},
	{"auto_command_mode", PREF_TYPE_FOR_UPGRADE_BOOLEAN, "Gtk.General", "AutoCommandMode"},
	{"command_prefix", PREF_TYPE_FOR_UPGRADE_STRING, "Gtk.General", "CommandPrefix"},

	{"common_buffer_max_line_number", PREF_TYPE_FOR_UPGRADE_UINT, "Gtk.General", "CommonBufferMaxLineNumber"},
	{"channel_buffer_max_line_number", PREF_TYPE_FOR_UPGRADE_UINT, "Gtk.General", "ChannelBufferMaxLineNumber"},
	
	{"connect_startup", PREF_TYPE_FOR_UPGRADE_BOOLEAN, "Gtk.General", "ConnectStartup"},
	{"time_format", PREF_TYPE_FOR_UPGRADE_STRING, "Gtk.General", "TimeFormat"},
	{"save_log", PREF_TYPE_FOR_UPGRADE_BOOLEAN, "Gtk.General", "SaveLog"},
	{"nick_list_sort_type", PREF_TYPE_FOR_UPGRADE_UINT, "Gtk.General", "NickListSortType"},
	{"select_channel_joined", PREF_TYPE_FOR_UPGRADE_BOOLEAN, "Gtk.General", "SelectChannelJoined"},

	{NULL, 0, NULL, NULL}
};

#define RC_FILENAME "loquirc.xml"

static gint in_prefs = 0;
static PrefElementForUpgrade *current_pref_elem = NULL;
static gboolean in_li = FALSE;

static void prefs_general_set_string(PrefElementForUpgrade *elem, const gchar *str);

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
	PrefElementForUpgrade *elem;

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

	elem = NULL;

	for (i = 0; prefs_general_defs[i].name != NULL; i++) {
		if (strcmp(prefs_general_defs[i].name, key) == 0) {
			elem = &prefs_general_defs[i];
			break;
		}
	}

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
	gchar **strarray_old;
	gchar **strarray;
	gsize len;
	int i;

	if(!current_pref_elem)
		return;

	if(current_pref_elem->type == PREF_TYPE_FOR_UPGRADE_STRING_LIST) {
		if(!in_li)
			return;
		/* inefficient but enough to upgrade */
		strarray_old = loqui_pref_get_string_list(loqui_get_general_pref(), current_pref_elem->group, current_pref_elem->key, &len, NULL);
		if (strarray_old) {
			strarray = g_new0(gchar *, len + 1);
			for (i = 0; i < len; i++)
				strarray[i] = strarray_old[i];
			strarray[i] = (gchar*) text;
			loqui_pref_set_string_list(loqui_get_general_pref(), current_pref_elem->group, current_pref_elem->key, strarray, len + 1);

			g_strfreev(strarray_old);
		} else {
			strarray = g_new0(gchar *, 2);
			strarray[0] = (gchar *) text;
			loqui_pref_set_string_list(loqui_get_general_pref(), current_pref_elem->group, current_pref_elem->key, strarray, 1);
		}
	} else {
		prefs_general_set_string(current_pref_elem, text);
	}
}

static void
prefs_general_set_string(PrefElementForUpgrade *elem, const gchar *str)
{
	gboolean boolean;
	switch(elem->type) {
	case PREF_TYPE_FOR_UPGRADE_BOOLEAN:
		boolean = (g_ascii_strcasecmp(str, "true") == 0);
		loqui_pref_set_boolean(loqui_get_general_pref(), elem->group, elem->key, boolean);
		break;
	case PREF_TYPE_FOR_UPGRADE_UINT:
		loqui_pref_set_integer(loqui_get_general_pref(), elem->group, elem->key, (gint) g_ascii_strtoull(str, NULL, 10));
		break;
	case PREF_TYPE_FOR_UPGRADE_STRING:
		loqui_pref_set_string(loqui_get_general_pref(), elem->group, elem->key, str);
		break;
	case PREF_TYPE_FOR_UPGRADE_STRING_LIST:
		break;
	default:
		g_warning(_("Unsupported pref type: %s"), elem->name);
		break;
	}
}
void prefs_general_upgrader_upgrade(void)
{
	gchar *contents;
	gchar *path;
	gchar *backup_path;
	gsize len;
	GError *error = NULL;
	GMarkupParseContext *context;

	g_print("Upgrading prefs_general...\n");
	
	path = g_build_filename(loqui_core_get_user_dir(loqui_get_core()), RC_FILENAME, NULL);

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

	backup_path = g_strconcat(path, "~", NULL);
	rename(path, backup_path);
	g_free(backup_path);

	g_print("Done.\n");
	return;
}
