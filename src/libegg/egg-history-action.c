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

#include "egg-history-action.h"

#include <string.h>

#include "intl.h"
#include "egg-toolitem.h"


enum {
	HISTORY_CHANGED_SIGNAL,
	LAST_SIGNAL
};

static void       egg_history_action_class_init       (EggHistoryActionClass *klass);
static void       egg_history_action_init             (EggHistoryAction *action);
static void       egg_history_action_dispose          (GObject *object);

static void       egg_history_action_activate         (EggAction *action);
static GtkWidget *egg_history_action_create_menu_item (EggAction *action);
static GtkWidget *egg_history_action_create_tool_item (EggAction *action);

static GtkCombo  *egg_history_action_real_get_combo_widget
						      (EggHistoryAction *action,
						       GtkWidget        *proxy);
static void       egg_history_action_real_set_history (EggHistoryAction *action,
						       GList            *list);

static void       egg_history_action_connect_proxy    (EggAction        *action,
						       GtkWidget        *proxy);

static EggEntryActionClass *parent_class = NULL;

static gint history_action_signals[LAST_SIGNAL] = { 0 };

GType
egg_history_action_get_type (void)
{
	static GtkType type = 0;

	if (!type)
	{
		static const GTypeInfo type_info =
			{
				sizeof (EggHistoryActionClass),
				(GBaseInitFunc) NULL,
				(GBaseFinalizeFunc) NULL,
				(GClassInitFunc) egg_history_action_class_init,
				(GClassFinalizeFunc) NULL,
				NULL,
				sizeof (EggHistoryAction),
				0, /* n_preallocs */
				(GInstanceInitFunc) egg_history_action_init,
			};

		type = g_type_register_static (EGG_TYPE_ENTRY_ACTION,
					       "EggHistoryAction",
					       &type_info, 0);
	}
	return type;
}

static void
egg_history_action_class_init (EggHistoryActionClass *klass)
{
	GObjectClass *object_class;
	EggActionClass *action_class;
	EggEntryActionClass *entry_action_class;

	parent_class = g_type_class_peek_parent(klass);
	object_class = G_OBJECT_CLASS(klass);
	action_class = EGG_ACTION_CLASS(klass);
	entry_action_class = EGG_ENTRY_ACTION_CLASS(klass);

	object_class->dispose      = egg_history_action_dispose;
	/*
	object_class->set_property = egg_history_action_set_property;
	object_class->get_property = egg_history_action_get_property;
	*/

	action_class->activate         = egg_history_action_activate;
	action_class->create_menu_item = egg_history_action_create_menu_item;
	action_class->create_tool_item = egg_history_action_create_tool_item;
	action_class->connect_proxy    = egg_history_action_connect_proxy;

	action_class->toolbar_item_type = EGG_TYPE_TOOL_ITEM;

	klass->get_combo_widget = egg_history_action_real_get_combo_widget;
	klass->set_history      = egg_history_action_real_set_history;
	klass->history_changed  = NULL;

	history_action_signals[HISTORY_CHANGED_SIGNAL] =
		g_signal_new("history-changed",
			     G_OBJECT_CLASS_TYPE(klass),
			     G_SIGNAL_RUN_FIRST | G_SIGNAL_NO_RECURSE,
			     G_STRUCT_OFFSET(EggHistoryActionClass, history_changed),
			     NULL,
			     NULL,
			     g_cclosure_marshal_VOID__VOID,
			     G_TYPE_NONE, 0);
}


static void
egg_history_action_init (EggHistoryAction *action)
{
	action->max_history = 32;
	action->history     = NULL;
}


static void
egg_history_action_dispose (GObject *object)
{
	EggHistoryAction *action = EGG_HISTORY_ACTION(object);

	if (action->history)
	{
		g_list_foreach (action->history, (GFunc) g_free, NULL);
		g_list_free(action->history);
		action->history = NULL;
	}

	if (G_OBJECT_CLASS (parent_class)->dispose)
		G_OBJECT_CLASS (parent_class)->dispose(object);
}


static void
egg_history_action_activate (EggAction *action)
{
	EggHistoryAction *hist_act = EGG_HISTORY_ACTION(action);
	const gchar *text;

	text = egg_entry_action_get_text(EGG_ENTRY_ACTION(action));

	if (text && *text)
	{
		GList *node;
		GSList *snode;
		GList *prev;

		/* append, or reorder history */
		prev = g_list_find_custom(hist_act->history, text,
					  (GCompareFunc)strcmp);

		if (prev)
		{
			text = prev->data;
			hist_act->history = g_list_remove(hist_act->history,
							  text);
		}
		else
		{
			text = g_strdup(text);
		}

		hist_act->history = g_list_prepend(hist_act->history,
						   (gpointer) text);

		/* check max history */
		if (g_list_length(hist_act->history) > hist_act->max_history)
		{
			node = g_list_nth(hist_act->history, hist_act->max_history);
			while (node)
			{
				GList *tmp;
				gchar *str = node->data;

				tmp = g_list_next(node);
				hist_act->history = g_list_remove(hist_act->history, str);
				g_free(str);
				node = tmp;
			}
		}

		/* set history to all proxy widgets */
		for (snode = action->proxies; snode; snode = g_slist_next(snode))
		{
			GtkCombo *combo;
			GtkWidget *proxy = snode->data;

			combo = egg_history_action_get_combo_widget
					(EGG_HISTORY_ACTION(action),
					 proxy);
			if (GTK_IS_COMBO(combo))
				gtk_combo_set_popdown_strings(combo,
							      hist_act->history);
		}
	}

	g_signal_emit(G_OBJECT(action),
		      history_action_signals[HISTORY_CHANGED_SIGNAL], 0);

	/* not needed */
	if (EGG_ACTION_CLASS (parent_class)->activate)
		EGG_ACTION_CLASS (parent_class)->activate(action);
}


static gboolean
cb_menu_item_combo_button_press (GtkWidget *widget)
{
	/* don't propagate to parent */
	return TRUE;
}


static gboolean
cb_menu_item_combo_button_release (GtkWidget *widget)
{
	/* don't propagate to parent */
	return TRUE;
}


static gboolean
cb_menu_item_combo_enter (GtkWidget *widget, GdkEventCrossing *event)
{
	gtk_grab_add(GTK_BIN(widget)->child);
	gtk_widget_grab_focus(GTK_BIN(widget)->child);

	return TRUE;
}


static gboolean
cb_menu_item_combo_leave (GtkWidget *widget, GdkEventCrossing *event)
{
	gtk_grab_remove(GTK_BIN(widget)->child);

	return TRUE;
}


static GtkWidget *
egg_history_action_create_menu_item (EggAction *action)
{
	GtkWidget *widget, *combo;

#warning FIXME! implement as our original widget and force blink cursor.
	widget = gtk_menu_item_new();
	combo = gtk_combo_new();
	gtk_widget_show(combo);
	gtk_container_add(GTK_CONTAINER(widget), combo);

	g_signal_connect_after(G_OBJECT(widget), "button-press-event",
			       G_CALLBACK(cb_menu_item_combo_button_press),
			       NULL);
	g_signal_connect_after(G_OBJECT(widget), "button-release-event",
			       G_CALLBACK(cb_menu_item_combo_button_release),
			       NULL);

	g_signal_connect(G_OBJECT(widget), "enter-notify-event",
			 G_CALLBACK(cb_menu_item_combo_enter), NULL);
	g_signal_connect(G_OBJECT(widget), "leave-notify-event",
			 G_CALLBACK(cb_menu_item_combo_leave), NULL);

	return widget;
}


static GtkWidget *
egg_history_action_create_tool_item (EggAction *action)
{
	GType toolbar_item_type;
	GtkWidget *widget, *combo;

	toolbar_item_type = EGG_ACTION_GET_CLASS(action)->toolbar_item_type;
	widget = GTK_WIDGET(g_object_new(toolbar_item_type, NULL));
	egg_tool_item_set_expandable (EGG_TOOL_ITEM(widget), TRUE);

	combo = gtk_combo_new();
	gtk_container_add(GTK_CONTAINER(widget), combo);
	gtk_container_set_border_width(GTK_CONTAINER(widget), 4);
	gtk_combo_disable_activate(GTK_COMBO(combo));
	gtk_widget_show(combo);

	return widget;
}


static void
egg_history_action_connect_proxy (EggAction *action, GtkWidget *proxy)
{
	GtkCombo *combo;

	g_return_if_fail(proxy);

	EGG_ACTION_CLASS(parent_class)->connect_proxy(action, proxy);

	if (EGG_HISTORY_ACTION(action)->history)
	{
		combo = egg_history_action_get_combo_widget(EGG_HISTORY_ACTION(action),
							    proxy);
		if (GTK_IS_COMBO(combo))
		{
			gtk_combo_set_popdown_strings
				(GTK_COMBO(combo), EGG_HISTORY_ACTION(action)->history);
			gtk_entry_set_text(GTK_ENTRY(combo->entry), "");
		}
	}
}


static GtkCombo *
egg_history_action_real_get_combo_widget (EggHistoryAction *action, GtkWidget *proxy)
{
	GtkCombo *combo = NULL;

	g_return_val_if_fail(proxy, NULL);

	if (GTK_IS_BIN(proxy))
		combo = GTK_COMBO(GTK_BIN(proxy)->child);
	else
		combo = GTK_COMBO(proxy);

	if (GTK_IS_COMBO(combo))
		return combo;

	return NULL;
}


GtkCombo *
egg_history_action_get_combo_widget (EggHistoryAction *action, GtkWidget *proxy)
{
	EggHistoryActionClass *klass;

	g_return_val_if_fail(EGG_IS_ENTRY_ACTION(action), NULL);

	klass = EGG_HISTORY_ACTION_GET_CLASS(action);

	if (klass->get_combo_widget)
		return klass->get_combo_widget(action, proxy);

	return NULL;
}


static void
egg_history_action_real_set_history (EggHistoryAction *action, GList *list)
{
	GList *node, *new_list = NULL;
	GSList *snode;
	gint num = 0;

	g_return_if_fail(EGG_IS_ENTRY_ACTION(action));

	/* dup the list */
	for (node = list, num = 0;
	     node && num < action->max_history;
	     node = g_list_next(node), num++)
	{
		const gchar *text = node->data;

		if (text && *text)
			new_list = g_list_append(new_list, g_strdup(text));
	}

	/* free old list */
	g_list_foreach(action->history, (GFunc) g_free, NULL);
	g_list_free(action->history);

	/* set history */
	action->history = new_list;

	/* set history to all proxy widgets */
	for (snode = EGG_ACTION(action)->proxies;
	     snode;
	     snode = g_slist_next(snode))
	{
		GtkCombo *combo;
		GtkWidget *proxy = snode->data;

		combo = egg_history_action_get_combo_widget
				(EGG_HISTORY_ACTION(action),
				 proxy);
		if (GTK_IS_COMBO(combo))
			gtk_combo_set_popdown_strings(combo,
						      action->history);
	}

	g_signal_emit(G_OBJECT(action),
		      history_action_signals[HISTORY_CHANGED_SIGNAL], 0);
}


void
egg_history_action_set_history (EggHistoryAction *action, GList *list)
{
	EggHistoryActionClass *klass;

	g_return_if_fail(EGG_IS_ENTRY_ACTION(action));

	klass = EGG_HISTORY_ACTION_GET_CLASS(action);

	if (klass->set_history)
		return klass->set_history(action, list);
}


const GList *
egg_history_action_get_history (EggHistoryAction *action)
{
	g_return_val_if_fail(EGG_IS_ENTRY_ACTION(action), NULL);

	return action->history;
}


void
egg_history_action_set_max_history (EggHistoryAction *action, guint max_history)
{
	g_return_if_fail(EGG_IS_ENTRY_ACTION(action));

	action->max_history = max_history;
}


guint
egg_history_action_get_max_history (EggHistoryAction *action)
{
	g_return_val_if_fail(EGG_IS_ENTRY_ACTION(action), 0);

	return action->max_history;
}
