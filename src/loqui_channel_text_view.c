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

#include "loqui_channel_text_view.h"
#include <gdk/gdkkeysyms.h>

enum {
	SIGNAL_NEEDLESS_KEY_PRESS,
        LAST_SIGNAL
};

enum {
        LAST_PROP
};

struct _LoquiChannelTextViewPrivate
{
};

static GtkTextViewClass *parent_class = NULL;

static guint channel_text_view_signals[LAST_SIGNAL] = { 0 };

static void loqui_channel_text_view_class_init(LoquiChannelTextViewClass *klass);
static void loqui_channel_text_view_init(LoquiChannelTextView *view);
static void loqui_channel_text_view_finalize(GObject *object);
static void loqui_channel_text_view_dispose(GObject *object);

static void loqui_channel_text_view_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec);
static void loqui_channel_text_view_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec);

static void loqui_channel_text_view_destroy(GtkObject *object);

static gboolean loqui_channel_text_view_key_press_event(GtkWidget *widget,
							GdkEventKey *event);

GType
loqui_channel_text_view_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(LoquiChannelTextViewClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) loqui_channel_text_view_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(LoquiChannelTextView),
				0,              /* n_preallocs */
				(GInstanceInitFunc) loqui_channel_text_view_init
			};
		
		type = g_type_register_static(GTK_TYPE_TEXT_VIEW,
					      "LoquiChannelTextView",
					      &our_info,
					      0);
	}
	
	return type;
}
static void 
loqui_channel_text_view_finalize(GObject *object)
{
	LoquiChannelTextView *view;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL_TEXT_VIEW(object));

        view = LOQUI_CHANNEL_TEXT_VIEW(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize)(object);

	g_free(view->priv);
}
static void 
loqui_channel_text_view_dispose(GObject *object)
{
	LoquiChannelTextView *view;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL_TEXT_VIEW(object));

        view = LOQUI_CHANNEL_TEXT_VIEW(object);

        if (G_OBJECT_CLASS(parent_class)->dispose)
                (* G_OBJECT_CLASS(parent_class)->dispose)(object);
}
static void
loqui_channel_text_view_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec)
{
        LoquiChannelTextView *view;        

        view = LOQUI_CHANNEL_TEXT_VIEW(object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}
static void
loqui_channel_text_view_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec)
{
        LoquiChannelTextView *view;        

        view = LOQUI_CHANNEL_TEXT_VIEW(object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}

static void
loqui_channel_text_view_class_init(LoquiChannelTextViewClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = loqui_channel_text_view_finalize;
        object_class->dispose = loqui_channel_text_view_dispose;
        object_class->get_property = loqui_channel_text_view_get_property;
        object_class->set_property = loqui_channel_text_view_set_property;
        GTK_OBJECT_CLASS(klass)->destroy = loqui_channel_text_view_destroy;

	widget_class->key_press_event = loqui_channel_text_view_key_press_event;

        channel_text_view_signals[SIGNAL_NEEDLESS_KEY_PRESS] = g_signal_new("needless_key_press",
									    G_OBJECT_CLASS_TYPE(object_class),
									    G_SIGNAL_RUN_FIRST,
									    G_STRUCT_OFFSET(LoquiChannelTextViewClass, needless_key_press),
									    NULL, NULL,
									    g_cclosure_marshal_VOID__VOID,
									    G_TYPE_NONE, 0);
}
static void 
loqui_channel_text_view_init(LoquiChannelTextView *view)
{
	LoquiChannelTextViewPrivate *priv;

	priv = g_new0(LoquiChannelTextViewPrivate, 1);

	view->priv = priv;
}
static void 
loqui_channel_text_view_destroy(GtkObject *object)
{
        LoquiChannelTextView *view;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL_TEXT_VIEW(object));

        view = LOQUI_CHANNEL_TEXT_VIEW(object);

        if (GTK_OBJECT_CLASS(parent_class)->destroy)
                (* GTK_OBJECT_CLASS(parent_class)->destroy)(object);
}
static gboolean
loqui_channel_text_view_key_press_event(GtkWidget *widget,
					GdkEventKey *event)
{
	GtkBindingSet *bset;
	GtkBindingEntry *bentry;

	switch (event->keyval) {
	case GDK_Shift_L:
	case GDK_Shift_R:
	case GDK_Control_L:
	case GDK_Control_R:
	case GDK_Caps_Lock:
	case GDK_Shift_Lock:
	case GDK_Meta_L:
	case GDK_Meta_R:
	case GDK_Alt_L:
	case GDK_Alt_R:
	case GDK_Super_L:
	case GDK_Super_R:
	case GDK_Hyper_L:
	case GDK_Hyper_R: /* FIXME: modifiers, enough? */
		return FALSE;
	default:
		bset = gtk_binding_set_find("GtkTextView");
		bentry = bset->entries;
		while (bentry != NULL) {
			if (event->keyval == bentry->keyval &&
			    event->state == bentry->modifiers)
				return FALSE;

			bentry = bentry->set_next;
		}
		g_signal_emit(widget, channel_text_view_signals[SIGNAL_NEEDLESS_KEY_PRESS], 0);
	}

	if (* GTK_WIDGET_CLASS(parent_class)->key_press_event)
		return (* GTK_WIDGET_CLASS(parent_class)->key_press_event)(widget, event);
	return TRUE;
}
GtkWidget *
loqui_channel_text_view_new(void)
{
        LoquiChannelTextView *view;
	LoquiChannelTextViewPrivate *priv;

	view = g_object_new(loqui_channel_text_view_get_type(),
			    "editable", FALSE,
			    "wrap_mode", GTK_WRAP_CHAR,
			    NULL);
	
        priv = view->priv;

        return GTK_WIDGET(view);
}
