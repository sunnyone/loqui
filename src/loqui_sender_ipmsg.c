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

#include "loqui_sender_ipmsg.h"

enum {
        LAST_SIGNAL
};

enum {
        LAST_PROP
};

struct _LoquiSenderIPMsgPrivate
{
};

static LoquiSenderClass *parent_class = NULL;

/* static guint loqui_sender_ipmsg_signals[LAST_SIGNAL] = { 0 }; */

static void loqui_sender_ipmsg_class_init(LoquiSenderIPMsgClass *klass);
static void loqui_sender_ipmsg_init(LoquiSenderIPMsg *sender);
static void loqui_sender_ipmsg_finalize(GObject *object);
static void loqui_sender_ipmsg_dispose(GObject *object);

static void loqui_sender_ipmsg_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec);
static void loqui_sender_ipmsg_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec);

GType
loqui_sender_ipmsg_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(LoquiSenderIPMsgClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) loqui_sender_ipmsg_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(LoquiSenderIPMsg),
				0,              /* n_preallocs */
				(GInstanceInitFunc) loqui_sender_ipmsg_init
			};
		
		type = g_type_register_static(LOQUI_TYPE_SENDER,
					      "LoquiSenderIPMsg",
					      &our_info,
					      0);
	}
	
	return type;
}
static void 
loqui_sender_ipmsg_finalize(GObject *object)
{
	LoquiSenderIPMsg *sender;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_SENDER_IPMSG(object));

        sender = LOQUI_SENDER_IPMSG(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize)(object);

	g_free(sender->priv);
}
static void 
loqui_sender_ipmsg_dispose(GObject *object)
{
	LoquiSenderIPMsg *sender;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_SENDER_IPMSG(object));

        sender = LOQUI_SENDER_IPMSG(object);

        if (G_OBJECT_CLASS(parent_class)->dispose)
                (* G_OBJECT_CLASS(parent_class)->dispose)(object);
}
static void
loqui_sender_ipmsg_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec)
{
        LoquiSenderIPMsg *sender;        

        sender = LOQUI_SENDER_IPMSG(object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}
static void
loqui_sender_ipmsg_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec)
{
        LoquiSenderIPMsg *sender;        

        sender = LOQUI_SENDER_IPMSG(object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}

static void
loqui_sender_ipmsg_class_init(LoquiSenderIPMsgClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = loqui_sender_ipmsg_finalize;
        object_class->dispose = loqui_sender_ipmsg_dispose;
        object_class->get_property = loqui_sender_ipmsg_get_property;
        object_class->set_property = loqui_sender_ipmsg_set_property;
}
static void 
loqui_sender_ipmsg_init(LoquiSenderIPMsg *sender)
{
	LoquiSenderIPMsgPrivate *priv;

	priv = g_new0(LoquiSenderIPMsgPrivate, 1);

	sender->priv = priv;
}
LoquiSenderIPMsg*
loqui_sender_ipmsg_new(LoquiAccount *account)
{
        LoquiSenderIPMsg *sender;
	LoquiSenderIPMsgPrivate *priv;

	sender = g_object_new(loqui_sender_ipmsg_get_type(), NULL);
	
        priv = sender->priv;
	LOQUI_SENDER(sender)->account = account;

        return sender;
}
