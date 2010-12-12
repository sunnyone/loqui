/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui -- IRC client for Gtk2 <http://launchpad.net/loqui/>
 * Copyright (C) 2003 Yoichi Imai <sunnyone41@gmail.com>
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

#include <libloqui/loqui-utils.h>

#include "prefs_dialog.h"
#include <glib/gi18n.h>
#include "gtkutils.h"
#include "loqui_app.h"

#include <string.h>
#include <loqui.h>
#include <loqui-general-pref-groups.h>
#include <loqui-general-pref-default.h>
#include "loqui-general-pref-gtk-groups.h"
#include "loqui-general-pref-gtk-default.h"
#include "loqui-core-gtk.h"

struct _PrefsDialogPrivate
{
	GtkWidget *check_save_size;
	GtkWidget *check_auto_switch_scrolling;
	GtkWidget *check_auto_switch_scrolling_common_buffer;
	GtkWidget *check_auto_reconnect;
	GtkWidget *check_connect_startup;
	GtkWidget *check_select_channel_joined;
	GtkWidget *entry_away_message;
	GtkWidget *entry_time_format;
	
	GtkWidget *spin_common_buffer_max_line_number;
	GtkWidget *spin_channel_buffer_max_line_number;

	GtkWidget *check_auto_command_mode;
	GtkWidget *entry_command_prefix;
	
	GtkWidget *check_save_log;
	
	GtkWidget *check_parse_plum_recent;
	GtkWidget *entry_recent_log_regexp;
	GtkWidget *check_treat_as_recent_log_until_first_pong_received;
	
	GtkWidget *check_use_notification;
	GtkWidget *check_exec_notification_by_notice;
	GtkWidget *textview_highlight;

	GtkWidget *check_use_transparent_ignore;
	GtkWidget *textview_transparent_ignore;

	GtkWidget *check_use_normal_ignore;
	GtkWidget *textview_normal_ignore;

	GtkWidget *entry_browser_command;
	GtkWidget *entry_notification_command;

	GtkWidget *textview_title_format_title;
	GtkWidget *textview_title_format_statusbar;

	LoquiApp *app;
};

static GtkDialogClass *parent_class = NULL;
#define PARENT_TYPE GTK_TYPE_DIALOG

static void prefs_dialog_class_init(PrefsDialogClass *klass);
static void prefs_dialog_init(PrefsDialog *prefs_dialog);
static void prefs_dialog_finalize(GObject *object);
static void prefs_dialog_destroy(GtkObject *object);

static void prefs_dialog_load_settings(PrefsDialog *dialog);
static void prefs_dialog_save_settings(PrefsDialog *dialog);

static void prefs_dialog_response_cb(GtkWidget *widget, gint response, gpointer data);

GType
prefs_dialog_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(PrefsDialogClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) prefs_dialog_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(PrefsDialog),
				0,              /* n_preallocs */
				(GInstanceInitFunc) prefs_dialog_init
			};
		
		type = g_type_register_static(PARENT_TYPE,
					      "PrefsDialog",
					      &our_info,
					      0);
	}
	
	return type;
}
static void
prefs_dialog_class_init (PrefsDialogClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);
        GtkObjectClass *gtk_object_class = GTK_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = prefs_dialog_finalize;
        gtk_object_class->destroy = prefs_dialog_destroy;
}
static void 
prefs_dialog_init (PrefsDialog *prefs_dialog)
{
	PrefsDialogPrivate *priv;

	priv = g_new0(PrefsDialogPrivate, 1);

	prefs_dialog->priv = priv;
}
static void 
prefs_dialog_finalize (GObject *object)
{
	PrefsDialog *prefs_dialog;

        g_return_if_fail(object != NULL);
        g_return_if_fail(IS_PREFS_DIALOG(object));

        prefs_dialog = PREFS_DIALOG(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize) (object);

	g_free(prefs_dialog->priv);
}
static void 
prefs_dialog_destroy (GtkObject *object)
{
        PrefsDialog *prefs_dialog;

        g_return_if_fail(object != NULL);
        g_return_if_fail(IS_PREFS_DIALOG(object));

        prefs_dialog = PREFS_DIALOG(object);

        if (GTK_OBJECT_CLASS(parent_class)->destroy)
                (* GTK_OBJECT_CLASS(parent_class)->destroy) (object);
}
static void
prefs_dialog_load_settings(PrefsDialog *dialog)
{
	PrefsDialogPrivate *priv;
	GtkTextBuffer *buffer;
	LoquiPref *general_pref;
	GList *list;
	gchar *buf;
	
	g_return_if_fail(dialog != NULL);
        g_return_if_fail(IS_PREFS_DIALOG(dialog));

	priv = dialog->priv;

	general_pref = loqui_get_general_pref();

#define SET_CHECKBOX(_check, _group, _key, _default) { \
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(_check), \
				     loqui_pref_get_with_default_boolean(general_pref, _group, _key, _default, NULL)); \
}
	SET_CHECKBOX(priv->check_auto_switch_scrolling,
		     LOQUI_GENERAL_PREF_GTK_GROUP_GENERAL, "AutoSwitchScrolling", LOQUI_GENERAL_PREF_GTK_DEFAULT_GENERAL_AUTO_SWITCH_SCROLLING);
	SET_CHECKBOX(priv->check_auto_switch_scrolling_common_buffer,
		     LOQUI_GENERAL_PREF_GTK_GROUP_GENERAL, "AutoSwitchScrollingCommonBuffer", LOQUI_GENERAL_PREF_GTK_DEFAULT_GENERAL_AUTO_SWITCH_SCROLLING_COMMON_BUFFER);
	SET_CHECKBOX(priv->check_parse_plum_recent, LOQUI_GENERAL_PREF_GROUP_PROXY, "ParsePlumRecent", LOQUI_GENERAL_PREF_DEFAULT_PROXY_PARSE_PLUM_RECENT);
	SET_CHECKBOX(priv->check_save_size,         LOQUI_GENERAL_PREF_GTK_GROUP_SIZE, "SaveSize", LOQUI_GENERAL_PREF_GTK_DEFAULT_SIZE_SAVE_SIZE);
	SET_CHECKBOX(priv->check_treat_as_recent_log_until_first_pong_received,
		     LOQUI_GENERAL_PREF_GROUP_PROXY, "TreatAsRecentLogUntilFirstPongReceived",
		     LOQUI_GENERAL_PREF_DEFAULT_PROXY_TREAT_AS_RECENT_LOG_UNTIL_FIRST_PONG_RECEIVED);
	SET_CHECKBOX(priv->check_use_notification,  LOQUI_GENERAL_PREF_GROUP_NOTIFICATION, "UseNotification", LOQUI_GENERAL_PREF_DEFAULT_NOTIFICATION_USE_NOTIFICATION);
	SET_CHECKBOX(priv->check_exec_notification_by_notice,
		     LOQUI_GENERAL_PREF_GROUP_NOTIFICATION, "ExecNotificationByNotice", LOQUI_GENERAL_PREF_DEFAULT_NOTIFICATION_EXEC_NOTIFICATION_BY_NOTICE);
	SET_CHECKBOX(priv->check_use_normal_ignore, LOQUI_GENERAL_PREF_GROUP_IGNORE, "UseNormalIgnore", LOQUI_GENERAL_PREF_DEFAULT_IGNORE_USE_NORMAL_IGNORE);
	SET_CHECKBOX(priv->check_use_transparent_ignore, LOQUI_GENERAL_PREF_GROUP_IGNORE, "UseTransparentIgnore", LOQUI_GENERAL_PREF_DEFAULT_IGNORE_USE_TRANSPARENT_IGNORE);
	SET_CHECKBOX(priv->check_auto_reconnect, LOQUI_GENERAL_PREF_GROUP_ACCOUNT, "AutoReconnect", LOQUI_GENERAL_PREF_DEFAULT_ACCOUNT_AUTO_RECONNECT);
	SET_CHECKBOX(priv->check_connect_startup, LOQUI_GENERAL_PREF_GTK_GROUP_GENERAL, "ConnectStartup", LOQUI_GENERAL_PREF_GTK_DEFAULT_GENERAL_CONNECT_STARTUP);
	SET_CHECKBOX(priv->check_select_channel_joined, LOQUI_GENERAL_PREF_GTK_GROUP_GENERAL, "SelectChannelJoined", LOQUI_GENERAL_PREF_GTK_DEFAULT_GENERAL_SELECT_CHANNEL_JOINED);
	SET_CHECKBOX(priv->check_auto_command_mode, LOQUI_GENERAL_PREF_GTK_GROUP_GENERAL, "AutoCommandMode", LOQUI_GENERAL_PREF_GTK_DEFAULT_GENERAL_AUTO_COMMAND_MODE);
	SET_CHECKBOX(priv->check_save_log, LOQUI_GENERAL_PREF_GTK_GROUP_GENERAL, "SaveLog", LOQUI_GENERAL_PREF_GTK_DEFAULT_GENERAL_SAVE_LOG);

#define LOAD_FROM_ARRAY_TO_TEXT_VIEW(_group, _key, _textview) { \
	list = loqui_utils_string_array_to_list(loqui_pref_get_string_list(general_pref, _group, _key, NULL, NULL), TRUE);  \
	gtkutils_set_textview_from_string_list(GTK_TEXT_VIEW(_textview), list); \
	loqui_utils_free_list_and_elements(list); \
}

	LOAD_FROM_ARRAY_TO_TEXT_VIEW(LOQUI_GENERAL_PREF_GROUP_NOTIFICATION, "HighlightList", priv->textview_highlight);
	LOAD_FROM_ARRAY_TO_TEXT_VIEW(LOQUI_GENERAL_PREF_GROUP_IGNORE, "NormalIgnoreList", priv->textview_normal_ignore);
	LOAD_FROM_ARRAY_TO_TEXT_VIEW(LOQUI_GENERAL_PREF_GROUP_IGNORE, "TransparentIgnoreList", priv->textview_transparent_ignore);

#undef LOAD_FROM_ARRAY_TO_TEXT_VIEW


	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(priv->textview_title_format_title));
	buf = loqui_pref_get_with_default_string(loqui_get_general_pref(),
						 LOQUI_GENERAL_PREF_GTK_GROUP_TITLE_FORMAT, "TitleFormatTitle",
						 LOQUI_GENERAL_PREF_GTK_DEFAULT_TITLE_FORMAT_TITLE_FORMAT_TITLE, NULL);
	gtk_text_buffer_set_text(GTK_TEXT_BUFFER(buffer), LOQUI_UTILS_EMPTY_IF_NULL(buf), -1);
	g_free(buf);

	buf = loqui_pref_get_with_default_string(loqui_get_general_pref(),
						 LOQUI_GENERAL_PREF_GTK_GROUP_TITLE_FORMAT, "TitleFormatStatusbar",
						 LOQUI_GENERAL_PREF_GTK_DEFAULT_TITLE_FORMAT_TITLE_FORMAT_STATUSBAR, NULL);
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(priv->textview_title_format_statusbar));
	gtk_text_buffer_set_text(GTK_TEXT_BUFFER(buffer), LOQUI_UTILS_EMPTY_IF_NULL(buf), -1);
	g_free(buf);

	buf = loqui_pref_get_with_default_string(loqui_get_general_pref(),
						 LOQUI_GENERAL_PREF_GROUP_MESSAGES, "AwayMessage",
						 LOQUI_GENERAL_PREF_DEFAULT_MESSAGES_AWAY_MESSAGE, NULL);
	gtk_entry_set_text(GTK_ENTRY(priv->entry_away_message), LOQUI_UTILS_EMPTY_IF_NULL(buf));
	g_free(buf);

	buf = loqui_pref_get_with_default_string(loqui_get_general_pref(),
						 LOQUI_GENERAL_PREF_GTK_GROUP_COMMANDS, "BrowserCommand",
						 LOQUI_GENERAL_PREF_GTK_DEFAULT_COMMANDS_BROWSER_COMMAND, NULL);
	gtk_entry_set_text(GTK_ENTRY(priv->entry_browser_command), LOQUI_UTILS_EMPTY_IF_NULL(buf));
	g_free(buf);

	buf = loqui_pref_get_with_default_string(loqui_get_general_pref(),
						 LOQUI_GENERAL_PREF_GTK_GROUP_COMMANDS, "NotificationCommand",
						 LOQUI_GENERAL_PREF_GTK_DEFAULT_COMMANDS_NOTIFICATION_COMMAND, NULL);
	gtk_entry_set_text(GTK_ENTRY(priv->entry_notification_command), LOQUI_UTILS_EMPTY_IF_NULL(buf));
	g_free(buf);

	buf = loqui_pref_get_with_default_string(loqui_get_general_pref(),
						 LOQUI_GENERAL_PREF_GTK_GROUP_GENERAL, "TimeFormat",
						 LOQUI_GENERAL_PREF_GTK_DEFAULT_GENERAL_TIME_FORMAT, NULL);
	gtk_entry_set_text(GTK_ENTRY(priv->entry_time_format), LOQUI_UTILS_EMPTY_IF_NULL(buf));
	g_free(buf);

	buf = loqui_pref_get_with_default_string(loqui_get_general_pref(),
						 LOQUI_GENERAL_PREF_GTK_GROUP_GENERAL, "CommandPrefix",
						 LOQUI_GENERAL_PREF_GTK_DEFAULT_GENERAL_COMMAND_PREFIX, NULL);
	gtk_entry_set_text(GTK_ENTRY(priv->entry_command_prefix), LOQUI_UTILS_EMPTY_IF_NULL(buf));
	g_free(buf);

	buf = loqui_pref_get_with_default_string(loqui_get_general_pref(),
						 LOQUI_GENERAL_PREF_GROUP_PROXY, "RecentLogRegexp",
						 LOQUI_GENERAL_PREF_DEFAULT_PROXY_RECENT_LOG_REGEXP, NULL);
	gtk_entry_set_text(GTK_ENTRY(priv->entry_recent_log_regexp), LOQUI_UTILS_EMPTY_IF_NULL(buf));
	g_free(buf);

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(priv->spin_common_buffer_max_line_number),
				  loqui_pref_get_with_default_integer(loqui_get_general_pref(),
								      LOQUI_GENERAL_PREF_GTK_GROUP_GENERAL, "CommonBufferMaxLineNumber",
								      LOQUI_GENERAL_PREF_GTK_DEFAULT_GENERAL_COMMON_BUFFER_MAX_LINE_NUMBER, NULL));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(priv->spin_channel_buffer_max_line_number),
				  loqui_pref_get_with_default_integer(loqui_get_general_pref(),
								      LOQUI_GENERAL_PREF_GTK_GROUP_GENERAL, "ChannelBufferMaxLineNumber",
								      LOQUI_GENERAL_PREF_GTK_DEFAULT_GENERAL_CHANNEL_BUFFER_MAX_LINE_NUMBER, NULL));
}
static void
prefs_dialog_save_settings(PrefsDialog *dialog)
{
	PrefsDialogPrivate *priv;
	gchar *buf;
	LoquiTitleFormat *ltf;
	GList *list;
	gchar **strarray;
	gsize len;

	g_return_if_fail(dialog != NULL);
        g_return_if_fail(IS_PREFS_DIALOG(dialog));

	priv = dialog->priv;

	loqui_pref_set_boolean(loqui_get_general_pref(),
                               LOQUI_GENERAL_PREF_GTK_GROUP_SIZE, "SaveSize",
                               gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->check_save_size)));
	loqui_pref_set_boolean(loqui_get_general_pref(),
                               LOQUI_GENERAL_PREF_GROUP_PROXY, "ParsePlumRecent",
                               gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->check_parse_plum_recent)));
	loqui_pref_set_boolean(loqui_get_general_pref(),
                               LOQUI_GENERAL_PREF_GROUP_IGNORE, "UseNormalIgnore",
                               gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->check_use_normal_ignore)));
	loqui_pref_set_boolean(loqui_get_general_pref(),
                               LOQUI_GENERAL_PREF_GROUP_IGNORE, "UseTransparentIgnore",
                               gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->check_use_transparent_ignore)));
	loqui_pref_set_boolean(loqui_get_general_pref(),
                               LOQUI_GENERAL_PREF_GROUP_NOTIFICATION, "UseNotification",
                               gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->check_use_notification)));
	loqui_pref_set_boolean(loqui_get_general_pref(),
                               LOQUI_GENERAL_PREF_GROUP_NOTIFICATION, "ExecNotificationByNotice",
                               gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->check_exec_notification_by_notice)));
	loqui_pref_set_boolean(loqui_get_general_pref(),
                               LOQUI_GENERAL_PREF_GROUP_PROXY, "TreatAsRecentLogUntilFirstPongReceived",
                               gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->check_treat_as_recent_log_until_first_pong_received)));
	loqui_pref_set_boolean(loqui_get_general_pref(),
                               LOQUI_GENERAL_PREF_GROUP_ACCOUNT, "AutoReconnect",
                               gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->check_auto_reconnect)));
	loqui_pref_set_boolean(loqui_get_general_pref(),
                               LOQUI_GENERAL_PREF_GTK_GROUP_GENERAL, "ConnectStartup",
                               gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->check_connect_startup)));
	loqui_pref_set_boolean(loqui_get_general_pref(),
                               LOQUI_GENERAL_PREF_GTK_GROUP_GENERAL, "SelectChannelJoined",
                               gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->check_select_channel_joined)));
	loqui_pref_set_boolean(loqui_get_general_pref(),
                               LOQUI_GENERAL_PREF_GTK_GROUP_GENERAL, "AutoCommandMode",
                               gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->check_auto_command_mode)));
	loqui_pref_set_boolean(loqui_get_general_pref(),
                               LOQUI_GENERAL_PREF_GTK_GROUP_GENERAL, "SaveLog",
                               gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->check_save_log)));

	loqui_app_set_auto_switch_scrolling_channel_buffers(priv->app, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->check_auto_switch_scrolling)));
	loqui_app_set_auto_switch_scrolling_common_buffer(priv->app, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->check_auto_switch_scrolling_common_buffer)));

#define SAVE_FROM_TEXT_VIEW_TO_ARRAY(_group, _key, _textview) { \
	list = NULL; \
	gtkutils_set_string_list_from_textview(&list, GTK_TEXT_VIEW(_textview)); \
	len = g_list_length(list); \
	strarray = loqui_utils_list_to_string_array(list, TRUE); \
	loqui_pref_set_string_list(loqui_get_general_pref(), _group, _key, strarray, len); \
	g_strfreev(strarray); \
}
	SAVE_FROM_TEXT_VIEW_TO_ARRAY(LOQUI_GENERAL_PREF_GROUP_NOTIFICATION, "HighlightList", priv->textview_highlight);
	SAVE_FROM_TEXT_VIEW_TO_ARRAY(LOQUI_GENERAL_PREF_GROUP_IGNORE, "NormalIgnoreList", priv->textview_normal_ignore);
	SAVE_FROM_TEXT_VIEW_TO_ARRAY(LOQUI_GENERAL_PREF_GROUP_IGNORE, "TransparentIgnoreList", priv->textview_transparent_ignore);

	loqui_pref_set_string(loqui_get_general_pref(),
                              LOQUI_GENERAL_PREF_GROUP_MESSAGES, "AwayMessage",
                              gtk_entry_get_text(GTK_ENTRY(priv->entry_away_message)));

	loqui_pref_set_string(loqui_get_general_pref(),
                              LOQUI_GENERAL_PREF_GTK_GROUP_COMMANDS, "BrowserCommand",
                              gtk_entry_get_text(GTK_ENTRY(priv->entry_browser_command)));

	loqui_pref_set_string(loqui_get_general_pref(),
                              LOQUI_GENERAL_PREF_GTK_GROUP_COMMANDS, "NotificationCommand",
                              gtk_entry_get_text(GTK_ENTRY(priv->entry_notification_command)));

	loqui_pref_set_string(loqui_get_general_pref(),
                              LOQUI_GENERAL_PREF_GTK_GROUP_GENERAL, "TimeFormat",
                              gtk_entry_get_text(GTK_ENTRY(priv->entry_time_format)));

	loqui_pref_set_string(loqui_get_general_pref(),
                              LOQUI_GENERAL_PREF_GTK_GROUP_GENERAL, "CommandPrefix",
                              gtk_entry_get_text(GTK_ENTRY(priv->entry_command_prefix)));

	loqui_pref_set_string(loqui_get_general_pref(),
			      LOQUI_GENERAL_PREF_GROUP_PROXY, "RecentLogRegexp",
                              gtk_entry_get_text(GTK_ENTRY(priv->entry_recent_log_regexp)));

	loqui_pref_set_integer(loqui_get_general_pref(),
                               LOQUI_GENERAL_PREF_GTK_GROUP_GENERAL, "CommonBufferMaxLineNumber",
                               gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(priv->spin_common_buffer_max_line_number)));
	loqui_pref_set_integer(loqui_get_general_pref(),
                               LOQUI_GENERAL_PREF_GTK_GROUP_GENERAL, "ChannelBufferMaxLineNumber",
                               gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(priv->spin_channel_buffer_max_line_number)));

/* FIXME: use _default */
#define SET_TITLE_FORMAT(_textview, _group, _key, _default, _setter) { \
	buf = gtkutils_get_text_from_textview(GTK_TEXT_VIEW(_textview)); \
	if (strlen(buf) > 0) { \
		ltf = loqui_title_format_new(); \
		if (!loqui_title_format_parse(ltf, buf, NULL)) { \
			gtkutils_msgbox_info(GTK_MESSAGE_ERROR, _("Invalid title format: default is used for %s."), _key); \
			loqui_title_format_free(ltf); \
			_setter(priv->app->appinfo, NULL); \
		} else { \
			_setter(priv->app->appinfo, ltf); \
		} \
	} else { \
		_setter(priv->app->appinfo, NULL); \
	} \
        loqui_pref_set_string(loqui_get_general_pref(), _group, _key, buf); \
}

	SET_TITLE_FORMAT(priv->textview_title_format_title,
			 LOQUI_GENERAL_PREF_GTK_GROUP_TITLE_FORMAT, "TitleFormatTitle",
			 LOQUI_GENERAL_PREF_GTK_DEFAULT_TITLE_FORMAT_TITLE_FORMAT_TITLE, loqui_app_info_set_title_format_title);
	SET_TITLE_FORMAT(priv->textview_title_format_statusbar,
			 LOQUI_GENERAL_PREF_GTK_GROUP_TITLE_FORMAT, "TitleFormatStatusbar",
			 LOQUI_GENERAL_PREF_GTK_DEFAULT_TITLE_FORMAT_TITLE_FORMAT_STATUSBAR, loqui_app_info_set_title_format_statusbar);

	loqui_core_gtk_save_general_pref(LOQUI_CORE_GTK(loqui_get_core()));
}

static void
prefs_dialog_response_cb(GtkWidget *widget, gint response, gpointer data)
{
	g_return_if_fail(IS_PREFS_DIALOG(data));

	if(response == GTK_RESPONSE_OK) {
		prefs_dialog_save_settings(PREFS_DIALOG(data));
	}
}

GtkWidget*
prefs_dialog_new(LoquiApp *app)
{
        PrefsDialog *dialog;
	PrefsDialogPrivate *priv;
	GtkWidget *notebook;
	GtkWidget *vbox;
	GtkWidget *vbox2;
	GtkWidget *frame;
	GtkWidget *label;
	GtkWidget *scrolled_win;

	dialog = g_object_new(prefs_dialog_get_type(), NULL);

	gtk_window_set_title(GTK_WINDOW(dialog), _("Common Preferences"));
	gtk_dialog_add_buttons(GTK_DIALOG(dialog),
                               GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                               GTK_STOCK_OK, GTK_RESPONSE_OK,
                               NULL);

	g_signal_connect(G_OBJECT(dialog),
			 "response",
			 G_CALLBACK(prefs_dialog_response_cb),
			 dialog);

	priv = dialog->priv;
	priv->app = app;

	notebook = gtk_notebook_new();
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), notebook, TRUE, TRUE, 5);

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox, gtk_label_new(_("General")));

	priv->check_save_size = gtk_check_button_new_with_label(_("Save window/widget sizes"));
	gtk_box_pack_start(GTK_BOX(vbox), priv->check_save_size, FALSE, FALSE, 0);
	priv->check_auto_switch_scrolling = gtk_check_button_new_with_label(_("Switch whether scrolling channel buffer or not automatically"));
	gtk_box_pack_start(GTK_BOX(vbox), priv->check_auto_switch_scrolling, FALSE, FALSE, 0);
	priv->check_auto_switch_scrolling_common_buffer = gtk_check_button_new_with_label(_("Switch whether scrolling common buffer or not automatically"));
	gtk_box_pack_start(GTK_BOX(vbox), priv->check_auto_switch_scrolling_common_buffer, FALSE, FALSE, 0);

	priv->check_auto_reconnect = gtk_check_button_new_with_label(_("Reconnect automatically when connections are terminated."));
	gtk_box_pack_start(GTK_BOX(vbox), priv->check_auto_reconnect, FALSE, FALSE, 0);

	priv->check_connect_startup = gtk_check_button_new_with_label(_("Connect default accounts when the program started"));
	gtk_box_pack_start(GTK_BOX(vbox), priv->check_connect_startup, FALSE, FALSE, 0);
	priv->check_select_channel_joined = gtk_check_button_new_with_label(_("Select a new channel automatically."));
	gtk_box_pack_start(GTK_BOX(vbox), priv->check_select_channel_joined, FALSE, FALSE, 0);

	gtkutils_add_label_entry(vbox, _("Away message: "), &priv->entry_away_message, "");
	gtkutils_add_label_entry(vbox, _("Format of time in buffers: "), &priv->entry_time_format, "");

	gtkutils_add_label_spin_button(vbox,
				       _("Max line number of a common buffer(0: unlimited): "),
				       &priv->spin_common_buffer_max_line_number,
				       0, G_MAXDOUBLE, 50);
	gtkutils_add_label_spin_button(vbox,
				       _("Max line number of a channel buffer(0: unlimited): "),
				       &priv->spin_channel_buffer_max_line_number,
				       0, G_MAXDOUBLE, 50);

	priv->check_auto_command_mode = gtk_check_button_new_with_label(_("Toggle command mode automatically"));
	gtk_box_pack_start(GTK_BOX(vbox), priv->check_auto_command_mode, FALSE, FALSE, 0);
	
	gtkutils_add_label_entry(vbox, _("Prefix for commands: "), &priv->entry_command_prefix, "");

	priv->check_save_log = gtk_check_button_new_with_label(_("Save logs (Experimental)"));
	gtk_box_pack_start(GTK_BOX(vbox), priv->check_save_log, FALSE, FALSE, 0);

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox, gtk_label_new(_("Proxy")));

	priv->check_parse_plum_recent = gtk_check_button_new_with_label(_("Parse plum (an irc proxy) recent feature (Experimental)"));
	gtk_box_pack_start(GTK_BOX(vbox), priv->check_parse_plum_recent, FALSE, FALSE, 0);

	gtkutils_add_label_entry(vbox, _("Regular Expression for recent logs: "), &priv->entry_recent_log_regexp, "");
	
	priv->check_treat_as_recent_log_until_first_pong_received = gtk_check_button_new_with_label(_("Treat server messages as recent logs until received a first PONG (for some irc-proxies)."));
	gtk_box_pack_start(GTK_BOX(vbox), priv->check_treat_as_recent_log_until_first_pong_received, FALSE, FALSE, 0);
	
	vbox = gtk_vbox_new(FALSE, 0);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox, gtk_label_new(_("Highlight")));

	priv->check_use_notification = gtk_check_button_new_with_label(_("Use notification"));
	gtk_box_pack_start(GTK_BOX(vbox), priv->check_use_notification, FALSE, FALSE, 0);

	priv->check_exec_notification_by_notice = gtk_check_button_new_with_label(_("Execute notification by NOTICE"));
	gtk_box_pack_start(GTK_BOX(vbox), priv->check_exec_notification_by_notice, FALSE, FALSE, 0);

	frame = gtkutils_create_framed_textview(&priv->textview_highlight,
						_("Highlighting keywords(Separate each words with linefeeds)"));
	gtk_box_pack_start(GTK_BOX(vbox), frame, TRUE, TRUE, 0);

	label = gtk_label_new(_("Special prefixes:\n"
				" \"re:\":     Perl Compatible Regular Expression (ex. re:foo*)\n"
				" \"plain:\":  Plain (normal) text (ex. plain:word)"));
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox, gtk_label_new(_("Ignore")));

	priv->check_use_normal_ignore = gtk_check_button_new_with_label(_("Use ignore (normal) feature"));
	gtk_box_pack_start(GTK_BOX(vbox), priv->check_use_normal_ignore, FALSE, FALSE, 0);

	frame = gtkutils_create_framed_textview(&priv->textview_normal_ignore,
						_("Nickname list to ignore (normal) ('*' and '?' can be used)"));
	gtk_box_pack_start(GTK_BOX(vbox), frame, TRUE, TRUE, 0);

	priv->check_use_transparent_ignore = gtk_check_button_new_with_label(_("Use ignore (transparent) feature"));
	gtk_box_pack_start(GTK_BOX(vbox), priv->check_use_transparent_ignore, FALSE, FALSE, 0);

	frame = gtkutils_create_framed_textview(&priv->textview_transparent_ignore,
						_("Nickname list to ignore (transparent) ('*' and '?' can be used)"));
	gtk_box_pack_start(GTK_BOX(vbox), frame, TRUE, TRUE, 0);

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox, gtk_label_new(_("Command")));
	
	gtkutils_add_label_entry(vbox, _("Browser: "), &priv->entry_browser_command, "");

	label = gtk_label_new(_("An URL will be quoted and substituted with %s.\n"
				"Sample:\n"
				" URL: http://example.com/#id\n"
				" Browser: mozilla -remote openURL\\(%s,new-tab\\)\n"
				"  -> mozilla -remote openURL\\('http://example.com/#id',new-tab\\)"));
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
	
	gtkutils_add_label_entry(vbox, _("Notification: "), &priv->entry_notification_command, "");

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox, gtk_label_new(_("Format")));

	label = gtk_label_new(_("See title formatting help of the software 'foobar2000'.\nThe list of variables is at loqui_app_info.c.\nIf you version is input, it is used."));
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 5);

	frame = gtk_frame_new(_("Title format of title"));
	gtk_box_pack_start(GTK_BOX(vbox), frame, TRUE, TRUE, 5);

	vbox2 = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(frame), vbox2);
	
	scrolled_win = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_win),
				       GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start(GTK_BOX(vbox2), scrolled_win, TRUE, TRUE, 0);

	priv->textview_title_format_title = gtk_text_view_new();
	gtk_container_add(GTK_CONTAINER(scrolled_win), priv->textview_title_format_title);

	label = gtk_label_new(_("Default:"));
	gtk_box_pack_start(GTK_BOX(vbox2), label, FALSE, FALSE, 0);

	label = gtk_label_new(LOQUI_APP_INFO_DEFAULT_TITLE_FORMAT_TITLE);
	gtk_box_pack_start(GTK_BOX(vbox2), label, FALSE, FALSE, 0);
	gtk_label_set_selectable(GTK_LABEL(label), TRUE);

	frame = gtk_frame_new(_("Title format of statusbar"));
	gtk_box_pack_start(GTK_BOX(vbox), frame, TRUE, TRUE, 5);

	vbox2 = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(frame), vbox2);
	
	scrolled_win = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_win),
				       GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start(GTK_BOX(vbox2), scrolled_win, TRUE, TRUE, 0);

	priv->textview_title_format_statusbar = gtk_text_view_new();
	gtk_container_add(GTK_CONTAINER(scrolled_win), priv->textview_title_format_statusbar);

	label = gtk_label_new(_("Default:"));
	gtk_box_pack_start(GTK_BOX(vbox2), label, FALSE, FALSE, 0);

	label = gtk_label_new(LOQUI_APP_INFO_DEFAULT_TITLE_FORMAT_STATUSBAR);
	gtk_box_pack_start(GTK_BOX(vbox2), label, FALSE, FALSE, 0);
	gtk_label_set_selectable(GTK_LABEL(label), TRUE);
	gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);

	prefs_dialog_load_settings(dialog);

	gtk_widget_show_all(GTK_WIDGET(GTK_DIALOG(dialog)->vbox));

	return GTK_WIDGET(dialog);
}

void 
prefs_dialog_open(LoquiApp *app)
{
        GtkWidget *dialog;

        dialog = prefs_dialog_new(app);
        gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(app));
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
}

