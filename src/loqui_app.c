/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui -- IRC client for Gtk2 <http://loqui.good-day.net/>
 * Copyright (C) 2002-2003 Yoichi Imai <yoichi@silver-forest.com>
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
#include "channel_tree.h"
#include "connection.h"
#include "utils.h"
#include "nick_list.h"
#include "account_manager.h"
#include "prefs_general.h"
#include "loqui_toolbar.h"

#include "intl.h"
#include "utils.h"

#include <string.h>

struct _LoquiAppPrivate
{
	GtkWidget *common_textview;
	GtkWidget *entry;
	GtkWidget *statusbar;
	GtkWidget *label_user_number;
	GtkWidget *label_channel;
	GtkWidget *label_channel_mode;
	GtkWidget *label_account;

	guint channel_buffer_inserted_signal_id;
	guint common_buffer_inserted_signal_id;
};

static GtkWindowClass *parent_class = NULL;
#define PARENT_TYPE GTK_TYPE_WINDOW

static void loqui_app_class_init(LoquiAppClass *klass);
static void loqui_app_init(LoquiApp *app);
static void loqui_app_finalize(GObject *object);
static void loqui_app_destroy(GtkObject *object);

static gint loqui_app_delete_event_cb(GtkWidget *widget, GdkEventAny *event);

static void loqui_app_restore_size(LoquiApp *app);
static void loqui_app_save_size(LoquiApp *app);
static void loqui_app_entry_activate_cb(GtkWidget *widget, gpointer data);
static void loqui_app_text_buffer_inserted_cb(GtkTextBuffer *textbuf,
					      GtkTextIter *arg1,
					      gchar *arg2,
					      gint arg3,
					      gpointer data);

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
	priv->channel_buffer_inserted_signal_id = 0;
	priv->common_buffer_inserted_signal_id = 0;
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
	account_manager_disconnect_all(account_manager_get());

        if (prefs_general.save_size) {
		loqui_app_save_size(LOQUI_APP(widget));
        }
	gtk_widget_destroy(widget);
	gtk_main_quit();

	return TRUE;
}
static void loqui_app_save_size(LoquiApp *app)
{
	LoquiAppPrivate *priv;

	gint height;
	gint width;

        g_return_if_fail(app != NULL);
        g_return_if_fail(LOQUI_IS_APP(app));

	priv = app->priv;

	gtk_window_get_size(GTK_WINDOW(app), &width, &height);
	prefs_general.window_width = width;
	prefs_general.window_height = height;
	
	prefs_general.common_buffer_height = priv->common_textview->allocation.height;

	prefs_general.channel_tree_height = GTK_WIDGET(app->channel_tree)->allocation.height;
	prefs_general.channel_tree_width = GTK_WIDGET(app->channel_tree)->allocation.width;
}
static void loqui_app_restore_size(LoquiApp *app)
{
	LoquiAppPrivate *priv;

        g_return_if_fail(app != NULL);
        g_return_if_fail(LOQUI_IS_APP(app));

	priv = app->priv;

        gtk_window_set_default_size(GTK_WINDOW(app),
				    prefs_general.window_width,
				    prefs_general.window_height);

	gtk_widget_set_usize(priv->common_textview, -1,
			     prefs_general.common_buffer_height);

	gtk_widget_set_usize(GTK_WIDGET(app->channel_tree),
			     prefs_general.channel_tree_width,
			     prefs_general.channel_tree_height);
}

static void
loqui_app_entry_activate_cb(GtkWidget *widget, gpointer data)
{
	Account *account;
	const gchar *str;

	str = gtk_entry_get_text(GTK_ENTRY(widget));
	if(str == NULL || strlen(str) == 0)
		return;
	
	account = account_manager_get_current_account(account_manager_get());
	if(account)
		account_speak(account, account_manager_get_current_channel(account_manager_get()), str);

	gtk_entry_set_text(GTK_ENTRY(widget), "");
}
static void
loqui_app_text_buffer_inserted_cb(GtkTextBuffer *textbuf,
				  GtkTextIter *arg1,
				  gchar *arg2,
				  gint arg3,
				  gpointer data)
{
	GtkTextView *textview;

	textview = GTK_TEXT_VIEW(data);
	if(!account_manager_get_whether_scrolling(account_manager_get()))
		return;

	gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(textview),
					   gtk_text_buffer_get_mark(textbuf, "end"));
}
void
loqui_app_set_current_info(LoquiApp *app, const gchar *account_name, 
			   const gchar *channel_name, const gchar *channel_mode,
			   const gchar *topic, gint user_number, gint op_number)
{
	LoquiAppPrivate *priv;
	gchar *buf, *title;
	guint context_id;
	
	priv = app->priv;

	if(channel_name && account_name) {
		title = g_strdup_printf("%s @ %s - Loqui version %s", channel_name, account_name, VERSION);
	} else if(account_name) {
		title = g_strdup_printf("%s - Loqui version %s", account_name, VERSION);
	} else {
		title = g_strdup_printf("Loqui version %s", VERSION);
	}
	gtk_window_set_title(GTK_WINDOW(app), title);
	g_free(title);

	gtk_label_set(GTK_LABEL(priv->label_channel), "");
	if(channel_name) {
		gtk_label_set(GTK_LABEL(priv->label_channel), channel_name);
	}

	gtk_label_set(GTK_LABEL(priv->label_account), "");
	if(account_name) {
		gtk_label_set(GTK_LABEL(priv->label_account), account_name);
	}

	gtk_label_set(GTK_LABEL(priv->label_channel_mode), "");
	if(channel_mode) {
		buf = g_strdup_printf("[%s]", channel_mode);
		gtk_label_set(GTK_LABEL(priv->label_channel_mode), buf);
		g_free(buf);
	}
	gtk_label_set(GTK_LABEL(priv->label_user_number), "");
	if(user_number > 0 && op_number >= 0) {
		buf = g_strdup_printf("(%d/%d)", op_number, user_number);
		gtk_label_set(GTK_LABEL(priv->label_user_number), buf);
		g_free(buf);
	}

	context_id = gtk_statusbar_get_context_id(GTK_STATUSBAR(priv->statusbar), "Topic");
	gtk_statusbar_pop(GTK_STATUSBAR(priv->statusbar), context_id);

	if(topic)
		gtk_statusbar_push(GTK_STATUSBAR(priv->statusbar), context_id, topic);

}
GtkWidget*
loqui_app_new (void)
{
	LoquiApp *app;
	LoquiAppPrivate *priv;

	GtkWidget *channel_tree;
	GtkWidget *nick_list;

	GtkWidget *vbox;
	GtkWidget *hpaned;
	GtkWidget *vpaned;
	GtkWidget *scrolled_win;

	GtkWidget *hsep;

	app = g_object_new(loqui_app_get_type(), NULL);
	priv = app->priv;

	g_signal_connect(G_OBJECT(app), "delete_event",
			 G_CALLBACK(loqui_app_delete_event_cb), NULL);

	gtk_window_set_policy(GTK_WINDOW (app), TRUE, TRUE, TRUE);

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(app), vbox);

	app->menu = loqui_menu_new(GTK_WINDOW(app));
	gtk_box_pack_start(GTK_BOX(vbox), loqui_menu_get_widget(app->menu),
			   FALSE, FALSE, 0);

	app->toolbar = loqui_toolbar_new(app);
	gtk_box_pack_start(GTK_BOX(vbox), app->toolbar, FALSE, FALSE, 0);

#define SET_SCROLLED_WINDOW(s, w, vpolicy, hpolicy) \
{ \
	s = gtk_scrolled_window_new(NULL, NULL); \
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(s), vpolicy, hpolicy); \
	gtk_container_add(GTK_CONTAINER(s), w); \
}

	hpaned = gtk_hpaned_new();
	gtk_box_pack_start_defaults(GTK_BOX(vbox), hpaned);

	priv->statusbar = gtk_statusbar_new();
	gtk_statusbar_set_has_resize_grip(GTK_STATUSBAR(priv->statusbar), FALSE);
	gtk_label_set_selectable(GTK_LABEL(GTK_STATUSBAR(priv->statusbar)->label), TRUE);
	gtk_box_pack_start(GTK_BOX(vbox), priv->statusbar, FALSE, FALSE, 1);

	priv->label_channel = gtk_label_new("");
	gtk_box_pack_start(GTK_BOX(priv->statusbar), priv->label_channel, FALSE, FALSE, 0);

	priv->label_channel_mode = gtk_label_new("");
	gtk_box_pack_start(GTK_BOX(priv->statusbar), priv->label_channel_mode, FALSE, FALSE, 0);

	priv->label_user_number = gtk_label_new("");
	gtk_box_pack_start(GTK_BOX(priv->statusbar), priv->label_user_number, FALSE, FALSE, 0);

	hsep = gtk_vseparator_new();
	gtk_box_pack_start(GTK_BOX(priv->statusbar), hsep, FALSE, FALSE, 2);

	priv->label_account = gtk_label_new("");
	gtk_box_pack_start(GTK_BOX(priv->statusbar), priv->label_account, FALSE, FALSE, 0);

	/* left side */
	vpaned = gtk_vpaned_new();
	gtk_paned_pack1(GTK_PANED(hpaned), vpaned, TRUE, TRUE);
	
	vbox = gtk_vbox_new(FALSE, 0);
	gtk_paned_pack1(GTK_PANED(vpaned), vbox, TRUE, TRUE);

	app->channel_textview = gtk_text_view_new();
	gtk_text_view_set_editable(GTK_TEXT_VIEW(app->channel_textview), FALSE);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(app->channel_textview), GTK_WRAP_CHAR);
	SET_SCROLLED_WINDOW(scrolled_win, app->channel_textview, 
			    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start_defaults(GTK_BOX(vbox), scrolled_win);

	/* TODO: this should be replaced with a widget considered multiline editing */
	priv->entry = gtk_entry_new();
	gtk_box_pack_end(GTK_BOX(vbox), priv->entry, FALSE, FALSE, 0);
	g_signal_connect(G_OBJECT(priv->entry), "activate",
                         G_CALLBACK(loqui_app_entry_activate_cb), NULL);

	priv->common_textview = gtk_text_view_new();
	gtk_text_view_set_editable(GTK_TEXT_VIEW(priv->common_textview), FALSE);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(priv->common_textview), GTK_WRAP_CHAR);
	SET_SCROLLED_WINDOW(scrolled_win, priv->common_textview, 
			    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_paned_pack2(GTK_PANED(vpaned), scrolled_win, FALSE, TRUE);

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
	app->nick_list = NICK_LIST(nick_list);

	loqui_app_set_current_info(app, NULL, NULL, NULL, NULL, -1, -1);
	loqui_app_restore_size(app);

#undef SET_SCROLLED_WINDOW

	loqui_app_set_focus(app);

	return GTK_WIDGET(app);
}
void loqui_app_set_focus(LoquiApp *app)
{
        g_return_if_fail(app != NULL);
        g_return_if_fail(LOQUI_IS_APP(app));

	gtk_widget_grab_focus(app->priv->entry);
}

void loqui_app_set_channel_buffer(LoquiApp *app, GtkTextBuffer *textbuf)
{
	LoquiAppPrivate *priv;
	GtkTextBuffer *old_buf;

        g_return_if_fail(app != NULL);
        g_return_if_fail(LOQUI_IS_APP(app));

	priv = app->priv;

	if(priv->channel_buffer_inserted_signal_id > 0) {
		old_buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app->channel_textview));
		g_signal_handler_disconnect(old_buf, priv->channel_buffer_inserted_signal_id);
	}

	gtk_text_view_set_buffer(GTK_TEXT_VIEW(app->channel_textview), textbuf);
	priv->channel_buffer_inserted_signal_id = g_signal_connect(G_OBJECT(textbuf), "insert-text",
								  G_CALLBACK(loqui_app_text_buffer_inserted_cb),
								  app->channel_textview);
	gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(app->channel_textview),
					   gtk_text_buffer_get_mark(textbuf, "end"));
}
void loqui_app_set_common_buffer(LoquiApp *app, GtkTextBuffer *textbuf)
{
	LoquiAppPrivate *priv;
	GtkTextBuffer *old_buf;

        g_return_if_fail(app != NULL);
        g_return_if_fail(LOQUI_IS_APP(app));

	priv = app->priv;

	if(priv->common_buffer_inserted_signal_id > 0) {
		old_buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(priv->common_textview));
		g_signal_handler_disconnect(old_buf, priv->common_buffer_inserted_signal_id);
	}

	gtk_text_view_set_buffer(GTK_TEXT_VIEW(priv->common_textview), textbuf);
	priv->common_buffer_inserted_signal_id = g_signal_connect(G_OBJECT(textbuf), "insert-text",
								  G_CALLBACK(loqui_app_text_buffer_inserted_cb),
								  priv->common_textview);
	gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(priv->common_textview),
					   gtk_text_buffer_get_mark(textbuf, "end"));
}
