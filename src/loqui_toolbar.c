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

#include "loqui_toolbar.h"
#include "intl.h"
#include "account_manager.h"

struct _LoquiToolbarPrivate
{
	guint toggle_scroll_hander_id;
};

static GtkToolbarClass *parent_class = NULL;
#define PARENT_TYPE GTK_TYPE_TOOLBAR

static void loqui_toolbar_class_init(LoquiToolbarClass *klass);
static void loqui_toolbar_init(LoquiToolbar *toolbar);
static void loqui_toolbar_finalize(GObject *object);
static void loqui_toolbar_destroy(GtkObject *object);

GType
loqui_toolbar_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(LoquiToolbarClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) loqui_toolbar_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(LoquiToolbar),
				0,              /* n_preallocs */
				(GInstanceInitFunc) loqui_toolbar_init
			};
		
		type = g_type_register_static(PARENT_TYPE,
					      "LoquiToolbar",
					      &our_info,
					      0);
	}
	
	return type;
}
static void
loqui_toolbar_class_init (LoquiToolbarClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);
        GtkObjectClass *gtk_object_class = GTK_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = loqui_toolbar_finalize;
        gtk_object_class->destroy = loqui_toolbar_destroy;
}
static void 
loqui_toolbar_init (LoquiToolbar *toolbar)
{
	LoquiToolbarPrivate *priv;

	priv = g_new0(LoquiToolbarPrivate, 1);

	toolbar->priv = priv;
}
static void 
loqui_toolbar_finalize (GObject *object)
{
	LoquiToolbar *toolbar;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_TOOLBAR(object));

        toolbar = LOQUI_TOOLBAR(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize) (object);

	g_free(toolbar->priv);
}
static void 
loqui_toolbar_destroy (GtkObject *object)
{
        LoquiToolbar *toolbar;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_TOOLBAR(object));

        toolbar = LOQUI_TOOLBAR(object);

        if (GTK_OBJECT_CLASS(parent_class)->destroy)
                (* GTK_OBJECT_CLASS(parent_class)->destroy) (object);
}

static void
loqui_toolbar_toggle_scrolling_cb(GtkWidget *widget, gpointer data)
{
	account_manager_set_whether_scrolling(account_manager_get(), 
					      gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)));
}

GtkWidget*
loqui_toolbar_new(gpointer data)
{
        LoquiToolbar *toolbar;
	LoquiToolbarPrivate *priv;

	toolbar = g_object_new(loqui_toolbar_get_type(), NULL);

	priv = toolbar->priv;
	gtk_toolbar_set_icon_size(GTK_TOOLBAR(toolbar), GTK_ICON_SIZE_SMALL_TOOLBAR);

	gtk_toolbar_insert_stock(GTK_TOOLBAR(toolbar),
                                 GTK_STOCK_JUMP_TO,
				 _("Connect"),
				 NULL,
				 NULL, // G_CALLBACK (toolbar_cb),
				 data,
				 -1);  /* -1 means "append" */
	
	toolbar->toggle_scroll = gtk_toolbar_append_element(GTK_TOOLBAR(toolbar),
							    GTK_TOOLBAR_CHILD_TOGGLEBUTTON,
							    NULL, _("Scroll"),
							    _("Toggle whether scrolling"),
							    NULL, NULL,
							    NULL, NULL);

	priv->toggle_scroll_hander_id = g_signal_connect(G_OBJECT(toolbar->toggle_scroll),
							 "toggled",
							 G_CALLBACK(loqui_toolbar_toggle_scrolling_cb),
							 toolbar);
	return GTK_WIDGET(toolbar);
}
void
loqui_toolbar_set_toggle_scrolling_without_signal_emission(LoquiToolbar *toolbar, gboolean is_scroll)
{
	LoquiToolbarPrivate *priv;

	priv = toolbar->priv;

	g_signal_handler_block(toolbar->toggle_scroll, priv->toggle_scroll_hander_id);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toolbar->toggle_scroll), is_scroll);
	g_signal_handler_unblock(toolbar->toggle_scroll, priv->toggle_scroll_hander_id);
}
