/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/*
 *  Copyright (C) 2003 Takuro Ashie
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "egg-entry-action.h"

#include <gtk/gtkentry.h>

#include "intl.h"
#include "egg-toolitem.h"

enum {
  PROP_0,
  PROP_TEXT,
};

static void       egg_entry_action_init         (EggEntryAction *action);
static void       egg_entry_action_class_init   (EggEntryActionClass *class);

static void       egg_entry_action_dispose      (GObject *object);
static void       egg_entry_action_set_property (GObject      *object,
						 guint         prop_id,
						 const GValue *value,
						 GParamSpec   *pspec);
static void       egg_entry_action_get_property (GObject      *object,
						 guint         prop_id,
						 GValue       *value,
						 GParamSpec   *pspec);

static GtkWidget *create_menu_item              (EggAction *action);
static GtkWidget *create_tool_item              (EggAction *action);

static void       connect_proxy                 (EggAction *action,
						 GtkWidget *proxy);
static void       disconnect_proxy              (EggAction *action,
						 GtkWidget *proxy);

static GtkEntry  *egg_entry_action_real_get_entry_widget (EggEntryAction *action,
							  GtkWidget      *proxy);

static EggActionClass *parent_class = NULL;

GType
egg_entry_action_get_type (void)
{
	static GtkType type = 0;

	if (!type)
	{
		static const GTypeInfo type_info =
			{
				sizeof (EggEntryActionClass),
				(GBaseInitFunc) NULL,
				(GBaseFinalizeFunc) NULL,
				(GClassInitFunc) egg_entry_action_class_init,
				(GClassFinalizeFunc) NULL,
				NULL,
				sizeof (EggEntryAction),
				0, /* n_preallocs */
				(GInstanceInitFunc) egg_entry_action_init,
			};

		type = g_type_register_static (EGG_TYPE_ACTION,
					       "EggEntryAction",
					       &type_info, 0);
	}
	return type;
}

static void
egg_entry_action_class_init (EggEntryActionClass *klass)
{
	GObjectClass *object_class;
	EggActionClass *action_class;

	parent_class = g_type_class_peek_parent(klass);
	object_class = G_OBJECT_CLASS(klass);
	action_class = EGG_ACTION_CLASS(klass);

	object_class->dispose      = egg_entry_action_dispose;
	object_class->set_property = egg_entry_action_set_property;
	object_class->get_property = egg_entry_action_get_property;

	action_class->create_menu_item = create_menu_item;
	action_class->create_tool_item = create_tool_item;
	action_class->connect_proxy    = connect_proxy;
	action_class->disconnect_proxy = disconnect_proxy;

	action_class->toolbar_item_type = EGG_TYPE_TOOL_ITEM;

	klass->get_entry_widget = egg_entry_action_real_get_entry_widget;

	g_object_class_install_property(
		object_class,
		PROP_TEXT,
		g_param_spec_string("text",
				    _("Text"),
				    _("Text in entries."),
				    NULL,
				    G_PARAM_READWRITE));
}


static void
egg_entry_action_init (EggEntryAction *action)
{
}


static void
egg_entry_action_dispose (GObject *object)
{
	EggEntryAction *action = EGG_ENTRY_ACTION(object);

	if (action->text)
		g_free (action->text);
	action->text = NULL;

	if (G_OBJECT_CLASS (parent_class)->dispose)
		G_OBJECT_CLASS (parent_class)->dispose(object);
}


static void
egg_entry_action_set_property (GObject      *object,
			       guint         prop_id,
			       const GValue *value,
			       GParamSpec   *pspec)
{
	EggEntryAction *action;

	action = EGG_ENTRY_ACTION(object);

	switch (prop_id)
	{
	case PROP_TEXT:
		g_free(action->text);
		action->text = g_value_dup_string(value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}


static void
egg_entry_action_get_property (GObject    *object,
			       guint       prop_id,
			       GValue     *value,
			       GParamSpec *pspec)
{
	EggEntryAction *action;

	action = EGG_ENTRY_ACTION(object);

	switch (prop_id)
	{
	case PROP_TEXT:
		g_value_set_string(value, action->text);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}


static gboolean
cb_menu_item_entry_button_press (GtkWidget *widget)
{
	/* don't propagate to parent */
	return TRUE;
}


static gboolean
cb_menu_item_entry_button_release (GtkWidget *widget)
{
	/* don't propagate to parent */
	return TRUE;
}


static gboolean
cb_menu_item_entry_enter (GtkWidget *widget, GdkEventCrossing *event)
{
	gtk_grab_add(GTK_BIN(widget)->child);
	gtk_widget_grab_focus(GTK_BIN(widget)->child);

	return TRUE;
}


static gboolean
cb_menu_item_entry_leave (GtkWidget *widget, GdkEventCrossing *event)
{
	gtk_grab_remove(GTK_BIN(widget)->child);

	return TRUE;
}


static GtkWidget *
create_menu_item (EggAction *action)
{
	GtkWidget *widget, *entry;

/* #warning FIXME! implement as our original widget and force blink cursor. */
	widget = gtk_menu_item_new();
	entry = gtk_entry_new();
	gtk_widget_show(entry);
	gtk_container_add(GTK_CONTAINER(widget), entry);

	g_signal_connect_after(G_OBJECT(widget), "button-press-event",
			       G_CALLBACK(cb_menu_item_entry_button_press),
			       NULL);
	g_signal_connect_after(G_OBJECT(widget), "button-release-event",
			       G_CALLBACK(cb_menu_item_entry_button_release),
			       NULL);

	g_signal_connect(G_OBJECT(widget), "enter-notify-event",
			 G_CALLBACK(cb_menu_item_entry_enter), NULL);
	g_signal_connect(G_OBJECT(widget), "leave-notify-event",
			 G_CALLBACK(cb_menu_item_entry_leave), NULL);

	return widget;
}


static void
cb_entry_changed(GtkEditable *editable, EggAction *action)
{
	GtkEntry *entry;
	const gchar *text;

	g_return_if_fail(GTK_IS_ENTRY(editable));

	entry = GTK_ENTRY(editable);
	text = gtk_entry_get_text(entry);

	g_object_set(G_OBJECT(action), "text", text, NULL);
}


static void
cb_entry_activate(GtkEntry *entry, EggAction *action)
{
	g_return_if_fail(GTK_IS_ENTRY(entry));
	g_return_if_fail(EGG_IS_ENTRY_ACTION(action));

	egg_action_activate(action);
}


static GtkWidget *
create_tool_item (EggAction *action)
{
	GtkWidget *widget, *entry;

	widget = (*EGG_ACTION_CLASS(parent_class)->create_tool_item) (action);
	egg_tool_item_set_expandable (EGG_TOOL_ITEM(widget), TRUE);

	entry = gtk_entry_new();
	gtk_container_add(GTK_CONTAINER(widget), entry);
	gtk_widget_show(entry);

	return widget;
}


static GtkEntry *
egg_entry_action_real_get_entry_widget(EggEntryAction *action, GtkWidget *proxy)
{
	GtkEntry *entry = NULL;

	g_return_val_if_fail(proxy, NULL);

	if (GTK_IS_BIN(proxy))
	{
		if (GTK_IS_ENTRY(GTK_BIN(proxy)->child))
		{
			entry = GTK_ENTRY(GTK_BIN(proxy)->child);
		}
		else if (GTK_IS_COMBO(GTK_BIN(proxy)->child))
		{
			entry = GTK_ENTRY(GTK_COMBO(GTK_BIN(proxy)->child)->entry);
		}
	}
	else if (GTK_IS_ENTRY(proxy))
	{
		entry = GTK_ENTRY(proxy);
	}
	else if (GTK_IS_COMBO(proxy))
	{
		entry = GTK_ENTRY(GTK_COMBO(proxy)->entry);
	}

	if (GTK_IS_ENTRY(entry))
		return entry;

	return NULL;
}


GtkEntry *
egg_entry_action_get_entry_widget(EggEntryAction *action, GtkWidget *proxy)
{
	EggEntryActionClass *klass;

	g_return_val_if_fail(EGG_IS_ENTRY_ACTION(action), NULL);

	klass = EGG_ENTRY_ACTION_GET_CLASS(action);

	if (klass->get_entry_widget)
		return klass->get_entry_widget(action, proxy);

	return NULL;
}


static void
connect_proxy (EggAction *action, GtkWidget *proxy)
{
	EggEntryAction *entry_action;
	GtkEntry *entry = NULL;

	entry_action = EGG_ENTRY_ACTION (action);

	entry = egg_entry_action_get_entry_widget(entry_action, proxy);
	if (GTK_IS_ENTRY(entry)) {
		g_signal_connect(entry, "changed",
				 G_CALLBACK(cb_entry_changed), action);
		g_signal_connect(entry, "activate",
				 G_CALLBACK(cb_entry_activate), action);

		g_object_ref(action);
		g_object_set_data_full(G_OBJECT(proxy), "egg-action", action,
				       g_object_unref);

		/* add this widget to the list of proxies */
		action->proxies = g_slist_prepend(action->proxies, proxy);
		g_signal_connect(proxy, "destroy",
				 G_CALLBACK(egg_action_remove_proxy), action);

		g_signal_connect_object (action, "notify::sensitive",
					 G_CALLBACK (egg_action_sync_property),
					 proxy, 0);
		gtk_widget_set_sensitive(proxy, action->sensitive);

		g_signal_connect_object(action, "notify::visible",
					G_CALLBACK(egg_action_sync_property),
					proxy, 0);
		if (action->visible)
			gtk_widget_show(proxy);
		else
			gtk_widget_hide(proxy);

		/* sync entry text */
		g_signal_connect_object(action, "notify::text",
					G_CALLBACK(egg_action_sync_property),
					entry, 0);
		if (entry_action->text)
			gtk_entry_set_text(entry, entry_action->text);
	}
	else
	{
		EGG_ACTION_CLASS (parent_class)->connect_proxy (action, proxy);
	}
}


static void
disconnect_proxy (EggAction *action, GtkWidget *proxy)
{
	EggEntryAction *entry_action;
	GtkEntry *entry;

	entry_action = EGG_ENTRY_ACTION (action);

	entry = egg_entry_action_get_entry_widget(entry_action, proxy);
	if (entry)
	{
		g_signal_handlers_disconnect_by_func
			(entry,  G_CALLBACK(cb_entry_changed), action);
		g_signal_handlers_disconnect_by_func
			(entry, G_CALLBACK(cb_entry_activate), action);
	}

	/* other signals will be remove by parent */
	EGG_ACTION_CLASS (parent_class)->disconnect_proxy(action, proxy);
}


void
egg_entry_action_set_text (EggEntryAction *action,
			   const gchar *text)
{
	g_return_if_fail(EGG_IS_ENTRY_ACTION(action));

	g_object_set(action, "text", text, NULL);
}


const gchar *
egg_entry_action_get_text (EggEntryAction *action)
{
	g_return_val_if_fail(EGG_IS_ENTRY_ACTION(action), NULL);

	return action->text;
}

