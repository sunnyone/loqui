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

#include "loqui_channel_entry_action.h"
#include "gobject_utils.h"
#include "intl.h"

/* FIXME */
#include "loqui_channel.h"
#include "account.h"
#include "loqui_stock.h"

enum {
        LAST_SIGNAL
};

enum {
	PROP_0,
	PROP_CHANNEL_ENTRY,
        LAST_PROP
};

struct _LoquiChannelEntryActionPrivate
{
};

static GtkActionClass *parent_class = NULL;

/* static guint loqui_channel_entry_action_signals[LAST_SIGNAL] = { 0 }; */

static void loqui_channel_entry_action_class_init(LoquiChannelEntryActionClass *klass);
static void loqui_channel_entry_action_init(LoquiChannelEntryAction *action);
static void loqui_channel_entry_action_finalize(GObject *object);
static void loqui_channel_entry_action_dispose(GObject *object);

static void loqui_channel_entry_action_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec);
static void loqui_channel_entry_action_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec);

GType
loqui_channel_entry_action_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(LoquiChannelEntryActionClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) loqui_channel_entry_action_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(LoquiChannelEntryAction),
				0,              /* n_preallocs */
				(GInstanceInitFunc) loqui_channel_entry_action_init
			};
		
		type = g_type_register_static(GTK_TYPE_ACTION,
					      "LoquiChannelEntryAction",
					      &our_info,
					      0);
	}
	
	return type;
}
static void 
loqui_channel_entry_action_finalize(GObject *object)
{
	LoquiChannelEntryAction *action;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL_ENTRY_ACTION(object));

        action = LOQUI_CHANNEL_ENTRY_ACTION(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize)(object);

	g_free(action->priv);
}
static void 
loqui_channel_entry_action_dispose(GObject *object)
{
	LoquiChannelEntryAction *action;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL_ENTRY_ACTION(object));

        action = LOQUI_CHANNEL_ENTRY_ACTION(object);

	G_OBJECT_UNREF_UNLESS_NULL(action->channel_entry);

        if (G_OBJECT_CLASS(parent_class)->dispose)
                (* G_OBJECT_CLASS(parent_class)->dispose)(object);
}
static void
loqui_channel_entry_action_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec)
{
        LoquiChannelEntryAction *action;        

        action = LOQUI_CHANNEL_ENTRY_ACTION(object);

        switch (param_id) {
 	case PROP_CHANNEL_ENTRY:
		g_value_set_object(value, loqui_channel_entry_action_get_channel_entry(action));
		break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}
static void
loqui_channel_entry_action_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec)
{
        LoquiChannelEntryAction *action;        

        action = LOQUI_CHANNEL_ENTRY_ACTION(object);

        switch (param_id) {
	case PROP_CHANNEL_ENTRY:
		loqui_channel_entry_action_set_channel_entry(action, g_value_get_object(value));
		break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}
static void
loqui_channel_entry_action_connect_proxy(GtkAction *action, GtkWidget *proxy)
{
	LoquiChannelEntryAction *ce_action;
	gchar *label;

	g_return_if_fail(action != NULL);
	g_return_if_fail(LOQUI_IS_CHANNEL_ENTRY_ACTION(action));

	ce_action = LOQUI_CHANNEL_ENTRY_ACTION(action);

	if (GTK_IS_LABEL(proxy)) {
		g_object_get(G_OBJECT(ce_action), "label", &label, NULL);
		gtk_label_set(GTK_LABEL(proxy), label);
	}

	(* parent_class->connect_proxy) (action, proxy);
}
static void
loqui_channel_entry_action_class_init(LoquiChannelEntryActionClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = loqui_channel_entry_action_finalize;
        object_class->dispose = loqui_channel_entry_action_dispose;
        object_class->get_property = loqui_channel_entry_action_get_property;
        object_class->set_property = loqui_channel_entry_action_set_property;

	GTK_ACTION_CLASS(klass)->connect_proxy = loqui_channel_entry_action_connect_proxy;

	g_object_class_install_property(object_class,
					PROP_CHANNEL_ENTRY,
					g_param_spec_object("channel_entry",
							    _("ChannelEntry"),
							    _("ChannelEntry"),
							    G_TYPE_OBJECT, G_PARAM_READWRITE)); /* FIXME */
}
static void 
loqui_channel_entry_action_init(LoquiChannelEntryAction *action)
{
	LoquiChannelEntryActionPrivate *priv;

	priv = g_new0(LoquiChannelEntryActionPrivate, 1);

	action->priv = priv;
}
LoquiChannelEntryAction*
loqui_channel_entry_action_new(const gchar *name)
{
        LoquiChannelEntryAction *action;
	LoquiChannelEntryActionPrivate *priv;

	action = g_object_new(loqui_channel_entry_action_get_type(), "name", name, NULL);
	
        priv = action->priv;

        return action;
}
void
loqui_channel_entry_action_set_channel_entry(LoquiChannelEntryAction *action, GObject *channel_entry)
{
	const gchar *name;

	g_return_if_fail(action != NULL);
	g_return_if_fail(LOQUI_IS_CHANNEL_ENTRY_ACTION(action));

	/* FIXME */
	if (channel_entry)
		g_return_if_fail(LOQUI_IS_CHANNEL(channel_entry) || IS_ACCOUNT(channel_entry));

	G_OBJECT_UNREF_UNLESS_NULL(action->channel_entry);

	if (channel_entry) {
		g_object_ref(channel_entry);
		action->channel_entry = channel_entry;
		/* FIXME */
		if (LOQUI_IS_CHANNEL(channel_entry))
			name = loqui_channel_entry_get_name(LOQUI_CHANNEL_ENTRY(channel_entry));
		else if (IS_ACCOUNT(channel_entry)) {
			name = loqui_profile_account_get_name(account_get_profile(ACCOUNT(channel_entry)));
			g_object_set(G_OBJECT(action), "stock_id", LOQUI_STOCK_CONSOLE, NULL);
		} else {
			name = NULL;
		}

		g_object_set(G_OBJECT(action), "label", name, NULL);
	}
}
GObject * /* FIXME */
loqui_channel_entry_action_get_channel_entry(LoquiChannelEntryAction *action)
{
	g_return_val_if_fail(action != NULL, NULL);
	g_return_val_if_fail(LOQUI_IS_CHANNEL_ENTRY_ACTION(action), NULL);

	return action->channel_entry;
}
