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

#include "channel_input_dialog.h"
#include "account_manager.h"
#include "intl.h"
#include <string.h>

struct _ChannelInputDialogPrivate
{
	Account *account;
	ChannelHistoryType history_type;
	ChannelInputFunc func;
	gpointer data;
};

static GtkDialogClass *parent_class = NULL;
#define PARENT_TYPE GTK_TYPE_DIALOG

static void channel_input_dialog_class_init(ChannelInputDialogClass *klass);
static void channel_input_dialog_init(ChannelInputDialog *channel_input_dialog);
static void channel_input_dialog_finalize(GObject *object);
static void channel_input_dialog_destroy(GtkObject *object);

static void channel_input_dialog_response_cb(GtkWidget *widget, gint response, gpointer data);
static void channel_input_dialog_option_menu_changed_cb(GtkWidget *widget, gpointer data);
static void channel_input_dialog_entry_activated_cb(GtkWidget *widget, gpointer data);

static void channel_input_dialog_update_combo(ChannelInputDialog *dialog);

GType
channel_input_dialog_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(ChannelInputDialogClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) channel_input_dialog_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(ChannelInputDialog),
				0,              /* n_preallocs */
				(GInstanceInitFunc) channel_input_dialog_init
			};
		
		type = g_type_register_static(PARENT_TYPE,
					      "ChannelInputDialog",
					      &our_info,
					      0);
	}
	
	return type;
}
static void
channel_input_dialog_class_init (ChannelInputDialogClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);
        GtkObjectClass *gtk_object_class = GTK_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = channel_input_dialog_finalize;
        gtk_object_class->destroy = channel_input_dialog_destroy;
}
static void 
channel_input_dialog_init (ChannelInputDialog *channel_input_dialog)
{
	ChannelInputDialogPrivate *priv;

	priv = g_new0(ChannelInputDialogPrivate, 1);

	channel_input_dialog->priv = priv;
}
static void 
channel_input_dialog_finalize (GObject *object)
{
	ChannelInputDialog *channel_input_dialog;

        g_return_if_fail(object != NULL);
        g_return_if_fail(IS_CHANNEL_INPUT_DIALOG(object));

        channel_input_dialog = CHANNEL_INPUT_DIALOG(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize) (object);

	g_free(channel_input_dialog->priv);
}
static void 
channel_input_dialog_destroy (GtkObject *object)
{
        ChannelInputDialog *channel_input_dialog;

        g_return_if_fail(object != NULL);
        g_return_if_fail(IS_CHANNEL_INPUT_DIALOG(object));

        channel_input_dialog = CHANNEL_INPUT_DIALOG(object);

        if (GTK_OBJECT_CLASS(parent_class)->destroy)
                (* GTK_OBJECT_CLASS(parent_class)->destroy) (object);
}

static void
channel_input_dialog_response_cb(GtkWidget *widget, gint response, gpointer data)
{
	ChannelInputDialog *dialog;
	ChannelInputDialogPrivate *priv;

	dialog = CHANNEL_INPUT_DIALOG(data);
	priv = dialog->priv;

	if(response == GTK_RESPONSE_OK) {
		if(priv->func) {
			priv->func(channel_input_dialog_get_account(dialog),
				   gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(dialog->combo)->entry)),
				   gtk_entry_get_text(GTK_ENTRY(dialog->entry)),
				   priv->data);
		}
	}
	gtk_widget_destroy(widget);
}
static void
channel_input_dialog_option_menu_changed_cb(GtkWidget *widget, gpointer data)
{
	ChannelInputDialog *dialog;
	ChannelInputDialogPrivate *priv;
	GtkWidget *menu, *menuitem;

	dialog = CHANNEL_INPUT_DIALOG(data);
	priv = dialog->priv;
	
	menu = gtk_option_menu_get_menu(GTK_OPTION_MENU(dialog->option_menu));

	menuitem = gtk_menu_get_active(GTK_MENU(menu));
	if(menuitem == NULL)
		return;

	priv->account = g_object_get_data(G_OBJECT(menuitem), "account");
	channel_input_dialog_update_combo(dialog);
}
static void
channel_input_dialog_entry_activated_cb(GtkWidget *widget, gpointer data)
{
	gtk_dialog_response(GTK_DIALOG(data), GTK_RESPONSE_OK);
}

GtkWidget*
channel_input_dialog_new(void)
{
        ChannelInputDialog *dialog;
	ChannelInputDialogPrivate *priv;
	GSList *account_list, *cur;
	Account *account;
	GtkWidget *hbox;
	GtkWidget *vsep;
	GtkWidget *label;
	GtkWidget *menu, *menuitem;

	dialog = g_object_new(channel_input_dialog_get_type(), NULL);
	priv = dialog->priv;

	gtk_dialog_add_buttons(GTK_DIALOG(dialog),
			       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			       GTK_STOCK_OK, GTK_RESPONSE_OK,
			       NULL);
	gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);

	g_signal_connect(G_OBJECT(dialog),
			 "response",
			 G_CALLBACK(channel_input_dialog_response_cb),
			 dialog);

	dialog->label = gtk_label_new("");
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), dialog->label, TRUE, TRUE, 5);

	hbox = gtk_hbox_new(FALSE, 5);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), hbox, FALSE, FALSE, 5);

	label = gtk_label_new(_("Account: "));
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

	dialog->option_menu = gtk_option_menu_new();
	gtk_box_pack_start(GTK_BOX(hbox), dialog->option_menu, FALSE, FALSE, 0);

	g_signal_connect(G_OBJECT(dialog->option_menu), "changed",
			 G_CALLBACK(channel_input_dialog_option_menu_changed_cb), dialog);

	menu = gtk_menu_new();
	gtk_option_menu_set_menu(GTK_OPTION_MENU(dialog->option_menu), menu);

	account_list = account_manager_get_account_list(account_manager_get());
	for(cur = account_list; cur != NULL; cur = cur->next) {
		account = ACCOUNT(cur->data);
		
		menuitem = gtk_menu_item_new_with_label(account_get_name(account));
		g_object_ref(account);
		g_object_set_data_full(G_OBJECT(menuitem), "account", account, g_object_unref);
		
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	}

	vsep = gtk_vseparator_new();
	gtk_box_pack_start(GTK_BOX(hbox),vsep, FALSE, FALSE, 0);

	label = gtk_label_new(_("Channel: "));
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	
	dialog->combo = gtk_combo_new();
	gtk_box_pack_start(GTK_BOX(hbox), dialog->combo, TRUE, TRUE, 0);
	g_signal_connect(G_OBJECT(GTK_COMBO(dialog->combo)->entry), "activate",
			 G_CALLBACK(channel_input_dialog_entry_activated_cb), dialog);

	dialog->entry = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), dialog->entry, TRUE, TRUE, 5);
	g_signal_connect(G_OBJECT(dialog->entry), "activate",
			 G_CALLBACK(channel_input_dialog_entry_activated_cb), dialog);

	return GTK_WIDGET(dialog);
}

static void channel_input_dialog_update_combo(ChannelInputDialog *dialog)
{
	ChannelInputDialogPrivate *priv;
	GList *items = NULL;
	GSList *cur;
	Channel *channel;
	GtkList *list;

        g_return_if_fail(dialog != NULL);
        g_return_if_fail(IS_CHANNEL_INPUT_DIALOG(dialog));
	
	priv = dialog->priv;

	gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(dialog->combo)->entry), "");
	list = GTK_LIST(GTK_COMBO(dialog->combo)->list);
	gtk_list_clear_items(list, 0, -1);

	switch(priv->history_type) {
	case CHANNEL_HISTORY_NONE:
	case CHANNEL_HISTORY_SAVED:
		break;
	case CHANNEL_HISTORY_JOINED:
		if(priv->account == NULL) break;

		for(cur = priv->account->channel_list; cur != NULL; cur = cur->next) {
			channel = CHANNEL(cur->data);
			items = g_list_append(items, channel_get_name(channel));
		}
		if(items) {
			gtk_combo_set_popdown_strings(GTK_COMBO(dialog->combo), items);
			g_list_free(items);
		}

		break;
	default:
		g_assert_not_reached();
	}
}

void channel_input_dialog_set_channel_history_type(ChannelInputDialog *dialog, ChannelHistoryType history_type)
{
	ChannelInputDialogPrivate *priv;

        g_return_if_fail(dialog != NULL);
        g_return_if_fail(IS_CHANNEL_INPUT_DIALOG(dialog));
	
	priv = dialog->priv;

	priv->history_type = history_type;

	channel_input_dialog_update_combo(dialog);
}

ChannelHistoryType channel_input_dialog_get_channel_history_type(ChannelInputDialog *dialog)
{
	ChannelInputDialogPrivate *priv;

        g_return_val_if_fail(dialog != NULL, 0);
        g_return_val_if_fail(IS_CHANNEL_INPUT_DIALOG(dialog), 0);

	priv = dialog->priv;
	
	return priv->history_type;
}
void channel_input_dialog_set_account(ChannelInputDialog *dialog, Account *account)
{
	ChannelInputDialogPrivate *priv;
	GtkWidget *menu, *menuitem = NULL;
	Account *cur_ac;
	GList *cur;
	guint i = 0;

        g_return_if_fail(dialog != NULL);
        g_return_if_fail(IS_CHANNEL_INPUT_DIALOG(dialog));

	priv = dialog->priv;

	menu = gtk_option_menu_get_menu(GTK_OPTION_MENU(dialog->option_menu));
	for(cur = GTK_MENU_SHELL(menu)->children; cur != NULL; cur = cur->next) {
		menuitem = cur->data;
		if(!GTK_IS_MENU_ITEM(menuitem)) continue;

		cur_ac = g_object_get_data(G_OBJECT(menuitem), "account");
		if(cur_ac == account)
			break;

		i++;
	}
	gtk_option_menu_set_history(GTK_OPTION_MENU(dialog->option_menu), i);
}
Account *channel_input_dialog_get_account(ChannelInputDialog *dialog)
{
	ChannelInputDialogPrivate *priv;

        g_return_val_if_fail(dialog != NULL, NULL);
        g_return_val_if_fail(IS_CHANNEL_INPUT_DIALOG(dialog), NULL);

	priv = dialog->priv;

	return priv->account;
}

void channel_input_dialog_open(GtkWindow *parent_window, 
			       gchar *title, gchar *info_label,
			       ChannelHistoryType history_type, 
			       ChannelInputFunc func, gpointer data,
			       gboolean use_account, Account *account, 
			       gboolean use_channel, gchar *channel_name, 
			       gboolean use_text, gchar *default_text)
{
	ChannelInputDialog *dialog;
	ChannelInputDialogPrivate *priv;

	dialog = CHANNEL_INPUT_DIALOG(channel_input_dialog_new());
	priv = dialog->priv;

	gtk_window_set_transient_for(GTK_WINDOW(dialog), parent_window);
	if(title)
		gtk_window_set_title(GTK_WINDOW(dialog), title);

	if(info_label)
		gtk_label_set_text(GTK_LABEL(dialog->label), info_label);

	if(account != NULL)
		channel_input_dialog_set_account(dialog, account);

	channel_input_dialog_set_channel_history_type(dialog, history_type);
	
	if(channel_name)
		gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(dialog->combo)->entry), channel_name);

	if(default_text)
		gtk_entry_set_text(GTK_ENTRY(dialog->entry), default_text);

	gtk_widget_set_sensitive(dialog->option_menu, use_account);
	gtk_widget_set_sensitive(dialog->combo, use_channel);
	gtk_widget_set_sensitive(dialog->entry, use_text);

	if(!use_text)
		gtk_widget_grab_focus(GTK_COMBO(dialog->combo)->entry);
	else
		gtk_widget_grab_focus(dialog->entry);

	priv->func = func;
	priv->data = data;

	gtk_widget_show_all(GTK_WIDGET(dialog));
}
