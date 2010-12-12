/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui -- Chat client for Gtk2 <http://launchpad.net/loqui/>
 * Copyright (C) 2004 Yoichi Imai <sunnyone41@gmail.com>
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

#include "loqui_dropdown_box.h"
#include "gtkutils.h"

enum {
        LAST_SIGNAL
};

enum {
        LAST_PROP
};

struct _LoquiDropdownBoxPrivate
{
	GtkWidget *arrow;
};

static GtkHBoxClass *parent_class = NULL;

/* static guint loqui_dropdown_box_signals[LAST_SIGNAL] = { 0 }; */

static void loqui_dropdown_box_class_init(LoquiDropdownBoxClass *klass);
static void loqui_dropdown_box_init(LoquiDropdownBox *dbox);
static void loqui_dropdown_box_finalize(GObject *object);
static void loqui_dropdown_box_dispose(GObject *object);

static void loqui_dropdown_box_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec);
static void loqui_dropdown_box_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec);

static void loqui_dropdown_box_destroy(GtkObject *object);

static void loqui_dropdown_box_menu_deactivated_cb(GtkWidget *widget, LoquiDropdownBox *dropdown_box);
static void loqui_dropdown_box_detacher(GtkWidget *widget, GtkMenu *menu);

GType
loqui_dropdown_box_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(LoquiDropdownBoxClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) loqui_dropdown_box_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(LoquiDropdownBox),
				0,              /* n_preallocs */
				(GInstanceInitFunc) loqui_dropdown_box_init
			};
		
		type = g_type_register_static(GTK_TYPE_HBOX,
					      "LoquiDropdownBox",
					      &our_info,
					      0);
	}
	
	return type;
}
static void 
loqui_dropdown_box_finalize(GObject *object)
{
	LoquiDropdownBox *dbox;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_DROPDOWN_BOX(object));

        dbox = LOQUI_DROPDOWN_BOX(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize)(object);

	g_free(dbox->priv);
}
static void 
loqui_dropdown_box_dispose(GObject *object)
{
	LoquiDropdownBox *dbox;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_DROPDOWN_BOX(object));

        dbox = LOQUI_DROPDOWN_BOX(object);

        if (G_OBJECT_CLASS(parent_class)->dispose)
                (* G_OBJECT_CLASS(parent_class)->dispose)(object);
}
static void
loqui_dropdown_box_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec)
{
        LoquiDropdownBox *dbox;        

        dbox = LOQUI_DROPDOWN_BOX(object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}
static void
loqui_dropdown_box_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec)
{
        LoquiDropdownBox *dbox;        

        dbox = LOQUI_DROPDOWN_BOX(object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}

static void
loqui_dropdown_box_class_init(LoquiDropdownBoxClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = loqui_dropdown_box_finalize;
        object_class->dispose = loqui_dropdown_box_dispose;
        object_class->get_property = loqui_dropdown_box_get_property;
        object_class->set_property = loqui_dropdown_box_set_property;
	GTK_OBJECT_CLASS(klass)->destroy = loqui_dropdown_box_destroy;
}
static void 
loqui_dropdown_box_init(LoquiDropdownBox *dbox)
{
	LoquiDropdownBoxPrivate *priv;

	priv = g_new0(LoquiDropdownBoxPrivate, 1);

	dbox->priv = priv;
}
static void 
loqui_dropdown_box_destroy(GtkObject *object)
{
        LoquiDropdownBox *dbox;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_DROPDOWN_BOX(object));

        dbox = LOQUI_DROPDOWN_BOX(object);

        if (GTK_OBJECT_CLASS(parent_class)->destroy)
                (* GTK_OBJECT_CLASS(parent_class)->destroy)(object);
}
static gboolean
loqui_dropdown_box_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
        LoquiDropdownBox *dropdown_box;
	LoquiDropdownBoxPrivate *priv;

	g_return_val_if_fail(data != NULL, TRUE);
	g_return_val_if_fail(LOQUI_IS_DROPDOWN_BOX(data), TRUE);

	dropdown_box = LOQUI_DROPDOWN_BOX(data);
	priv = dropdown_box->priv;

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dropdown_box->drop_button), TRUE);

	if (dropdown_box->menu)
		gtk_menu_popup(GTK_MENU(dropdown_box->menu), NULL, NULL,
			       gtkutils_menu_position_under_widget, dropdown_box,
			       event ? event->button : 0,
			       event ? event->time : gtk_get_current_event_time());

	return TRUE;
}
static void
loqui_dropdown_box_detacher(GtkWidget *widget, GtkMenu *menu)
{
        LoquiDropdownBox *dropdown_box;

        g_return_if_fail(widget != NULL);
        g_return_if_fail(LOQUI_IS_DROPDOWN_BOX(widget));

        dropdown_box = LOQUI_DROPDOWN_BOX(widget);

	if (dropdown_box->menu)
		g_signal_handlers_disconnect_by_func(dropdown_box->menu,
						     loqui_dropdown_box_menu_deactivated_cb,
						     dropdown_box);

	dropdown_box->menu = NULL;
}
static void
loqui_dropdown_box_menu_deactivated_cb(GtkWidget *widget, LoquiDropdownBox *dropdown_box)
{
	g_return_if_fail(dropdown_box != NULL);
	g_return_if_fail(LOQUI_IS_DROPDOWN_BOX(dropdown_box));
	
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dropdown_box->drop_button), FALSE);
}
GtkWidget *
loqui_dropdown_box_new(GtkWidget *drop_widget)
{
        LoquiDropdownBox *dbox;
	LoquiDropdownBoxPrivate *priv;

	dbox = g_object_new(loqui_dropdown_box_get_type(), NULL);
	
        priv = dbox->priv;

	dbox->drop_button = gtk_toggle_button_new();
	g_signal_connect(dbox->drop_button, "button-press-event",
			 G_CALLBACK(loqui_dropdown_box_button_press_event), dbox);
	/* gtk_button_set_relief(GTK_BUTTON(dbox->drop_button), GTK_RELIEF_NONE); */
	gtk_box_pack_end(GTK_BOX(dbox), dbox->drop_button, FALSE, FALSE, 0);

	if (drop_widget)
		dbox->drop_widget = drop_widget;
	else
		dbox->drop_widget = gtk_arrow_new(GTK_ARROW_DOWN, GTK_SHADOW_OUT);

	gtk_container_add(GTK_CONTAINER(dbox->drop_button), dbox->drop_widget);

        return GTK_WIDGET(dbox);
}
void
loqui_dropdown_box_set_menu(LoquiDropdownBox *dropdown_box, GtkWidget *menu)
{
	LoquiDropdownBoxPrivate *priv;

	g_return_if_fail(dropdown_box != NULL);
	g_return_if_fail(LOQUI_IS_DROPDOWN_BOX(dropdown_box));
	g_return_if_fail(GTK_IS_MENU(menu));

	priv = dropdown_box->priv;

	if (dropdown_box->menu != menu) {
		dropdown_box->menu = menu;
		g_signal_connect(menu, "deactivate",
				 G_CALLBACK(loqui_dropdown_box_menu_deactivated_cb), dropdown_box);
		gtk_menu_attach_to_widget(GTK_MENU(menu),
					  GTK_WIDGET(dropdown_box),
					  loqui_dropdown_box_detacher);
	}
}
GtkWidget *
loqui_dropdown_box_get_menu(LoquiDropdownBox *dropdown_box)
{
	LoquiDropdownBoxPrivate *priv;

	g_return_val_if_fail(dropdown_box != NULL, NULL);
	g_return_val_if_fail(LOQUI_IS_DROPDOWN_BOX(dropdown_box), NULL);

	priv = dropdown_box->priv;

	return dropdown_box->menu;
}
void
loqui_dropdown_box_remove_menu(LoquiDropdownBox *dropdown_box)
{
	LoquiDropdownBoxPrivate *priv;

	g_return_if_fail(dropdown_box != NULL);
	g_return_if_fail(LOQUI_IS_DROPDOWN_BOX(dropdown_box));

	priv = dropdown_box->priv;
	if (dropdown_box->menu) {
		gtk_menu_detach(GTK_MENU(dropdown_box->menu));
	}
}
