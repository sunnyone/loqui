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

#include "loqui_select_dialog.h"
#include <glib/gi18n.h>

#include <loqui_channel_entry.h>
#include <loqui_account_manager_iter.h>
#include <loqui_channel_entry_utils.h>

enum {
        LAST_SIGNAL
};

enum {
        LAST_PROP
};

enum {
	COLUMN_NAME,
	COLUMN_CHANNEL_NUMBER,
	COLUMN_ACCOUNT_NAME,
	COLUMN_CHANNEL_NAME,

	COLUMN_CHANNEL_ENTRY,
	N_COLUMNS
};

struct _LoquiSelectDialogPrivate
{
	LoquiApp *app;

	GtkWidget *entry_keyword;
	GtkWidget *check_case;
	GtkWidget *check_migemo;
	GtkWidget *treeview;
	GtkWidget *label_account;
	GtkWidget *label_channel;

	GtkListStore *store;
};

static GtkDialogClass *parent_class = NULL;

/* static guint loqui_select_dialog_signals[LAST_SIGNAL] = { 0 }; */

static GObject* loqui_select_dialog_constructor(GType type, guint n_props, GObjectConstructParam *props);

static void loqui_select_dialog_class_init(LoquiSelectDialogClass *klass);
static void loqui_select_dialog_init(LoquiSelectDialog *sdialog);
static void loqui_select_dialog_finalize(GObject *object);
static void loqui_select_dialog_dispose(GObject *object);

static void loqui_select_dialog_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec);
static void loqui_select_dialog_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec);

static void loqui_select_dialog_destroy(GtkObject *object);
GType
loqui_select_dialog_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(LoquiSelectDialogClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) loqui_select_dialog_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(LoquiSelectDialog),
				0,              /* n_preallocs */
				(GInstanceInitFunc) loqui_select_dialog_init
			};
		
		type = g_type_register_static(GTK_TYPE_DIALOG,
					      "LoquiSelectDialog",
					      &our_info,
					      0);
	}
	
	return type;
}
static GObject*
loqui_select_dialog_constructor(GType type, guint n_props, GObjectConstructParam *props)
{
        GObject *object;
        GObjectClass *object_class = G_OBJECT_CLASS(parent_class);

	object = object_class->constructor(type, n_props, props);

	return object;
}
static void 
loqui_select_dialog_finalize(GObject *object)
{
	LoquiSelectDialog *sdialog;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_SELECT_DIALOG(object));

        sdialog = LOQUI_SELECT_DIALOG(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize)(object);

	g_free(sdialog->priv);
}
static void 
loqui_select_dialog_dispose(GObject *object)
{
	LoquiSelectDialog *sdialog;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_SELECT_DIALOG(object));

        sdialog = LOQUI_SELECT_DIALOG(object);

        if (G_OBJECT_CLASS(parent_class)->dispose)
                (* G_OBJECT_CLASS(parent_class)->dispose)(object);
}
static void
loqui_select_dialog_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec)
{
        LoquiSelectDialog *sdialog;        

        sdialog = LOQUI_SELECT_DIALOG(object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}
static void
loqui_select_dialog_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec)
{
        LoquiSelectDialog *sdialog;        

        sdialog = LOQUI_SELECT_DIALOG(object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}

static void
loqui_select_dialog_class_init(LoquiSelectDialogClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);

	object_class->constructor = loqui_select_dialog_constructor; 
        object_class->finalize = loqui_select_dialog_finalize;
        object_class->dispose = loqui_select_dialog_dispose;
        object_class->get_property = loqui_select_dialog_get_property;
        object_class->set_property = loqui_select_dialog_set_property;
        GTK_OBJECT_CLASS(klass)->destroy = loqui_select_dialog_destroy;
}
static void 
loqui_select_dialog_init(LoquiSelectDialog *sdialog)
{
	LoquiSelectDialogPrivate *priv;

	priv = g_new0(LoquiSelectDialogPrivate, 1);

	sdialog->priv = priv;
}
static void 
loqui_select_dialog_destroy(GtkObject *object)
{
        LoquiSelectDialog *sdialog;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_SELECT_DIALOG(object));

        sdialog = LOQUI_SELECT_DIALOG(object);

        if (GTK_OBJECT_CLASS(parent_class)->destroy)
                (* GTK_OBJECT_CLASS(parent_class)->destroy)(object);
}
GtkWidget *
loqui_select_dialog_new(LoquiApp *app)
{
        LoquiSelectDialog *sdialog;
	LoquiSelectDialogPrivate *priv;

	GtkWidget *dialog_vbox;
	GtkWidget *hbox1;
	GtkWidget *label1;
	GtkWidget *hseparator1;
	GtkWidget *scrolledwindow1;
	GtkWidget *hseparator2;
	GtkWidget *frame1;
	GtkWidget *hbox3;
	GtkWidget *hbox4;
	GtkWidget *label7;
	GtkWidget *vseparator1;
	GtkWidget *hbox5;
	GtkWidget *label8;
	GtkTreeViewColumn *column;
	GtkCellRenderer *renderer;

	sdialog = g_object_new(loqui_select_dialog_get_type(), NULL);
	
        priv = sdialog->priv;
	priv->app = app;

	gtk_window_set_title(GTK_WINDOW(sdialog), _("Selection Dialog"));
	gtk_dialog_add_buttons(GTK_DIALOG(sdialog), 
			       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			       GTK_STOCK_OK, GTK_RESPONSE_OK,
			       NULL);

	dialog_vbox = GTK_DIALOG(sdialog)->vbox;

	hbox1 = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(dialog_vbox), hbox1, FALSE, FALSE, 5);

	label1 = gtk_label_new(_("Keyword:"));
	gtk_box_pack_start(GTK_BOX(hbox1), label1, FALSE, FALSE, 0);

	priv->entry_keyword = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox1), priv->entry_keyword, TRUE, TRUE, 0);

	priv->check_case = gtk_check_button_new_with_mnemonic(_("Case sensitive"));
	gtk_box_pack_start(GTK_BOX(dialog_vbox), priv->check_case, FALSE, FALSE, 0);

	priv->check_migemo = gtk_check_button_new_with_mnemonic(_("Use migemo"));
	gtk_box_pack_start(GTK_BOX(dialog_vbox), priv->check_migemo, FALSE, FALSE, 0);
	gtk_widget_set_sensitive(priv->check_migemo, FALSE);

	hseparator1 = gtk_hseparator_new();
	gtk_box_pack_start(GTK_BOX(dialog_vbox), hseparator1, FALSE, FALSE, 5);

	scrolledwindow1 = gtk_scrolled_window_new(NULL, NULL);
	gtk_box_pack_start(GTK_BOX(dialog_vbox), scrolledwindow1, TRUE, TRUE, 0);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledwindow1), GTK_POLICY_AUTOMATIC,
				       GTK_POLICY_AUTOMATIC);

	priv->store = gtk_list_store_new(N_COLUMNS,
					 G_TYPE_STRING,
					 G_TYPE_INT,
					 G_TYPE_STRING,
					 G_TYPE_STRING,
					 LOQUI_TYPE_CHANNEL_ENTRY);

	priv->treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(priv->store));
	gtk_container_add(GTK_CONTAINER(scrolledwindow1), priv->treeview);

        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes(_("#"),
							  renderer,
							  "text", COLUMN_CHANNEL_NUMBER,
							  NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(priv->treeview), column);

        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes(_("Account"),
							  renderer,
							  "text", COLUMN_ACCOUNT_NAME,
							  NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(priv->treeview), column);

        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes(_("Channel"),
							  renderer,
							  "text", COLUMN_CHANNEL_NAME,
							  NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(priv->treeview), column);
	
	hseparator2 = gtk_hseparator_new();
	gtk_box_pack_start(GTK_BOX(dialog_vbox), hseparator2, FALSE, FALSE, 5);

	frame1 = gtk_frame_new(_("Target"));
	gtk_box_pack_start(GTK_BOX(dialog_vbox), frame1, FALSE, FALSE, 5);
	gtk_frame_set_shadow_type(GTK_FRAME(frame1), GTK_SHADOW_ETCHED_OUT);

	hbox3 = gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(frame1), hbox3);
	gtk_container_set_border_width(GTK_CONTAINER(hbox3), 5);

	hbox4 = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox3), hbox4, TRUE, TRUE, 0);

	label7 = gtk_label_new(_("Account:"));
	gtk_box_pack_start(GTK_BOX(hbox4), label7, FALSE, FALSE, 0);

	priv->label_account = gtk_label_new("");
	gtk_box_pack_start(GTK_BOX(hbox4), priv->label_account, FALSE, FALSE, 0);
	gtk_label_set_selectable(GTK_LABEL(priv->label_account), TRUE);

	vseparator1 = gtk_vseparator_new();
	gtk_box_pack_start(GTK_BOX(hbox3), vseparator1, FALSE, FALSE, 5);

	hbox5 = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox3), hbox5, TRUE, TRUE, 0);

	label8 = gtk_label_new(_("Channel:"));
	gtk_box_pack_start(GTK_BOX(hbox5), label8, FALSE, FALSE, 0);

	priv->label_channel = gtk_label_new("");
	gtk_box_pack_start(GTK_BOX(hbox5), priv->label_channel, FALSE, FALSE, 0);
	gtk_label_set_selectable(GTK_LABEL(priv->label_channel), TRUE);

        return GTK_WIDGET(sdialog);
}
void
loqui_select_dialog_construct_channel_entry_list(LoquiSelectDialog *sdialog)
{
	LoquiSelectDialogPrivate *priv;
	LoquiChannelEntry *chent;
	LoquiAccountManagerIter iter;
	GtkTreeIter iter_tree;
	LoquiAccount *account;
	LoquiChannel *channel;
	const gchar *account_name, *channel_name;
	priv = sdialog->priv;
	
	loqui_account_manager_iter_init(loqui_app_get_account_manager(priv->app), &iter);
	loqui_account_manager_iter_set_first_channel_entry(&iter);
	while ((chent = loqui_account_manager_iter_channel_entry_next(&iter)) != NULL) {
		loqui_channel_entry_utils_separate(chent, &account, &channel);

		account_name = account ? loqui_channel_entry_get_name(LOQUI_CHANNEL_ENTRY(account)) : "";
		channel_name = channel ? loqui_channel_entry_get_name(LOQUI_CHANNEL_ENTRY(channel)) : "";

		gtk_list_store_append(priv->store,
				      &iter_tree);

		gtk_list_store_set(priv->store,
				   &iter_tree,
				   COLUMN_CHANNEL_NUMBER, loqui_channel_entry_get_position(chent),
				   COLUMN_ACCOUNT_NAME, account_name,
				   COLUMN_CHANNEL_NAME, channel_name,
				   COLUMN_CHANNEL_ENTRY, chent,
				   -1);
	}
}
