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

#include "loqui_transfer_item.h"

enum {
        LAST_SIGNAL
};

enum {
        LAST_PROP
};

struct _LoquiTransferItemPrivate
{
};

static GObjectClass *parent_class = NULL;

/* static guint loqui_transfer_item_signals[LAST_SIGNAL] = { 0 }; */

static GObject* loqui_transfer_item_constructor(GType type, guint n_props, GObjectConstructParam *props);

static void loqui_transfer_item_class_init(LoquiTransferItemClass *klass);
static void loqui_transfer_item_init(LoquiTransferItem *transitem);
static void loqui_transfer_item_finalize(GObject *object);
static void loqui_transfer_item_dispose(GObject *object);

static void loqui_transfer_item_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec);
static void loqui_transfer_item_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec);

GType
loqui_transfer_item_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(LoquiTransferItemClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) loqui_transfer_item_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(LoquiTransferItem),
				0,              /* n_preallocs */
				(GInstanceInitFunc) loqui_transfer_item_init
			};
		
		type = g_type_register_static(G_TYPE_OBJECT,
					      "LoquiTransferItem",
					      &our_info,
					      0);
	}
	
	return type;
}
static GObject*
loqui_transfer_item_constructor(GType type, guint n_props, GObjectConstructParam *props)
{
        GObject *object;
        GObjectClass *object_class = G_OBJECT_CLASS(parent_class);

	object = object_class->constructor(type, n_props, props);

	return object;
}
static void 
loqui_transfer_item_finalize(GObject *object)
{
	LoquiTransferItem *transitem;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_TRANSFER_ITEM(object));

        transitem = LOQUI_TRANSFER_ITEM(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize)(object);

	g_free(transitem->priv);
}
static void 
loqui_transfer_item_dispose(GObject *object)
{
	LoquiTransferItem *transitem;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_TRANSFER_ITEM(object));

        transitem = LOQUI_TRANSFER_ITEM(object);

        if (G_OBJECT_CLASS(parent_class)->dispose)
                (* G_OBJECT_CLASS(parent_class)->dispose)(object);
}
static void
loqui_transfer_item_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec)
{
        LoquiTransferItem *transitem;        

        transitem = LOQUI_TRANSFER_ITEM(object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}
static void
loqui_transfer_item_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec)
{
        LoquiTransferItem *transitem;        

        transitem = LOQUI_TRANSFER_ITEM(object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}

static void
loqui_transfer_item_class_init(LoquiTransferItemClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);

	object_class->constructor = loqui_transfer_item_constructor; 
        object_class->finalize = loqui_transfer_item_finalize;
        object_class->dispose = loqui_transfer_item_dispose;
        object_class->get_property = loqui_transfer_item_get_property;
        object_class->set_property = loqui_transfer_item_set_property;
}
static void 
loqui_transfer_item_init(LoquiTransferItem *transitem)
{
	LoquiTransferItemPrivate *priv;

	priv = g_new0(LoquiTransferItemPrivate, 1);

	transitem->priv = priv;
}
LoquiTransferItem*
loqui_transfer_item_new(void)
{
        LoquiTransferItem *transitem;
	LoquiTransferItemPrivate *priv;

	transitem = g_object_new(loqui_transfer_item_get_type(), NULL);
	
        priv = transitem->priv;

        return transitem;
}
