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

#include "prefs_dialog.h"
#include "intl.h"
#include "prefs_general.h"
#include "utils.h"
#include "gtkutils.h"
#include "codeconv.h"

#include <string.h>

struct _PrefsDialogPrivate
{
	GtkWidget *check_save_size;
	GtkWidget *check_auto_switch_scrolling;
	GtkWidget *check_parse_plum_recent;
	GtkWidget *check_auto_reconnect;
	GtkWidget *check_connect_startup;
	GtkWidget *entry_away_message;
	GtkWidget *entry_time_format;
	
	GtkWidget *spin_common_buffer_max_line_number;
	GtkWidget *spin_channel_buffer_max_line_number;

	GtkWidget *check_auto_command_mode;
	GtkWidget *entry_command_prefix;
		
	GtkWidget *check_use_notification;
	GtkWidget *textview_highlight;

	GtkWidget *check_use_transparent_ignore;
	GtkWidget *textview_transparent_ignore;

	GtkWidget *entry_browser_command;
	GtkWidget *entry_notification_command;
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

	g_return_if_fail(dialog != NULL);
        g_return_if_fail(IS_PREFS_DIALOG(dialog));

	priv = dialog->priv;

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->check_auto_switch_scrolling), prefs_general.auto_switch_scrolling);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->check_parse_plum_recent), prefs_general.parse_plum_recent);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->check_save_size), prefs_general.save_size);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->check_use_notification), prefs_general.use_notification);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->check_use_transparent_ignore), prefs_general.use_transparent_ignore);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->check_auto_reconnect), prefs_general.auto_reconnect);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->check_connect_startup), prefs_general.connect_startup);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->check_auto_command_mode), prefs_general.auto_command_mode);

	gtkutils_set_textview_from_string_list(GTK_TEXT_VIEW(priv->textview_highlight), prefs_general.highlight_list);
	gtkutils_set_textview_from_string_list(GTK_TEXT_VIEW(priv->textview_transparent_ignore),
					       prefs_general.transparent_ignore_list);

	gtk_entry_set_text(GTK_ENTRY(priv->entry_away_message), prefs_general.away_message);
	gtk_entry_set_text(GTK_ENTRY(priv->entry_browser_command), prefs_general.browser_command);
	gtk_entry_set_text(GTK_ENTRY(priv->entry_notification_command), prefs_general.notification_command);
	gtk_entry_set_text(GTK_ENTRY(priv->entry_time_format), prefs_general.time_format);
	gtk_entry_set_text(GTK_ENTRY(priv->entry_command_prefix), prefs_general.command_prefix);

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(priv->spin_common_buffer_max_line_number),
				  prefs_general.common_buffer_max_line_number);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(priv->spin_channel_buffer_max_line_number),
				  prefs_general.channel_buffer_max_line_number);
}
static void
prefs_dialog_save_settings(PrefsDialog *dialog)
{
	PrefsDialogPrivate *priv;

	g_return_if_fail(dialog != NULL);
        g_return_if_fail(IS_PREFS_DIALOG(dialog));

	priv = dialog->priv;

	prefs_general.save_size = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->check_save_size));
	prefs_general.auto_switch_scrolling = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->check_auto_switch_scrolling));
	prefs_general.parse_plum_recent = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->check_parse_plum_recent));
	prefs_general.use_transparent_ignore = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->check_use_transparent_ignore));
	prefs_general.use_notification = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->check_use_notification));
	prefs_general.auto_reconnect = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->check_auto_reconnect));
	prefs_general.connect_startup = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->check_connect_startup));
	prefs_general.auto_command_mode = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->check_auto_command_mode));

	gtkutils_set_string_list_from_textview(&prefs_general.highlight_list, GTK_TEXT_VIEW(priv->textview_highlight));
	gtkutils_set_string_list_from_textview(&prefs_general.transparent_ignore_list,
					       GTK_TEXT_VIEW(priv->textview_transparent_ignore));

	G_FREE_UNLESS_NULL(prefs_general.away_message);
	prefs_general.away_message = g_strdup(gtk_entry_get_text(GTK_ENTRY(priv->entry_away_message)));

	G_FREE_UNLESS_NULL(prefs_general.browser_command);
	prefs_general.browser_command = g_strdup(gtk_entry_get_text(GTK_ENTRY(priv->entry_browser_command)));

	G_FREE_UNLESS_NULL(prefs_general.notification_command);
	prefs_general.notification_command = g_strdup(gtk_entry_get_text(GTK_ENTRY(priv->entry_notification_command)));

	G_FREE_UNLESS_NULL(prefs_general.time_format);
	prefs_general.time_format = g_strdup(gtk_entry_get_text(GTK_ENTRY(priv->entry_time_format)));

	G_FREE_UNLESS_NULL(prefs_general.command_prefix);
	prefs_general.command_prefix = g_strdup(gtk_entry_get_text(GTK_ENTRY(priv->entry_command_prefix)));

	prefs_general.common_buffer_max_line_number = (guint) gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(priv->spin_common_buffer_max_line_number));
	prefs_general.channel_buffer_max_line_number = (guint) gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(priv->spin_channel_buffer_max_line_number));
	
	prefs_general_save();
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
prefs_dialog_new(void)
{
        PrefsDialog *dialog;
	PrefsDialogPrivate *priv;
	GtkWidget *notebook;
	GtkWidget *vbox;
	GtkWidget *frame;
	GtkWidget *label;
	
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

	notebook = gtk_notebook_new();
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), notebook, TRUE, TRUE, 5);

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox, gtk_label_new(_("General")));

	priv->check_save_size = gtk_check_button_new_with_label(_("Save window/widget sizes"));
	gtk_box_pack_start(GTK_BOX(vbox), priv->check_save_size, FALSE, FALSE, 0);
	priv->check_auto_switch_scrolling = gtk_check_button_new_with_label(_("Switch whether scrolling or not automatically"));
	gtk_box_pack_start(GTK_BOX(vbox), priv->check_auto_switch_scrolling, FALSE, FALSE, 0);
	priv->check_parse_plum_recent = gtk_check_button_new_with_label(_("Parse plum (an irc proxy) recent feature (Experimental)"));
	gtk_box_pack_start(GTK_BOX(vbox), priv->check_parse_plum_recent, FALSE, FALSE, 0);
	priv->check_auto_reconnect = gtk_check_button_new_with_label(_("Reconnect automatically when connections are terminated."));
	gtk_box_pack_start(GTK_BOX(vbox), priv->check_auto_reconnect, FALSE, FALSE, 0);

	priv->check_connect_startup = gtk_check_button_new_with_label(_("Connect default accounts when the program started"));
	gtk_box_pack_start(GTK_BOX(vbox), priv->check_connect_startup, FALSE, FALSE, 0);

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
	
	vbox = gtk_vbox_new(FALSE, 0);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox, gtk_label_new(_("Highlight")));

	priv->check_use_notification = gtk_check_button_new_with_label(_("Use notification"));
	gtk_box_pack_start(GTK_BOX(vbox), priv->check_use_notification, FALSE, FALSE, 0);

	frame = gtkutils_create_framed_textview(&priv->textview_highlight,
						_("Highlighting keywords(Separate each words with linefeeds)"));
	gtk_box_pack_start(GTK_BOX(vbox), frame, TRUE, TRUE, 0);

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox, gtk_label_new(_("Ignore")));

	priv->check_use_transparent_ignore = gtk_check_button_new_with_label(_("Use ignore (transparent) feature"));
	gtk_box_pack_start(GTK_BOX(vbox), priv->check_use_transparent_ignore, FALSE, FALSE, 0);

	frame = gtkutils_create_framed_textview(&priv->textview_transparent_ignore,
						_("Nickname list to ignore (transparent) ('*' and '?' can be used)"));
	gtk_box_pack_start(GTK_BOX(vbox), frame, TRUE, TRUE, 0);

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox, gtk_label_new(_("Command")));
	
	gtkutils_add_label_entry(vbox, _("Browser: "), &priv->entry_browser_command, "");

	label = gtk_label_new(_("URL is passed to a browser with quoted, like 'http://example.com/'"));
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
	
	gtkutils_add_label_entry(vbox, _("Notification: "), &priv->entry_notification_command, "");

	prefs_dialog_load_settings(dialog);

	gtk_widget_show_all(GTK_WIDGET(GTK_DIALOG(dialog)->vbox));

	return GTK_WIDGET(dialog);
}

void 
prefs_dialog_open(GtkWindow *parent)
{
        GtkWidget *dialog;

        dialog = prefs_dialog_new();
        gtk_window_set_transient_for(GTK_WINDOW(dialog), parent);
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
}

