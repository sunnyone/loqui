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
#include "loqui_channel_entry_ui.h"

static guint loqui_app_new_channel_entry_id(LoquiApp *app);
static void loqui_channel_entry_action_activate_cb(LoquiChannelEntryAction *action, LoquiApp *app);

static guint
loqui_app_new_channel_entry_id(LoquiApp *app)
{
	return (++app->channel_entry_id_max);
}
static void
loqui_channel_entry_action_activate_cb(LoquiChannelEntryAction *action, LoquiApp *app)
{
	LoquiChannelEntry *chent;

	if (!loqui_app_has_toplevel_focus(app))
		gtk_window_present(GTK_WINDOW(app));

	chent = loqui_channel_entry_action_get_channel_entry(action);
	loqui_app_set_current_channel_entry(app, chent);
}
void
loqui_channel_entry_ui_attach_channel_entry_action(LoquiApp *app, LoquiChannelEntry *channel_entry)
{
	LoquiChannelEntryAction *ce_action;
	gchar *ce_name;

	ce_name = g_strdup_printf("ChannelEntry-%d", loqui_app_new_channel_entry_id(app));
	ce_action = loqui_channel_entry_action_new(ce_name);
	loqui_channel_entry_action_set_channel_entry(ce_action, channel_entry);
	g_signal_connect(G_OBJECT(ce_action), "activate", G_CALLBACK(loqui_channel_entry_action_activate_cb), app);
	g_object_set_data_full(G_OBJECT(channel_entry), "channel-entry-action", ce_action, (GDestroyNotify) g_object_unref);
	gtk_action_group_add_action(app->channel_entry_group, GTK_ACTION(ce_action));
	g_object_unref(ce_action);
	g_free(ce_name);
}
void
loqui_channel_entry_ui_add_account(LoquiApp *app, LoquiAccount *account, const gchar *path, const gchar *data_prefix)
{
	gchar *ph_name, *ph_path, *data_name;
	const gchar *ce_name;
	guint id;
	LoquiChannelEntryAction *ce_action;
	GtkAction *ph_action;

	g_return_if_fail(account != NULL);
	g_return_if_fail(LOQUI_IS_ACCOUNT(account));
        g_return_if_fail(app != NULL);
        g_return_if_fail(LOQUI_IS_APP(app));

	ce_action = g_object_get_data(G_OBJECT(account), "channel-entry-action");
	g_return_if_fail(ce_action != NULL);

	ce_name = gtk_action_get_name(GTK_ACTION(ce_action));

	id = gtk_ui_manager_new_merge_id(app->ui_manager);
	ph_name = g_strdup_printf("%s-ChannelEntryPlaceHolder-%d", data_prefix, id);
	ph_action = gtk_action_new(ph_name, "", NULL, NULL);
	gtk_action_group_add_action(app->channel_entry_group, ph_action);
	gtk_ui_manager_add_ui(app->ui_manager, id, path, ph_name, ph_name, GTK_UI_MANAGER_PLACEHOLDER, FALSE);
	g_object_unref(ph_action);

	data_name = g_strdup_printf("%s-placeholder-name", data_prefix);
	g_object_set_data_full(G_OBJECT(account), data_name, ph_name, (GDestroyNotify) g_free);
	g_free(data_name);

	data_name = g_strdup_printf("%s-placeholder-merge-id", data_prefix);
	g_object_set_data(G_OBJECT(account), data_name, GUINT_TO_POINTER(id));
	g_free(data_name);

	id = gtk_ui_manager_new_merge_id(app->ui_manager);
	ph_path = g_strconcat(path, "/", ph_name, NULL);
	gtk_ui_manager_add_ui(app->ui_manager, id, ph_path, ce_name, ce_name, GTK_UI_MANAGER_MENUITEM, FALSE);
	data_name = g_strdup_printf("%s-menuitem-merge-id", data_prefix);
	g_object_set_data(G_OBJECT(account), data_name, GUINT_TO_POINTER(id));
	g_free(data_name);
	g_free(ph_path);
}
void
loqui_channel_entry_ui_remove_account(LoquiApp *app, LoquiAccount *account, const gchar *path, const gchar *data_prefix)
{
	LoquiChannelEntryAction *ce_action;
	gchar *data_name;
	guint id;

	g_return_if_fail(account != NULL);
	g_return_if_fail(LOQUI_IS_ACCOUNT(account));
        g_return_if_fail(app != NULL);
        g_return_if_fail(LOQUI_IS_APP(app));

	ce_action = g_object_get_data(G_OBJECT(account), "channel-entry-action");
	g_return_if_fail(ce_action != NULL);

	data_name = g_strdup_printf("%s-menuitem-merge-id", data_prefix);
	id = GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(account), data_name));
	gtk_ui_manager_remove_ui(app->ui_manager, id);
	g_free(data_name);

	data_name = g_strdup_printf("%s-placeholder-merge-id", data_prefix);
	id = GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(account), data_name));
	gtk_ui_manager_remove_ui(app->ui_manager, id);
	g_free(data_name);
}
void
loqui_channel_entry_ui_add_channel(LoquiApp *app, LoquiChannel *channel, const gchar *path, const gchar *data_prefix)
{
	gchar *ph_name, *ph_path, *data_name;
	const gchar *ce_name;
	guint id;
	LoquiChannelEntryAction *ce_action;

	g_return_if_fail(channel != NULL);
	g_return_if_fail(LOQUI_IS_CHANNEL(channel));
        g_return_if_fail(app != NULL);
        g_return_if_fail(LOQUI_IS_APP(app));

	ce_action = g_object_get_data(G_OBJECT(channel), "channel-entry-action");
	g_return_if_fail(ce_action != NULL);

	ce_name = gtk_action_get_name(GTK_ACTION(ce_action));

	data_name = g_strdup_printf("%s-placeholder-name", data_prefix);
	ph_name = g_object_get_data(G_OBJECT(channel->account), data_name);
	g_free(data_name);

	id = gtk_ui_manager_new_merge_id(app->ui_manager);
	ph_path = g_strconcat(path, "/", ph_name, NULL);
	gtk_ui_manager_add_ui(app->ui_manager, id, ph_path, ce_name, ce_name, GTK_UI_MANAGER_MENUITEM, FALSE);
	data_name = g_strdup_printf("%s-menuitem-merge-id", data_prefix);
	g_object_set_data(G_OBJECT(channel), data_name, GUINT_TO_POINTER(id));
	g_free(data_name);
	g_free(ph_path);
}

void
loqui_channel_entry_ui_remove_channel(LoquiApp *app, LoquiChannel *channel, const gchar *data_prefix)
{
	gchar *data_name;
	guint id;

	g_return_if_fail(channel != NULL);
	g_return_if_fail(LOQUI_IS_CHANNEL(channel));
        g_return_if_fail(app != NULL);
        g_return_if_fail(LOQUI_IS_APP(app));

	data_name = g_strdup_printf("%s-menuitem-merge-id", data_prefix);
	id = GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(channel), data_name));
	gtk_ui_manager_remove_ui(app->ui_manager, id);
	g_free(data_name);
}
