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
#include "prefs_emphasis_words.h"
#include "utils.h"
#include "gtkutils.h"
#include "codeconv.h"

struct _PrefsDialogPrivate
{
	GtkWidget *check_save_size;
	GtkWidget *entry_away_message;
	GtkWidget *option_codeconv;
	GtkWidget *entry_codeset;

	GtkWidget *check_use_notification;
	GtkWidget *textview_emphasis_allow;

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
	gchar *buf;
	GtkTextBuffer *buffer;
	GtkWidget *menu;
	GtkWidget *menuitem;
	gint i;

	g_return_if_fail(dialog != NULL);
        g_return_if_fail(IS_PREFS_DIALOG(dialog));

	priv = dialog->priv;

	menu = gtk_option_menu_get_menu(GTK_OPTION_MENU(priv->option_codeconv));

	for(i = 0; conv_table[i].title != NULL; i++) {
		menuitem = gtk_menu_item_new_with_label(gettext(conv_table[i].title));
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	}
	gtk_option_menu_set_history(GTK_OPTION_MENU(priv->option_codeconv), prefs_general.codeconv);

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->check_save_size), prefs_general.save_size);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->check_use_notification), prefs_general.use_notification);

	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(priv->textview_emphasis_allow));
	buf = utils_line_separated_text_from_slist(prefs_emphasis_words.allow_list);
	gtk_text_buffer_set_text(GTK_TEXT_BUFFER(buffer), buf, -1);
	g_free(buf);

	gtk_entry_set_text(GTK_ENTRY(priv->entry_away_message), prefs_general.away_message);
	gtk_entry_set_text(GTK_ENTRY(priv->entry_browser_command), prefs_general.browser_command);
	gtk_entry_set_text(GTK_ENTRY(priv->entry_notification_command), prefs_general.notification_command);
}
static void
prefs_dialog_save_settings(PrefsDialog *dialog)
{
	PrefsDialogPrivate *priv;
	gchar *buf;
	GtkTextBuffer *buffer;
	GtkTextIter start, end;

	g_return_if_fail(dialog != NULL);
        g_return_if_fail(IS_PREFS_DIALOG(dialog));

	priv = dialog->priv;

	prefs_general.save_size = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->check_save_size));
	prefs_general.use_notification = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->check_use_notification));
	prefs_general.codeconv = gtk_option_menu_get_history(GTK_OPTION_MENU(priv->option_codeconv));

	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(priv->textview_emphasis_allow));
	gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(buffer), &start);
	gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(buffer), &end);
	buf = gtk_text_buffer_get_text(GTK_TEXT_BUFFER(buffer), &start, &end, FALSE);
	prefs_emphasis_words_init();
	prefs_emphasis_words.allow_list = utils_line_separated_text_to_slist(buf);
	g_free(buf);
	
	G_FREE_UNLESS_NULL(prefs_general.codeset);
	prefs_general.codeset = g_strdup(gtk_entry_get_text(GTK_ENTRY(priv->entry_codeset)));

	G_FREE_UNLESS_NULL(prefs_general.away_message);
	prefs_general.away_message = g_strdup(gtk_entry_get_text(GTK_ENTRY(priv->entry_away_message)));

	G_FREE_UNLESS_NULL(prefs_general.browser_command);
	prefs_general.browser_command = g_strdup(gtk_entry_get_text(GTK_ENTRY(priv->entry_browser_command)));

	G_FREE_UNLESS_NULL(prefs_general.notification_command);
	prefs_general.notification_command = g_strdup(gtk_entry_get_text(GTK_ENTRY(priv->entry_notification_command)));

	codeconv_init();
}

static void
prefs_dialog_response_cb(GtkWidget *widget, gint response, gpointer data)
{
	g_return_if_fail(IS_PREFS_DIALOG(data));

	if(response == GTK_RESPONSE_OK) {
		prefs_dialog_save_settings(PREFS_DIALOG(data));
	}
}
static void
prefs_dialog_option_codeconv_changed_cb(GtkWidget *widget, gpointer data)
{
	PrefsDialog *dialog;
	PrefsDialogPrivate *priv;
	gint i;

	dialog = PREFS_DIALOG(data);
	priv = dialog->priv;

	i = gtk_option_menu_get_history(GTK_OPTION_MENU(widget));
	if(i == CODECONV_CUSTOM) {
		gtk_widget_set_sensitive(priv->entry_codeset, TRUE);
		gtk_entry_set_text(GTK_ENTRY(priv->entry_codeset), prefs_general.codeset);
	} else {
		gtk_widget_set_sensitive(priv->entry_codeset, FALSE);
		if(conv_table[i].codeset)
			gtk_entry_set_text(GTK_ENTRY(priv->entry_codeset), conv_table[i].codeset);
		else
			gtk_entry_set_text(GTK_ENTRY(priv->entry_codeset), "");
	}

}
GtkWidget*
prefs_dialog_new(void)
{
        PrefsDialog *dialog;
	PrefsDialogPrivate *priv;
	GtkWidget *notebook;
	GtkWidget *vbox;
	GtkWidget *label;
	GtkWidget *hbox;
	GtkWidget *frame;
	GtkWidget *scrolled_win;
	GtkWidget *menu;

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

	frame = gtk_frame_new(_("Code convertion"));
	gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 2);
	
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(frame), hbox);

	priv->option_codeconv = gtk_option_menu_new();
	gtk_box_pack_start(GTK_BOX(hbox), priv->option_codeconv, FALSE, FALSE, 0);
	menu = gtk_menu_new();
	gtk_option_menu_set_menu(GTK_OPTION_MENU(priv->option_codeconv), menu);
	g_signal_connect(G_OBJECT(priv->option_codeconv), "changed",
			 G_CALLBACK(prefs_dialog_option_codeconv_changed_cb), dialog);
	
	priv->entry_codeset = gtk_entry_new();
	gtk_box_pack_end(GTK_BOX(hbox), priv->entry_codeset, FALSE, FALSE, 0);

	label = gtk_label_new("codeset: ");
	gtk_box_pack_end(GTK_BOX(hbox), label, FALSE, FALSE, 0);

	gtkutils_add_label_entry(vbox, _("Away message: "), &priv->entry_away_message, "");

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox, gtk_label_new(_("Emphasis")));

	priv->check_use_notification = gtk_check_button_new_with_label(_("Use notification"));
	gtk_box_pack_start(GTK_BOX(vbox), priv->check_use_notification, FALSE, FALSE, 0);

	frame = gtk_frame_new(_("Emphasizing keywords(Separate each words with linefeeds)"));
	gtk_box_pack_start(GTK_BOX(vbox), frame, TRUE, TRUE, 0);

	scrolled_win = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_win), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(frame), scrolled_win);

	priv->textview_emphasis_allow = gtk_text_view_new();
	gtk_container_add(GTK_CONTAINER(scrolled_win), priv->textview_emphasis_allow);
	vbox = gtk_vbox_new(FALSE, 0);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox, gtk_label_new(_("Command")));

	gtkutils_add_label_entry(vbox, _("Browser: "), &priv->entry_browser_command, "");
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

