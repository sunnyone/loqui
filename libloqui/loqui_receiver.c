/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * libloqui -- Chat/IM client library for GLib <http://loqui.good-day.net/>
 * Copyright (C) 2004 Yoichi Imai <yoichi@silver-forest.com>
 *
 * This Library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with the Gnome Library; see the file COPYING.LIB.  If not,
 * write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#include "config.h"

#include "loqui_receiver.h"

enum {
        LAST_SIGNAL
};

enum {
        LAST_PROP
};

struct _LoquiReceiverPrivate
{
};

static GObjectClass *parent_class = NULL;

/* static guint loqui_receiver_signals[LAST_SIGNAL] = { 0 }; */

static void loqui_receiver_class_init(LoquiReceiverClass *klass);
static void loqui_receiver_init(LoquiReceiver *receiver);
static void loqui_receiver_finalize(GObject *object);
static void loqui_receiver_dispose(GObject *object);

static void loqui_receiver_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec);
static void loqui_receiver_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec);

GType
loqui_receiver_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(LoquiReceiverClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) loqui_receiver_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(LoquiReceiver),
				0,              /* n_preallocs */
				(GInstanceInitFunc) loqui_receiver_init
			};
		
		type = g_type_register_static(G_TYPE_OBJECT,
					      "LoquiReceiver",
					      &our_info,
					      0);
	}
	
	return type;
}
static void 
loqui_receiver_finalize(GObject *object)
{
	LoquiReceiver *receiver;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_RECEIVER(object));

        receiver = LOQUI_RECEIVER(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize)(object);

	g_free(receiver->priv);
}
static void 
loqui_receiver_dispose(GObject *object)
{
	LoquiReceiver *receiver;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_RECEIVER(object));

        receiver = LOQUI_RECEIVER(object);

        if (G_OBJECT_CLASS(parent_class)->dispose)
                (* G_OBJECT_CLASS(parent_class)->dispose)(object);
}
static void
loqui_receiver_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec)
{
        LoquiReceiver *receiver;        

        receiver = LOQUI_RECEIVER(object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}
static void
loqui_receiver_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec)
{
        LoquiReceiver *receiver;        

        receiver = LOQUI_RECEIVER(object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}

static void
loqui_receiver_class_init(LoquiReceiverClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = loqui_receiver_finalize;
        object_class->dispose = loqui_receiver_dispose;
        object_class->get_property = loqui_receiver_get_property;
        object_class->set_property = loqui_receiver_set_property;
}
static void 
loqui_receiver_init(LoquiReceiver *receiver)
{
	LoquiReceiverPrivate *priv;

	priv = g_new0(LoquiReceiverPrivate, 1);

	receiver->priv = priv;
}
LoquiReceiver*
loqui_receiver_new(LoquiAccount *account)
{
        LoquiReceiver *receiver;
	LoquiReceiverPrivate *priv;

	receiver = g_object_new(loqui_receiver_get_type(), NULL);
	
        priv = receiver->priv;
	receiver->account = account;

        return receiver;
}
LoquiAccount *
loqui_receiver_get_account(LoquiReceiver *receiver)
{
        g_return_val_if_fail(receiver != NULL, NULL);
        g_return_val_if_fail(LOQUI_IS_RECEIVER(receiver), NULL);

	/* check account is valid */
	g_return_val_if_fail(receiver->account != NULL, NULL);
	g_return_val_if_fail(LOQUI_IS_ACCOUNT(receiver->account), NULL);

	return receiver->account;
}
