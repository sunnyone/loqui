/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui -- IRC client for GNOME2
 * Copyright (C) 2002 Yoichi Imai <yoichi@silver-forest.com>
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

#include "loqui_app.h"
#include "loqui_gconf.h"
#include "channel_tree.h"
#include "connection.h"
#include "utils.h"
#include "channel_book.h"
#include "nick_list.h"
#include "account_manager.h"

struct _LoquiAppPrivate
{
	GtkWidget *label_topic;
	GtkWidget *toggle_scroll;
};

static GnomeAppClass *parent_class = NULL;
#define PARENT_TYPE GNOME_TYPE_APP

static void loqui_app_class_init(LoquiAppClass *klass);
static void loqui_app_init(LoquiApp *app);
static void loqui_app_finalize(GObject *object);
static void loqui_app_destroy(GtkObject *object);

static gint loqui_app_delete_event_cb(GtkWidget *widget, GdkEventAny *event);

static void loqui_app_restore_size(LoquiApp *app);
static void loqui_app_save_size(LoquiApp *app);
static void loqui_app_entry_activate_cb(GtkWidget *widget, gpointer data);

GType
loqui_app_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(LoquiAppClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) loqui_app_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(LoquiApp),
				0,              /* n_preallocs */
				(GInstanceInitFunc) loqui_app_init
			};
		
		type = g_type_register_static(PARENT_TYPE,
					      "LoquiApp",
					      &our_info,
					      0);
	}
	
	return type;
}
static void
loqui_app_class_init (LoquiAppClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);
        GtkObjectClass *gtk_object_class = GTK_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
	
        object_class->finalize = loqui_app_finalize;
	gtk_object_class->destroy = loqui_app_destroy;
}
static void 
loqui_app_init (LoquiApp *app)
{
	LoquiAppPrivate *priv;

	priv = g_new0(LoquiAppPrivate, 1);
	app->priv = priv;

	app->menu = NULL;
}
static void
loqui_app_destroy(GtkObject *object)
{
	LoquiApp *app;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_APP(object));

	app = LOQUI_APP(object);
	
	if(app->menu) {
		g_object_unref(app->menu);
		app->menu = NULL;
	}
	
	if (GTK_OBJECT_CLASS(parent_class)->destroy)
                (* GTK_OBJECT_CLASS(parent_class)->destroy) (object);

}	
static void 
loqui_app_finalize (GObject *object)
{
	LoquiApp *app;

        g_return_if_fail (object != NULL);
        g_return_if_fail (LOQUI_IS_APP(object));

        app = LOQUI_APP(object);

	if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize) (object);

	g_free(app->priv);
}

static gint
loqui_app_delete_event_cb(GtkWidget *widget, GdkEventAny *event)
{
        if (eel_gconf_get_boolean(LOQUI_GCONF_SIZE "/save_size")) {
		loqui_app_save_size(LOQUI_APP(widget));
        }
	gtk_widget_destroy(widget);
	gtk_main_quit();

	return TRUE;
}
static void loqui_app_save_size(LoquiApp *app)
{
	gint height;
	gint width;

	gtk_window_get_size(GTK_WINDOW(app), &width, &height);
	eel_gconf_set_integer(LOQUI_GCONF_SIZE "/window_width", width);
	eel_gconf_set_integer(LOQUI_GCONF_SIZE "/window_height", height);
	
	/* TODO: save other size */
}
static void loqui_app_restore_size(LoquiApp *app)
{
	gint height;
	gint width;

	height = eel_gconf_get_integer(LOQUI_GCONF_SIZE "/window_width");
        width = eel_gconf_get_integer(LOQUI_GCONF_SIZE "/window_height");
        gtk_window_set_default_size(GTK_WINDOW(app), height, width);
}

static void
loqui_app_entry_activate_cb(GtkWidget *widget, gpointer data)
{
	const gchar *str;

	str = gtk_entry_get_text(GTK_ENTRY(widget));
	if(str == NULL || strlen(str) == 0)
		return;
	account_manager_speak(account_manager_get(), str);
}
GtkWidget*
loqui_app_new (void)
{
	LoquiApp *app;
	LoquiAppPrivate *priv;

	GtkWidget *channel_book;
	GtkWidget *channel_tree;
	GtkWidget *nick_list;
	GtkWidget *common_text;

	GtkWidget *appbar;
	GtkWidget *vbox;
	GtkWidget *entry;
	GtkWidget *hbox;
	GtkWidget *hpaned;
	GtkWidget *vpaned;
	GtkWidget *scrolled_win;

	app = g_object_new(loqui_app_get_type(), NULL);
	priv = app->priv;

	g_signal_connect(G_OBJECT(app), "delete_event",
			 G_CALLBACK(loqui_app_delete_event_cb), NULL);

	gnome_app_construct(GNOME_APP(app),
			    "Loqui",
			    _("Loqui"));
	gtk_window_set_policy (GTK_WINDOW (app), TRUE, TRUE, TRUE);

	vbox = gtk_vbox_new(FALSE, 0);
	gnome_app_set_contents(GNOME_APP(app), vbox);

	/* topic line */
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

	priv->toggle_scroll = gtk_toggle_button_new_with_mnemonic("Sc_roll");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->toggle_scroll), TRUE);
	gtk_box_pack_start(GTK_BOX(hbox), priv->toggle_scroll, FALSE, FALSE, 0);

	priv->label_topic = gtk_label_new("Topic");
	gtk_label_set_selectable(GTK_LABEL(priv->label_topic), TRUE);
	gtk_label_set_justify(GTK_LABEL(priv->label_topic), GTK_JUSTIFY_LEFT);

	gtk_box_pack_start_defaults(GTK_BOX(hbox), priv->label_topic);
	
#define SET_SCROLLED_WINDOW(s, w, vpolicy, hpolicy) \
{ \
	s = gtk_scrolled_window_new(NULL, NULL); \
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(s), vpolicy, hpolicy); \
	gtk_container_add(GTK_CONTAINER(s), w); \
}

	hpaned = gtk_hpaned_new();
	gtk_box_pack_start_defaults(GTK_BOX(vbox), hpaned);

	/* left side */
	vpaned = gtk_vpaned_new();
	gtk_paned_pack1(GTK_PANED(hpaned), vpaned, TRUE, TRUE);
	
	vbox = gtk_vbox_new(FALSE, 0);
	gtk_paned_pack1(GTK_PANED(vpaned), vbox, TRUE, TRUE);

	channel_book = channel_book_new();
	gtk_box_pack_start_defaults(GTK_BOX(vbox), channel_book);

	/* TODO: this should be replaced with a widget considered multiline editing */
	entry = gtk_entry_new();
	gtk_box_pack_end(GTK_BOX(vbox), entry, FALSE, FALSE, 0);
	gtk_window_set_focus(GTK_WINDOW(app), entry);
	g_signal_connect(G_OBJECT(entry), "activate",
                         G_CALLBACK(loqui_app_entry_activate_cb), NULL);

	common_text = channel_text_new();
	gtk_paned_pack2(GTK_PANED(vpaned), common_text, FALSE, TRUE);

	/* right side */
	vpaned = gtk_vpaned_new();
	gtk_paned_pack2(GTK_PANED(hpaned), vpaned, FALSE, TRUE);
	
	nick_list = nick_list_new();
	SET_SCROLLED_WINDOW(scrolled_win, nick_list, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_paned_pack1(GTK_PANED(vpaned), scrolled_win, TRUE, TRUE);	

	channel_tree = channel_tree_new();
	SET_SCROLLED_WINDOW(scrolled_win, channel_tree, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC); 
	gtk_paned_pack2(GTK_PANED(vpaned), scrolled_win, FALSE, TRUE);
	
	app->channel_tree = CHANNEL_TREE(channel_tree);
	app->channel_book = CHANNEL_BOOK(channel_book);
	app->nick_list = NICK_LIST(nick_list);
	app->common_text = CHANNEL_TEXT(common_text);

	/* status bar */
	appbar = gnome_appbar_new(FALSE, TRUE, GNOME_PREFERENCES_NEVER);
        gnome_app_set_statusbar(GNOME_APP(app), appbar);
        gnome_appbar_set_default(GNOME_APPBAR(appbar),  "Loqui version " VERSION);

	app->menu = loqui_menu_new();
	loqui_menu_attach(app->menu, GNOME_APP(app));

	loqui_app_restore_size(app);

#undef SET_SCROLLED_WINDOW

	return GTK_WIDGET(app);
}

void
loqui_app_set_topic(LoquiApp *app, const gchar *str)
{
	gtk_label_set_text(GTK_LABEL(app->priv->label_topic), str);
}

gboolean
loqui_app_is_scroll(LoquiApp *app)
{
	LoquiAppPrivate *priv;

	priv = app->priv;

	return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->toggle_scroll));
}
