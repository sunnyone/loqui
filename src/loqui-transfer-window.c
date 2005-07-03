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

#include "loqui-transfer-window.h"
#include <glib/gi18n.h>

enum {
        LAST_SIGNAL
};

enum {
        LAST_PROP
};

struct _LoquiTransferWindowPrivate
{
	GtkWidget *handlebox_toolbar;
	GtkWidget *toolbar;
	GtkWidget *toggletb_info;
	GtkWidget *treeview;
	GtkWidget *button_dir;
	GtkWidget *entry_filename;
	GtkWidget *entry_directory;
	GtkWidget *label_updown;
	GtkWidget *label_status;
	GtkWidget *label_path;
	GtkWidget *progressbar;
	GtkWidget *label_size;
	GtkWidget *label_rate;
	GtkWidget *textview_log;
	GtkWidget *statusbar;
};

static GtkWindowClass *parent_class = NULL;

/* static guint loqui_transfer_window_signals[LAST_SIGNAL] = { 0 }; */

static GObject* loqui_transfer_window_constructor(GType type, guint n_props, GObjectConstructParam *props);

static void loqui_transfer_window_class_init(LoquiTransferWindowClass *klass);
static void loqui_transfer_window_init(LoquiTransferWindow *window);
static void loqui_transfer_window_finalize(GObject *object);
static void loqui_transfer_window_dispose(GObject *object);

static void loqui_transfer_window_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec);
static void loqui_transfer_window_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec);

static void loqui_transfer_window_destroy(GtkObject *object);
GType
loqui_transfer_window_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(LoquiTransferWindowClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) loqui_transfer_window_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(LoquiTransferWindow),
				0,              /* n_preallocs */
				(GInstanceInitFunc) loqui_transfer_window_init
			};
		
		type = g_type_register_static(GTK_TYPE_WINDOW,
					      "LoquiTransferWindow",
					      &our_info,
					      0);
	}
	
	return type;
}
static GObject*
loqui_transfer_window_constructor(GType type, guint n_props, GObjectConstructParam *props)
{
        GObject *object;
        GObjectClass *object_class = G_OBJECT_CLASS(parent_class);

	object = object_class->constructor(type, n_props, props);

	return object;
}
static void 
loqui_transfer_window_finalize(GObject *object)
{
	LoquiTransferWindow *window;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_TRANSFER_WINDOW(object));

        window = LOQUI_TRANSFER_WINDOW(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize)(object);

	g_free(window->priv);
}
static void 
loqui_transfer_window_dispose(GObject *object)
{
	LoquiTransferWindow *window;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_TRANSFER_WINDOW(object));

        window = LOQUI_TRANSFER_WINDOW(object);

        if (G_OBJECT_CLASS(parent_class)->dispose)
                (* G_OBJECT_CLASS(parent_class)->dispose)(object);
}
static void
loqui_transfer_window_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec)
{
        LoquiTransferWindow *window;        

        window = LOQUI_TRANSFER_WINDOW(object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}
static void
loqui_transfer_window_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec)
{
        LoquiTransferWindow *window;        

        window = LOQUI_TRANSFER_WINDOW(object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}

static void
loqui_transfer_window_class_init(LoquiTransferWindowClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);

	object_class->constructor = loqui_transfer_window_constructor; 
        object_class->finalize = loqui_transfer_window_finalize;
        object_class->dispose = loqui_transfer_window_dispose;
        object_class->get_property = loqui_transfer_window_get_property;
        object_class->set_property = loqui_transfer_window_set_property;
        GTK_OBJECT_CLASS(klass)->destroy = loqui_transfer_window_destroy;
}
static void 
loqui_transfer_window_init(LoquiTransferWindow *window)
{
	LoquiTransferWindowPrivate *priv;

	priv = g_new0(LoquiTransferWindowPrivate, 1);

	window->priv = priv;
}
static void 
loqui_transfer_window_destroy(GtkObject *object)
{
        LoquiTransferWindow *window;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_TRANSFER_WINDOW(object));

        window = LOQUI_TRANSFER_WINDOW(object);

        if (GTK_OBJECT_CLASS(parent_class)->destroy)
                (* GTK_OBJECT_CLASS(parent_class)->destroy)(object);
}
GtkWidget *
loqui_transfer_window_new(void)
{
        LoquiTransferWindow *window;
	LoquiTransferWindowPrivate *priv;

	GtkWidget *vbox1;
	GtkWidget *handlebox_toolbar;
	GtkWidget *toolbar;
	GtkWidget *image;
	GtkWidget *toolbutton_start;
	GtkWidget *toolbutton_stop;
	GtkWidget *toggletb_info;
	GtkWidget *vpaned1;
	GtkWidget *hpaned1;
	GtkWidget *scrolled_win;
	GtkWidget *treeview;
	GtkWidget *frame1;
	GtkWidget *viewport1;
	GtkWidget *table;
	GtkWidget *label;
	GtkWidget *button_dir;
	GtkWidget *hseparator1;
	GtkWidget *entry_filename;
	GtkWidget *entry_directory;
	GtkWidget *label_updown;
	GtkWidget *label_status;
	GtkWidget *label_path;
	GtkWidget *progressbar;
	GtkWidget *label_size;
	GtkWidget *label_rate;
	GtkWidget *textview_log;
	GtkWidget *statusbar;

	gint iconsize;
	gint row;

	window = g_object_new(loqui_transfer_window_get_type(), NULL);
	
        priv = window->priv;

	gtk_window_set_policy(GTK_WINDOW(window), TRUE, TRUE, TRUE);
	gtk_window_set_title(GTK_WINDOW(window), _("File Transfer Manager"));

	vbox1 = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), vbox1);

	handlebox_toolbar = gtk_handle_box_new();
	gtk_box_pack_start(GTK_BOX(vbox1), handlebox_toolbar, FALSE, FALSE, 0);

	toolbar = gtk_toolbar_new();
	gtk_container_add(GTK_CONTAINER(handlebox_toolbar), toolbar);
	gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_BOTH);
	iconsize = gtk_toolbar_get_icon_size(GTK_TOOLBAR(toolbar));

	image = gtk_image_new_from_stock(GTK_STOCK_GO_FORWARD, iconsize);
	toolbutton_start = GTK_WIDGET(gtk_tool_button_new(image, _("Start")));
	gtk_container_add(GTK_CONTAINER(toolbar), toolbutton_start);

	image = gtk_image_new_from_stock(GTK_STOCK_STOP, iconsize);
	toolbutton_stop = GTK_WIDGET(gtk_tool_button_new(image, _("Stop")));
	gtk_container_add(GTK_CONTAINER(toolbar), toolbutton_stop);

	toggletb_info = GTK_WIDGET(gtk_toggle_tool_button_new());
	gtk_tool_button_set_label(GTK_TOOL_BUTTON(toggletb_info), _("Info"));
	image = gtk_image_new_from_stock(GTK_STOCK_PROPERTIES, iconsize);
	gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(toggletb_info), image);
	gtk_container_add(GTK_CONTAINER(toolbar), toggletb_info);

	vpaned1 = gtk_vpaned_new();
	gtk_box_pack_start(GTK_BOX(vbox1), vpaned1, TRUE, TRUE, 0);

	hpaned1 = gtk_hpaned_new();
	gtk_paned_pack1(GTK_PANED(vpaned1), hpaned1, FALSE, TRUE);

	scrolled_win = gtk_scrolled_window_new(NULL, NULL);
	gtk_paned_pack1(GTK_PANED(hpaned1), scrolled_win, FALSE, TRUE);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_win),
				       GTK_POLICY_AUTOMATIC,
				       GTK_POLICY_AUTOMATIC);

	treeview = gtk_tree_view_new();
	gtk_container_add(GTK_CONTAINER(scrolled_win), treeview);

	frame1 = gtk_frame_new(NULL);
	gtk_frame_set_label(GTK_FRAME(frame1), _("Information"));
	gtk_paned_pack2(GTK_PANED(hpaned1), frame1, TRUE, TRUE);


	scrolled_win = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_win),
				       GTK_POLICY_AUTOMATIC,
				       GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(frame1), scrolled_win);

	viewport1 = gtk_viewport_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(scrolled_win), viewport1);

	table = gtk_table_new(10, 3, FALSE);
	gtk_container_add(GTK_CONTAINER(viewport1), table);
	gtk_table_set_row_spacings(GTK_TABLE(table), 2);
	gtk_table_set_col_spacings(GTK_TABLE(table), 2);

	row = 0;

	label = gtk_label_new(_("Filename:"));
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, row, row+1,
			 (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	entry_filename = gtk_entry_new();
	gtk_widget_show(entry_filename);
	gtk_table_attach(GTK_TABLE(table), entry_filename, 1, 3, 0, 1,
			 (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 2);

	row++;

	label = gtk_label_new(_("Directory:"));
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, row, row+1,
			 (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	entry_directory = gtk_entry_new();
	gtk_widget_show(entry_directory);
	gtk_table_attach(GTK_TABLE(table), entry_directory, 1, 2, row, row+1,
			 (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 2);

	button_dir = gtk_button_new_with_mnemonic(_("Select..."));
	gtk_table_attach(GTK_TABLE(table), button_dir, 2, 3, row, row+1,
			 (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;

	hseparator1 = gtk_hseparator_new();
	gtk_table_attach(GTK_TABLE(table), hseparator1, 0, 3, row, row+1,
			 (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (GTK_FILL), 0, 0);
	gtk_widget_set_size_request(hseparator1, -1, 5);

	row++;

	label = gtk_label_new(_("Up / Down:"));
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, row, row+1,
			 (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	label_updown = gtk_label_new(_("Down"));
	gtk_table_attach(GTK_TABLE(table), label_updown, 1, 3, row, row+1,
			 (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 2);

	row++;

	label = gtk_label_new(_("Status:"));
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, row, row+1,
			 (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	label_status = gtk_label_new(_("Transferring"));
	gtk_table_attach(GTK_TABLE(table), label_status, 1, 3, row, row+1,
			 (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 2);

	row++;

	label = gtk_label_new(_("Path:"));
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, row, row+1,
			 (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	label_path = gtk_label_new(_("hoge.jpg"));
	gtk_table_attach(GTK_TABLE(table), label_path, 1, 3, row, row+1,
			 (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 2);

	row++;

	label = gtk_label_new(_("Progress:"));
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, row, row+1,
			 (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	progressbar = gtk_progress_bar_new();
	gtk_widget_show(progressbar);
	gtk_table_attach(GTK_TABLE(table), progressbar, 1, 3, row, row+1,
			 (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 2);
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progressbar), 0.5);
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progressbar), _("50%"));

	row++;

	label = gtk_label_new(_("Size:"));
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, row, row+1,
			 (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	label_size = gtk_label_new(_("10,243,000 / 50,222,222"));
	gtk_widget_show(label_size);
	gtk_table_attach(GTK_TABLE(table), label_size, 1, 3, row, row+1,
			 (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 2);
	gtk_label_set_justify(GTK_LABEL(label_size), GTK_JUSTIFY_CENTER);

	row++;

	label = gtk_label_new(_("Rate:"));
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, row, row+1,
			 (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	label_rate = gtk_label_new(_("50.2 KB/s"));
	gtk_table_attach(GTK_TABLE(table), label_rate, 1, 3, row, row+1,
			 (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 2);
	gtk_label_set_justify(GTK_LABEL(label_rate), GTK_JUSTIFY_CENTER);


	scrolled_win = gtk_scrolled_window_new(NULL, NULL);
	gtk_widget_show(scrolled_win);
	gtk_paned_pack2(GTK_PANED(vpaned1), scrolled_win, TRUE, TRUE);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_win), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

	textview_log = gtk_text_view_new();
	gtk_container_add(GTK_CONTAINER(scrolled_win), textview_log);

	statusbar = gtk_statusbar_new();
	gtk_box_pack_start(GTK_BOX(vbox1), statusbar, FALSE, FALSE, 0);

	priv->handlebox_toolbar = handlebox_toolbar;
	priv->toolbar = toolbar;
	priv->toggletb_info = toggletb_info;
	priv->treeview = treeview;
	priv->button_dir = button_dir;
	priv->entry_filename = entry_filename;
	priv->entry_directory = entry_directory;
	priv->label_updown = label_updown;
	priv->label_status = label_status;
	priv->label_path = label_path;
	priv->progressbar = progressbar;
	priv->label_size = label_size;
	priv->label_rate = label_rate;
	priv->textview_log = textview_log;
	priv->statusbar = statusbar;

	gtk_widget_set_size_request(GTK_WIDGET(window), 600, 400);
	gtk_widget_set_size_request(GTK_WIDGET(priv->treeview), 300, 250);

        return GTK_WIDGET(window);
}
