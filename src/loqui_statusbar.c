/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui -- IRC client for Gtk2 <http://loqui.good-day.net/>
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

#include "loqui_statusbar.h"
#include "intl.h"
#include "icons/pixbufs.h"
#include "gtkutils.h"

#include <string.h>

#define STATUSBAR_ICON_SIZE 14
#define STRING_DISCONNECTED _("(disconnected)")
#define STRING_UNSELECTED _("(unselected)")

typedef enum {
	AWAY_STATE_NONE,
	AWAY_STATE_ONLINE,
	AWAY_STATE_AWAY,
	AWAY_STATE_BUSY,
	AWAY_STATE_OFFLINE
} AwayState;

enum {
	NICK_CHANGE,
	NICK_SELECTED,
        LAST_SIGNAL
};

struct _LoquiStatusbarPrivate
{
	GtkWidget *label_account;
	
	GtkWidget *image_online;
	GtkWidget *image_offline;
	GtkWidget *image_away;
	GtkWidget *image_busy;
		
	GtkWidget *button_away;
	GtkWidget *button_nick;
	GtkWidget *label_nick;
	GtkWidget *toggle_preset;
	GtkWidget *progress_lag;
	GtkWidget *toggle_scroll;
	
	GtkWidget *menu_preset;
};

static GtkStatusbarClass *parent_class = NULL;
#define PARENT_TYPE GTK_TYPE_STATUSBAR

static guint loqui_statusbar_signals[LAST_SIGNAL] = { 0 };

static void loqui_statusbar_class_init(LoquiStatusbarClass *klass);
static void loqui_statusbar_init(LoquiStatusbar *statusbar);
static void loqui_statusbar_finalize(GObject *object);
static void loqui_statusbar_destroy(GtkObject *object);

static void loqui_statusbar_set_away_status(LoquiStatusbar *statusbar, AwayState away_state);
static void loqui_statusbar_preset_menu_deactivated_cb(GtkWidget *widget, LoquiStatusbar *statusbar);
static void loqui_statusbar_preset_button_press_event_cb(GtkWidget *widget, GdkEventButton *event, LoquiStatusbar *statusbar);
static void loqui_statusbar_preset_menuitem_activated_cb(GtkWidget *widget, LoquiStatusbar *statusbar);
static void loqui_statusbar_set_preset(LoquiStatusbar *statusbar, GList *nick_list);
static void loqui_statusbar_nick_button_clicked_cb(GtkWidget *widget, LoquiStatusbar *statusbar);

GType
loqui_statusbar_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(LoquiStatusbarClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) loqui_statusbar_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(LoquiStatusbar),
				0,              /* n_preallocs */
				(GInstanceInitFunc) loqui_statusbar_init
			};
		
		type = g_type_register_static(PARENT_TYPE,
					      "LoquiStatusbar",
					      &our_info,
					      0);
	}
	
	return type;
}
static void
loqui_statusbar_class_init (LoquiStatusbarClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);
        GtkObjectClass *gtk_object_class = GTK_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);


        loqui_statusbar_signals[NICK_CHANGE] = g_signal_new("nick-change",
							    G_OBJECT_CLASS_TYPE(object_class),
							    G_SIGNAL_RUN_FIRST,
							    0,
						            NULL, NULL,
							    g_cclosure_marshal_VOID__VOID,
							    G_TYPE_NONE, 0);
        loqui_statusbar_signals[NICK_SELECTED] = g_signal_new("nick-selected",
						  	      G_OBJECT_CLASS_TYPE(object_class),
						   	      G_SIGNAL_RUN_FIRST,
						   	      0,
						              NULL, NULL,
						   	      g_cclosure_marshal_VOID__STRING,
						              G_TYPE_NONE, 1,
						   	      G_TYPE_STRING);

	object_class->finalize = loqui_statusbar_finalize;
        gtk_object_class->destroy = loqui_statusbar_destroy;
}
static void 
loqui_statusbar_init (LoquiStatusbar *statusbar)
{
	LoquiStatusbarPrivate *priv;

	priv = g_new0(LoquiStatusbarPrivate, 1);

	statusbar->priv = priv;
}
static void 
loqui_statusbar_finalize (GObject *object)
{
	LoquiStatusbar *statusbar;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_STATUSBAR(object));

        statusbar = LOQUI_STATUSBAR(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize) (object);

	g_free(statusbar->priv);
}
static void 
loqui_statusbar_destroy (GtkObject *object)
{
        LoquiStatusbar *statusbar;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_STATUSBAR(object));

        statusbar = LOQUI_STATUSBAR(object);

        if (GTK_OBJECT_CLASS(parent_class)->destroy)
                (* GTK_OBJECT_CLASS(parent_class)->destroy) (object);
}
static void
loqui_statusbar_set_away_status(LoquiStatusbar *statusbar, AwayState away_state)
{
	LoquiStatusbarPrivate *priv;
        g_return_if_fail(statusbar != NULL);
        g_return_if_fail(LOQUI_IS_STATUSBAR(statusbar));
        GtkWidget *old_image;
        
        priv = statusbar->priv;	
	
	old_image = gtk_bin_get_child(GTK_BIN(priv->button_away));
	if (old_image) {
		g_object_ref(old_image);
		gtk_container_remove(GTK_CONTAINER(priv->button_away), old_image);
	}
	
	switch (away_state) {
	case AWAY_STATE_ONLINE:
		gtk_container_add(GTK_CONTAINER(priv->button_away), priv->image_online);
		break;
	case AWAY_STATE_AWAY:
		gtk_container_add(GTK_CONTAINER(priv->button_away), priv->image_away);
		break;
	case AWAY_STATE_BUSY:
		gtk_container_add(GTK_CONTAINER(priv->button_away), priv->image_busy);
		break;		
	case AWAY_STATE_OFFLINE:
		gtk_container_add(GTK_CONTAINER(priv->button_away), priv->image_offline);
		break;
	default:
		break;
	}
	
}
static void
loqui_statusbar_preset_menu_deactivated_cb(GtkWidget *widget, LoquiStatusbar *statusbar)
{
	LoquiStatusbarPrivate *priv;
        g_return_if_fail(statusbar != NULL);
        g_return_if_fail(LOQUI_IS_STATUSBAR(statusbar));
        
        priv = statusbar->priv;	
	
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->toggle_preset), FALSE);
}
static void
loqui_statusbar_preset_button_press_event_cb(GtkWidget *widget, GdkEventButton *event, LoquiStatusbar *statusbar)
{
	LoquiStatusbarPrivate *priv;
        g_return_if_fail(statusbar != NULL);
        g_return_if_fail(LOQUI_IS_STATUSBAR(statusbar));
        
        priv = statusbar->priv;	
        
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->toggle_preset), TRUE);
	gtk_menu_popup(GTK_MENU(priv->menu_preset), NULL, NULL,
		       gtkutils_menu_position_under_widget, priv->toggle_preset,
		       event ? event->button : 0,
		       event ? event->time : gtk_get_current_event_time());
}
static void
loqui_statusbar_preset_menuitem_activated_cb(GtkWidget *widget, LoquiStatusbar *statusbar)
{
	LoquiStatusbarPrivate *priv;
	const gchar *str;

	g_return_if_fail(GTK_IS_MENU_ITEM(widget));
        g_return_if_fail(statusbar != NULL);
        g_return_if_fail(LOQUI_IS_STATUSBAR(statusbar));

        priv = statusbar->priv;		

	str = g_object_get_data(G_OBJECT(widget), "nick");
	g_signal_emit(statusbar, loqui_statusbar_signals[NICK_SELECTED], 0, str);
}
static void
loqui_statusbar_set_preset(LoquiStatusbar *statusbar, GList *nick_list)
{
	LoquiStatusbarPrivate *priv;
	GtkWidget *menuitem;
	GList *cur, *tmp_list = NULL;

        g_return_if_fail(statusbar != NULL);
        g_return_if_fail(LOQUI_IS_STATUSBAR(statusbar));
        
        priv = statusbar->priv;	
     
	for (cur = GTK_MENU_SHELL(priv->menu_preset)->children; cur != NULL; cur = cur->next) {
		tmp_list = g_list_append(tmp_list, cur->data);
	}
	for (cur = tmp_list; cur != NULL; cur = cur->next) {
		gtk_container_remove(GTK_CONTAINER(priv->menu_preset), cur->data);
	}
	for (cur = nick_list; cur != NULL; cur = cur->next) {
		if (!cur->data)
			continue;
		
		menuitem = gtk_menu_item_new_with_label(cur->data);
		g_object_set_data_full(G_OBJECT(menuitem), "nick", g_strdup(cur->data), (GDestroyNotify) g_free);
		g_signal_connect(G_OBJECT(menuitem), "activate",
				 G_CALLBACK(loqui_statusbar_preset_menuitem_activated_cb), statusbar);
		gtk_widget_show(menuitem);
		gtk_menu_shell_append(GTK_MENU_SHELL(priv->menu_preset), menuitem);
	}
}
static void
loqui_statusbar_nick_button_clicked_cb(GtkWidget *widget, LoquiStatusbar *statusbar)
{
	LoquiStatusbarPrivate *priv;

        g_return_if_fail(statusbar != NULL);
        g_return_if_fail(LOQUI_IS_STATUSBAR(statusbar));

	g_signal_emit(statusbar, loqui_statusbar_signals[NICK_CHANGE], 0);
}
static GtkWidget*
loqui_statusbar_create_icon_image(const guint8 *data)
{
	GtkWidget *image;
	GdkPixbuf *pixbuf, *pixbuf_scaled;
	
	pixbuf = gdk_pixbuf_new_from_inline(-1, data, FALSE, NULL);
	if(pixbuf == NULL)
		g_error("Can't get pixbuf");
	pixbuf_scaled = gdk_pixbuf_scale_simple(pixbuf, STATUSBAR_ICON_SIZE, STATUSBAR_ICON_SIZE, GDK_INTERP_BILINEAR);
	if(pixbuf_scaled == NULL)
		g_error("Can't scale pixbuf");
	image = gtk_image_new_from_pixbuf(pixbuf_scaled);
	gtk_widget_show(image);
	g_object_unref(pixbuf);
	g_object_unref(pixbuf_scaled);
	
	return image;
}

GtkWidget*
loqui_statusbar_new (void)
{
        LoquiStatusbar *statusbar;
	LoquiStatusbarPrivate *priv;
	GtkWidget *hsep;
	GtkWidget *arrow;
	
	statusbar = g_object_new(loqui_statusbar_get_type(), NULL);
	priv = statusbar->priv;
	
	gtk_statusbar_set_has_resize_grip(GTK_STATUSBAR(statusbar), FALSE);
	gtk_label_set_selectable(GTK_LABEL(GTK_STATUSBAR(statusbar)->label), TRUE);
	
	priv->image_offline = loqui_statusbar_create_icon_image(offline_pixbuf);
	priv->image_online = loqui_statusbar_create_icon_image(online_pixbuf);
	priv->image_away = loqui_statusbar_create_icon_image(away_pixbuf);
	priv->image_busy = loqui_statusbar_create_icon_image(busy_pixbuf);

/* FIXME: why statusbar becomes taller when button widget is on it? */
#define WIDGET_MINIMIZE_HEIGHT(widget) gtk_widget_set_usize(widget, -1, 1);

	priv->button_away = gtk_button_new();
	WIDGET_MINIMIZE_HEIGHT(priv->button_away);
	gtk_box_pack_start(GTK_BOX(statusbar), priv->button_away, FALSE, FALSE, 0);
	gtk_button_set_relief(GTK_BUTTON(priv->button_away), GTK_RELIEF_NONE);	

	priv->button_nick = gtk_button_new();
	WIDGET_MINIMIZE_HEIGHT(priv->button_nick);
	g_signal_connect(G_OBJECT(priv->button_nick), "clicked",
			 G_CALLBACK(loqui_statusbar_nick_button_clicked_cb), statusbar);
	gtk_box_pack_start(GTK_BOX(statusbar), priv->button_nick, FALSE, FALSE, 0);
	gtk_button_set_relief(GTK_BUTTON(priv->button_nick), GTK_RELIEF_NONE);
	
	priv->label_nick = gtk_label_new("");
	gtk_container_add(GTK_CONTAINER(priv->button_nick), priv->label_nick);

	priv->toggle_preset = gtk_toggle_button_new();
	
	gtk_box_pack_start(GTK_BOX(statusbar), priv->toggle_preset, FALSE, FALSE, 0);
	gtk_button_set_relief(GTK_BUTTON(priv->toggle_preset), GTK_RELIEF_NONE);
	WIDGET_MINIMIZE_HEIGHT(priv->toggle_preset);
	arrow = gtk_arrow_new(GTK_ARROW_DOWN, GTK_SHADOW_OUT);
	gtk_container_add(GTK_CONTAINER(priv->toggle_preset), arrow);
	g_signal_connect(G_OBJECT(priv->toggle_preset), "button_press_event",
			 G_CALLBACK(loqui_statusbar_preset_button_press_event_cb), statusbar);

	hsep = gtk_vseparator_new();
	gtk_box_pack_start(GTK_BOX(statusbar), hsep, FALSE, FALSE, 2);

	priv->label_account = gtk_label_new("");
	gtk_box_pack_start(GTK_BOX(statusbar), priv->label_account, FALSE, FALSE, 0);

	priv->progress_lag = gtk_progress_bar_new();
	gtk_box_pack_start(GTK_BOX(statusbar), priv->progress_lag, FALSE, FALSE, 0);
	gtk_widget_set_size_request(priv->progress_lag, 70, -1);

	priv->toggle_scroll = gtk_toggle_button_new_with_mnemonic("Scroll");
	WIDGET_MINIMIZE_HEIGHT(priv->toggle_scroll);
	gtk_box_pack_start(GTK_BOX(statusbar), priv->toggle_scroll, FALSE, FALSE, 0);
	
	priv->menu_preset = gtk_menu_new();
	g_signal_connect(priv->menu_preset, "deactivate",
			 G_CALLBACK(loqui_statusbar_preset_menu_deactivated_cb), statusbar);
	
	return GTK_WIDGET(statusbar);
}

void
loqui_statusbar_set_current_channel(LoquiStatusbar *statusbar, Channel *channel)
{
	LoquiStatusbarPrivate *priv;
	
        g_return_if_fail(statusbar != NULL);
        g_return_if_fail(LOQUI_IS_STATUSBAR(statusbar));
    
    	priv = statusbar->priv;
}
void
loqui_statusbar_set_current_account(LoquiStatusbar *statusbar, Account *account)
{
	LoquiStatusbarPrivate *priv;
	AwayState away_state;
        g_return_if_fail(statusbar != NULL);
        g_return_if_fail(LOQUI_IS_STATUSBAR(statusbar));
        
        priv = statusbar->priv;
        
        // set account label
        gtk_label_set(GTK_LABEL(priv->label_account), account ? account_get_name(account) : STRING_UNSELECTED);

	// set nick        
        if (account == NULL) {
        	gtk_widget_set_sensitive(priv->button_nick, FALSE);
//        	gtk_widget_set_sensitive(priv->toggle_preset, FALSE);
        	gtk_label_set(GTK_LABEL(priv->label_nick), STRING_UNSELECTED);
        } else if (!account_is_connected(account)) {
 	      	gtk_widget_set_sensitive(priv->button_nick, FALSE);
 //       	gtk_widget_set_sensitive(priv->toggle_preset, FALSE);
        	gtk_label_set(GTK_LABEL(priv->label_nick), STRING_DISCONNECTED);
        } else {
        	gtk_widget_set_sensitive(priv->button_nick, TRUE);
//        	gtk_widget_set_sensitive(priv->toggle_preset, TRUE);
        	gtk_label_set(GTK_LABEL(priv->label_nick), account_get_current_nick(account));
        }
        
        if (account)
        	loqui_statusbar_set_preset(statusbar, account->nick_list);
        
        // set icon
        if (account == NULL) {
        	away_state = AWAY_STATE_OFFLINE;
        } else if (!account_is_connected(account)) {
        	away_state = AWAY_STATE_OFFLINE;
        } else if (account_get_away_status(account)) {
        	away_state = AWAY_STATE_AWAY;
        } else {
        	away_state = AWAY_STATE_ONLINE;
        }
        
        loqui_statusbar_set_away_status(statusbar, away_state);
}
void
loqui_statusbar_set_default(LoquiStatusbar *statusbar, const gchar *str)
{
	guint context_id;

        g_return_if_fail(statusbar != NULL);
        g_return_if_fail(LOQUI_IS_STATUSBAR(statusbar));
        	
	context_id = gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), "Default");
	gtk_statusbar_pop(GTK_STATUSBAR(statusbar), context_id);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), context_id, str);
}
