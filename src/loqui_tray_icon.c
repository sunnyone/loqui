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

#include "loqui_tray_icon.h"
#include "loqui_stock.h"
#include "gtkutils.h"

enum {
        LAST_SIGNAL
};

enum {
        LAST_PROP
};

struct _LoquiTrayIconPrivate
{
	GtkMenu *menu;
};

static EggTrayIconClass *parent_class = NULL;

/* static guint loqui_tray_icon_signals[LAST_SIGNAL] = { 0 }; */

static void loqui_tray_icon_class_init(LoquiTrayIconClass *klass);
static void loqui_tray_icon_init(LoquiTrayIcon *tray_icon);
static void loqui_tray_icon_finalize(GObject *object);
static void loqui_tray_icon_dispose(GObject *object);

static void loqui_tray_icon_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec);
static void loqui_tray_icon_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec);

static void loqui_tray_icon_destroy(GtkObject *object);
GType
loqui_tray_icon_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(LoquiTrayIconClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) loqui_tray_icon_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(LoquiTrayIcon),
				0,              /* n_preallocs */
				(GInstanceInitFunc) loqui_tray_icon_init
			};
		
		type = g_type_register_static(EGG_TYPE_TRAY_ICON,
					      "LoquiTrayIcon",
					      &our_info,
					      0);
	}
	
	return type;
}
static void 
loqui_tray_icon_finalize(GObject *object)
{
	LoquiTrayIcon *tray_icon;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_TRAY_ICON(object));

        tray_icon = LOQUI_TRAY_ICON(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize)(object);

	g_free(tray_icon->priv);
}
static void 
loqui_tray_icon_dispose(GObject *object)
{
	LoquiTrayIcon *tray_icon;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_TRAY_ICON(object));

        tray_icon = LOQUI_TRAY_ICON(object);

        if (G_OBJECT_CLASS(parent_class)->dispose)
                (* G_OBJECT_CLASS(parent_class)->dispose)(object);
}
static void
loqui_tray_icon_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec)
{
        LoquiTrayIcon *tray_icon;        

        tray_icon = LOQUI_TRAY_ICON(object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}
static void
loqui_tray_icon_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec)
{
        LoquiTrayIcon *tray_icon;        

        tray_icon = LOQUI_TRAY_ICON(object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}

static void
loqui_tray_icon_class_init(LoquiTrayIconClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = loqui_tray_icon_finalize;
        object_class->dispose = loqui_tray_icon_dispose;
        object_class->get_property = loqui_tray_icon_get_property;
        object_class->set_property = loqui_tray_icon_set_property;
        GTK_OBJECT_CLASS(klass)->destroy = loqui_tray_icon_destroy;
}
static void 
loqui_tray_icon_init(LoquiTrayIcon *tray_icon)
{
	LoquiTrayIconPrivate *priv;

	priv = g_new0(LoquiTrayIconPrivate, 1);

	tray_icon->priv = priv;
}
static void 
loqui_tray_icon_destroy(GtkObject *object)
{
        LoquiTrayIcon *tray_icon;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_TRAY_ICON(object));

        tray_icon = LOQUI_TRAY_ICON(object);

        if (GTK_OBJECT_CLASS(parent_class)->destroy)
                (* GTK_OBJECT_CLASS(parent_class)->destroy)(object);
}
static gboolean
loqui_tray_icon_button_press_event_cb(GtkWidget *widget, GdkEventButton *event, LoquiTrayIcon *tray_icon)
{
	LoquiTrayIconPrivate *priv;

        g_return_val_if_fail(tray_icon != NULL, FALSE);
        g_return_val_if_fail(LOQUI_IS_TRAY_ICON(tray_icon), FALSE);

        priv = tray_icon->priv;

	if (event->type == GDK_BUTTON_PRESS) {
		switch (event->button) {
		case 1:
			if (gtkutils_widget_is_iconified(GTK_WIDGET(tray_icon->app)) ||
			    loqui_app_is_obscured(tray_icon->app))
				gtk_window_present(GTK_WINDOW(tray_icon->app));
			else
				gtk_window_iconify(GTK_WINDOW(tray_icon->app));
			break;
		case 3:
			gtk_menu_popup(GTK_MENU(priv->menu), NULL, NULL,
				       gtkutils_menu_position_under_widget, GTK_WIDGET(tray_icon),
				       event ? event->button : 0,
				       event ? event->time : gtk_get_current_event_time());
			break;
		default:
			break;
		}
	}
	return FALSE;
}
GtkWidget *
loqui_tray_icon_new(LoquiApp *app, GtkMenu *menu)
{
        LoquiTrayIcon *tray_icon;
	LoquiTrayIconPrivate *priv;

	GtkWidget *event_box;

	tray_icon = g_object_new(loqui_tray_icon_get_type(), "title", "Loqui", NULL);
	
        priv = tray_icon->priv;
	tray_icon->app = app;
	priv->menu = menu;

	event_box = gtk_event_box_new();
	gtk_container_add(GTK_CONTAINER(tray_icon), event_box);
	
	g_signal_connect(G_OBJECT(event_box), "button_press_event",
			 G_CALLBACK(loqui_tray_icon_button_press_event_cb), tray_icon);

	tray_icon->image = gtk_image_new_from_stock(LOQUI_STOCK_LOQUI, GTK_ICON_SIZE_LARGE_TOOLBAR);
	gtk_container_add(GTK_CONTAINER(event_box), tray_icon->image);

	loqui_tray_icon_set_hilighted(tray_icon, FALSE);
	gtk_widget_show_all(GTK_WIDGET(tray_icon));
	
        return GTK_WIDGET(tray_icon);
}
void
loqui_tray_icon_set_hilighted(LoquiTrayIcon *tray_icon, gboolean is_hilighted)
{
	LoquiTrayIconPrivate *priv;

        g_return_if_fail(tray_icon != NULL);
        g_return_if_fail(LOQUI_IS_TRAY_ICON(tray_icon));

	priv = tray_icon->priv;

	if (tray_icon->is_hilighted == is_hilighted)
		return;

	tray_icon->is_hilighted = is_hilighted;

	gtk_image_set_from_stock(GTK_IMAGE(tray_icon->image),
				 is_hilighted ? LOQUI_STOCK_LOQUI_HILIGHTED : LOQUI_STOCK_LOQUI,
				 GTK_ICON_SIZE_LARGE_TOOLBAR);
}
