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

#include "loqui_account.h"
#include "loqui_stock.h"
#include "loqui_gtk.h"
#include "gtkutils.h"

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

static void loqui_channel_entry_action_entry_notify_is_updated_cb(LoquiChannelEntry *chent, GParamSpec *psec, LoquiChannelEntryAction *action);
static void loqui_channel_entry_action_entry_notify_name_cb(LoquiChannelEntry *chent, GParamSpec *psec, LoquiChannelEntryAction *action);
static void loqui_channel_entry_action_set_label_color(LoquiChannelEntryAction *ce_ction);

static void loqui_channel_entry_action_entry_notify_id_cb(LoquiChannelEntry *ce_action, GParamSpec *psec, LoquiChannelEntryAction *action);
static void loqui_channel_entry_set_id_accel_path(LoquiChannelEntryAction *action);

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
	loqui_channel_entry_action_set_label_color(ce_action);
}
static void
loqui_channel_entry_action_set_label_color(LoquiChannelEntryAction *action)
{
	const gchar *color;
	GtkWidget *label;
	GList *children;
	GtkWidget *proxy;
	GSList *cur;

	g_return_if_fail(action != NULL);
	g_return_if_fail(LOQUI_IS_CHANNEL_ENTRY_ACTION(action));

	if(!action->channel_entry)
		return;

	for (cur = gtk_action_get_proxies(GTK_ACTION(action)); cur != NULL; cur = cur->next) {
		proxy = cur->data;

		if (loqui_channel_entry_get_has_unread_keyword(action->channel_entry))
			color = HIGHLIGHT_COLOR;
		else if (loqui_channel_entry_get_is_updated(action->channel_entry))
			color = FRESH_COLOR;
		else if (loqui_channel_entry_get_is_updated_weak(action->channel_entry))
			color = FRESH_WEAK_COLOR;
		else
			color = NONFRESH_COLOR;

		if (GTK_IS_LABEL(proxy))
			gtkutils_set_label_color(GTK_LABEL(proxy), color);
		else if (GTK_IS_MENU_ITEM(proxy)) {
			children = gtk_container_get_children(GTK_CONTAINER(proxy));
			label = children->data;
			if(!label || !GTK_IS_LABEL(label)) {
				g_warning("Child is not label!");
				return;
			}
			gtkutils_set_label_color(GTK_LABEL(label), color);
		}
	}
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
							    LOQUI_TYPE_CHANNEL_ENTRY,
							    G_PARAM_READWRITE));
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
static void
loqui_channel_entry_action_entry_notify_is_updated_cb(LoquiChannelEntry *chent, GParamSpec *psec, LoquiChannelEntryAction *action)
{
	loqui_channel_entry_action_set_label_color(action);
}
static void
loqui_channel_entry_action_entry_notify_name_cb(LoquiChannelEntry *chent, GParamSpec *psec, LoquiChannelEntryAction *action)
{
	g_object_set(G_OBJECT(action), "label", loqui_channel_entry_get_name(chent), NULL);
}
static void
loqui_channel_entry_action_entry_notify_id_cb(LoquiChannelEntry *ce_action, GParamSpec *psec, LoquiChannelEntryAction *action)
{
	loqui_channel_entry_set_id_accel_path(action);
}
static void
loqui_channel_entry_set_id_accel_path(LoquiChannelEntryAction *action)
{
	gint id;
	gchar *path;

	if (action->channel_entry)
		id = loqui_channel_entry_get_id(action->channel_entry);
	else
		return;

	if (id < 0)
		return;

	path = g_strdup_printf(SHORTCUT_CHANNEL_ENTRY_ACCEL_MAP_PREFIX "%d", id);
	gtk_action_set_accel_path(GTK_ACTION(action), path);
	g_free(path);
}
void
loqui_channel_entry_action_set_channel_entry(LoquiChannelEntryAction *action, LoquiChannelEntry *channel_entry)
{
	g_return_if_fail(action != NULL);
	g_return_if_fail(LOQUI_IS_CHANNEL_ENTRY_ACTION(action));

	if (channel_entry)
		g_return_if_fail(LOQUI_IS_CHANNEL_ENTRY(channel_entry));

	g_signal_handlers_disconnect_by_func(G_OBJECT(channel_entry),
					     loqui_channel_entry_action_entry_notify_is_updated_cb, action);
	g_signal_handlers_disconnect_by_func(G_OBJECT(channel_entry),
					     loqui_channel_entry_action_entry_notify_name_cb, action);
	g_signal_handlers_disconnect_by_func(G_OBJECT(channel_entry),
					     loqui_channel_entry_action_entry_notify_id_cb, action);

	if (channel_entry) {
		action->channel_entry = channel_entry;
		g_signal_connect(G_OBJECT(channel_entry), "notify::id",
				 G_CALLBACK(loqui_channel_entry_action_entry_notify_id_cb), action);

		g_signal_connect(G_OBJECT(channel_entry), "notify::is-updated",
				 G_CALLBACK(loqui_channel_entry_action_entry_notify_is_updated_cb), action);
		g_signal_connect(G_OBJECT(channel_entry), "notify::name",
				 G_CALLBACK(loqui_channel_entry_action_entry_notify_name_cb), action);

		g_object_set(G_OBJECT(action), "label", loqui_channel_entry_get_name(channel_entry), NULL);

		if (LOQUI_IS_ACCOUNT(channel_entry))
			g_object_set(G_OBJECT(action), "stock_id", LOQUI_STOCK_CONSOLE, NULL);

		loqui_channel_entry_action_set_label_color(action);

		loqui_channel_entry_set_id_accel_path(action);
	}
}
LoquiChannelEntry *
loqui_channel_entry_action_get_channel_entry(LoquiChannelEntryAction *action)
{
	g_return_val_if_fail(action != NULL, NULL);
	g_return_val_if_fail(LOQUI_IS_CHANNEL_ENTRY_ACTION(action), NULL);

	return action->channel_entry;
}
