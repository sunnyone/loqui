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
#include "loqui_actions.h"
#include "loqui_stock.h"
#include "intl.h"

#include "account_manager.h"
#include "about.h"
#include "remark_entry.h"
#include "command_dialog.h"
#include "gtkutils.h"

#include <egg-toggle-action.h>

#define CTRL "<control>"
#define ALT  "<alt>"
#define SHIFT "<shift>"

static void loqui_actions_about_cb(EggAction *action, LoquiApp *app);
static void loqui_actions_common_settings_cb(EggAction *action, LoquiApp *app);
static void loqui_actions_account_settings_cb(EggAction *action, LoquiApp *app);
static void loqui_actions_connect_cb(EggAction *action, LoquiApp *app);
static void loqui_actions_quit_cb(EggAction *action, LoquiApp *app);
static void loqui_actions_edit_menu_cb(EggAction *action, LoquiApp *app);
static void loqui_actions_cut_cb(EggAction *action, LoquiApp *app);
static void loqui_actions_copy_cb(EggAction *action, LoquiApp *app);
static void loqui_actions_paste_cb(EggAction *action, LoquiApp *app);
static void loqui_actions_clear_cb(EggAction *action, LoquiApp *app);
static void loqui_actions_toggle_channelbar_cb(EggAction *action, LoquiApp *app);
static void loqui_actions_toggle_statusbar_cb(EggAction *action, LoquiApp *app);
static void loqui_actions_toggle_command_mode_cb(EggAction *action, LoquiApp *app);
static void loqui_actions_previous_updated_channel_cb(EggAction *action, LoquiApp *app);
static void loqui_actions_next_updated_channel_cb(EggAction *action, LoquiApp *app);
static void loqui_actions_previous_channel_cb(EggAction *action, LoquiApp *app);
static void loqui_actions_next_channel_cb(EggAction *action, LoquiApp *app);
static void loqui_actions_join_cb(EggAction *action, LoquiApp *app);
static void loqui_actions_part_cb(EggAction *action, LoquiApp *app);
static void loqui_actions_set_topic_cb(EggAction *action, LoquiApp *app);
static void loqui_actions_nick_cb(EggAction *action, LoquiApp *app);
static void loqui_actions_away_info_cb(EggAction *action, LoquiApp *app);
static void loqui_actions_start_private_talk_cb(EggAction *action, LoquiApp *app);

static EggActionGroupEntry loqui_action_entries[] =
{
	/* name, label, stock_id, accelerator, tooltip, callback, user_data, entry_type, extra_data */
	{"StockFileMenu",        N_("_File"),       NULL, NULL, NULL, NULL, NULL},
	{"StockEditMenu",        N_("_Edit"),       NULL, NULL, NULL, G_CALLBACK(loqui_actions_edit_menu_cb), NULL},
	{"StockCommandMenu",     N_("_Command"),    NULL, NULL, NULL, NULL, NULL},
	{"StockViewMenu",        N_("_View"),       NULL, NULL, NULL, NULL, NULL},
	{"StockBuffersMenu",     N_("_Buffers"),    NULL, NULL, NULL, NULL, NULL},
	{"StockSettingsMenu",    N_("_Settings"),   NULL, NULL, NULL, NULL, NULL},
	{"StockHelpMenu",        N_("_Help"),       NULL, NULL, NULL, NULL, NULL},
	
	{"StockModeMenu",        N_("Change user mode [Mode]"),    NULL, NULL, NULL, NULL, NULL},
	{"StockCTCPMenu",        N_("CTCP"),        NULL, NULL, NULL, NULL, NULL},

	{"FakeToplevel",         (""),               NULL, NULL, NULL, NULL, NULL },

        {"Quit",                  N_("_Quit"), GTK_STOCK_QUIT, CTRL"Q", NULL, G_CALLBACK(loqui_actions_quit_cb), NULL},

        {"Connect",               N_("_Connect"), NULL, NULL, NULL, G_CALLBACK(loqui_actions_connect_cb), NULL},

        {"Cut",                   N_("_Cut"), GTK_STOCK_CUT, CTRL "X", NULL, G_CALLBACK(loqui_actions_cut_cb), NULL},
        {"Copy",                  N_("_Copy"), GTK_STOCK_COPY, CTRL "C", NULL, G_CALLBACK(loqui_actions_copy_cb), NULL},
        {"Paste",                 N_("_Paste"), GTK_STOCK_PASTE, CTRL "P", NULL, G_CALLBACK(loqui_actions_paste_cb), NULL},
        {"PasteWithLinefeedsCut", N_("_Paste with Linefeeds Cut"), GTK_STOCK_PASTE, NULL, NULL, NULL, NULL},
        {"Clear",                 N_("_Clear"), GTK_STOCK_CLEAR, NULL, NULL, G_CALLBACK(loqui_actions_clear_cb), NULL},
        {"ToggleCommandMode",     N_("_Toggle Command Mode"), LOQUI_STOCK_COMMAND, CTRL"slash", NULL, G_CALLBACK(loqui_actions_toggle_command_mode_cb), NULL, TOGGLE_ACTION},
        {"Find",                  N_("_Find"), GTK_STOCK_FIND, CTRL"F", NULL, NULL, NULL},
        {"FindAgain",             N_("_Find Again"), GTK_STOCK_FIND, NULL, NULL, NULL, NULL},

	{"Join",                  N_("_Join"), NULL, ALT "J", NULL, G_CALLBACK(loqui_actions_join_cb), NULL},
        {"StartPrivateTalk",      N_("_Start Private Talk"), NULL, NULL, NULL, G_CALLBACK(loqui_actions_start_private_talk_cb), NULL},
        {"Part",                  N_("_Part"), NULL, NULL, NULL, G_CALLBACK(loqui_actions_part_cb), NULL},
        {"SetTopic",              N_("Set _Topic"), NULL, ALT "T", NULL, G_CALLBACK(loqui_actions_set_topic_cb), NULL},
        {"ChangeNick",            N_("_Change Nickname"), NULL, CTRL ALT "N", NULL, G_CALLBACK(loqui_actions_nick_cb), NULL},
        {"RefreshAway",           N_("_Refresh users' away information of current channel"), NULL, CTRL ALT "A", NULL, G_CALLBACK(loqui_actions_away_info_cb), NULL},

        {"ToggleChannelbar",       N_("_Channelbar"), NULL, NULL, NULL, G_CALLBACK(loqui_actions_toggle_channelbar_cb), NULL, TOGGLE_ACTION},
        {"ToggleStatusbar",        N_("_Statusbar"), NULL, NULL, NULL, G_CALLBACK(loqui_actions_toggle_statusbar_cb), NULL, TOGGLE_ACTION},
        {"PreviousUpdatedChannel", N_("_Previous Updated Channel"), GTK_STOCK_GOTO_TOP, NULL, NULL, G_CALLBACK(loqui_actions_previous_updated_channel_cb), NULL},
        {"NextUpdatedChannel",     N_("_Next Updated Channel"), GTK_STOCK_GOTO_BOTTOM, NULL, NULL, G_CALLBACK(loqui_actions_next_updated_channel_cb), NULL},
        {"PreviousChannel",        N_("Previous Channel"), GTK_STOCK_GO_UP, CTRL "P", NULL, G_CALLBACK(loqui_actions_previous_channel_cb), NULL},
        {"NextChannel",            N_("Next Channel"), GTK_STOCK_GO_DOWN, CTRL "N", NULL, G_CALLBACK(loqui_actions_next_channel_cb), NULL},
        {"GeneralSettings",        N_("_General Settings"), NULL, NULL, NULL, G_CALLBACK(loqui_actions_common_settings_cb), NULL},
        {"AccountSettings",        N_("_Account Settings"), NULL, NULL, NULL, G_CALLBACK(loqui_actions_account_settings_cb), NULL},

        {"About",                  N_("_About"), NULL, NULL, NULL, G_CALLBACK(loqui_actions_about_cb), NULL},

        {"StartPrivateTalkSelected", N_("Start private talk"), NULL, NULL, NULL, NULL, NULL},
        {"WhoisSelected",            N_("Show information [Whois]"), NULL, NULL, NULL, NULL, NULL},
        {"GiveOpSelected",           N_("Give Channel Operator Privilege (+o)"), LOQUI_STOCK_OPERATOR, NULL, NULL, NULL, NULL},
        {"GiveVoiceSelected",        N_("Give Voice Privilege (+v)"), LOQUI_STOCK_SPEAK_ABILITY, NULL, NULL, NULL, NULL},
        {"DepriveOpSelected",        N_("Deprive Channel Operator Privilege (-o)"), NULL, NULL, NULL, NULL, NULL},
        {"DepriveVoiceSelected",     N_("Deprive Voice Privilege (-v)"), NULL, NULL, NULL, NULL, NULL},

        {"CTCPVersionSelected",      N_("_Version"), NULL, NULL, NULL, NULL, NULL},
        {"CTCPClientinfoSelected",   N_("_Clientinfo"), NULL, NULL, NULL, NULL, NULL},
        {"CTCPUserinfoSelected",     N_("User_info"), NULL, NULL, NULL, NULL, NULL},
        {"CTCPPingSelected",         N_("_Ping"), NULL, NULL, NULL, NULL, NULL},
        {"CTCPTimeSelected",         N_("_Time"), NULL, NULL, NULL, NULL, NULL},
        {"CTCPFingerSelected",       N_("_Finger"), NULL, NULL, NULL, NULL, NULL},
};

EggActionGroup *
loqui_actions_create_group(LoquiApp *app, GtkAccelGroup *accel_group)
{
	EggActionGroup *action_group;
        gint i;

	for (i = 0; i < G_N_ELEMENTS(loqui_action_entries); i++)
                loqui_action_entries[i].user_data = app;

        action_group = egg_action_group_new("LoquiApp");
	if (accel_group)
                egg_action_group_set_accel_group(action_group, accel_group);
	
	egg_action_group_add_actions(action_group,
				     loqui_action_entries,
				     G_N_ELEMENTS(loqui_action_entries));
	
	ACTION_GROUP_ACTION_SET_SENSITIVE(action_group, "PasteWithLinefeedsCut", FALSE);
	ACTION_GROUP_ACTION_SET_SENSITIVE(action_group, "Find", FALSE);
	ACTION_GROUP_ACTION_SET_SENSITIVE(action_group, "FindAgain", FALSE);

	return action_group;
}

/* callbacks */
static void
loqui_actions_edit_menu_cb(EggAction *action, LoquiApp *app)
{
	gboolean cutable, copiable, pastable, clearable, findable;

	loqui_app_get_current_widget_editing_status(app, &cutable, &copiable, &pastable, &clearable, &findable);

	ACTION_GROUP_ACTION_SET_SENSITIVE(app->action_group, "Cut", cutable);
	ACTION_GROUP_ACTION_SET_SENSITIVE(app->action_group, "Paste", pastable);
	ACTION_GROUP_ACTION_SET_SENSITIVE(app->action_group, "Copy", copiable);
	ACTION_GROUP_ACTION_SET_SENSITIVE(app->action_group, "Clear", clearable);
}
static void
loqui_actions_about_cb(EggAction *action, LoquiApp *app)
{
	about_open();
}
static void
loqui_actions_common_settings_cb(EggAction *action, LoquiApp *app)
{
	account_manager_open_prefs_dialog(account_manager_get());
}
static void
loqui_actions_account_settings_cb(EggAction *action, LoquiApp *app)
{
	account_manager_open_account_list_dialog(account_manager_get());
}
static void
loqui_actions_quit_cb(EggAction *action, LoquiApp *app)
{
	gtk_main_quit();
}
static void
loqui_actions_connect_cb(EggAction *action, LoquiApp *app)
{
	account_manager_open_connect_dialog(account_manager_get());
}
static void
loqui_actions_cut_cb(EggAction *action, LoquiApp *app)
{
	loqui_app_current_widget_cut(app);
}
static void
loqui_actions_copy_cb(EggAction *action, LoquiApp *app)
{
	loqui_app_current_widget_copy(app);
}
static void
loqui_actions_paste_cb(EggAction *action, LoquiApp *app)
{
	loqui_app_current_widget_paste(app);
}
static void
loqui_actions_clear_cb(EggAction *action, LoquiApp *app)
{
	loqui_app_current_widget_clear(app);
}
static void
loqui_actions_toggle_channelbar_cb(EggAction *action, LoquiApp *app)
{
	loqui_app_set_show_channelbar(app, egg_toggle_action_get_active(EGG_TOGGLE_ACTION(action)));
}
static void
loqui_actions_toggle_statusbar_cb(EggAction *action, LoquiApp *app)
{
	loqui_app_set_show_statusbar(app, egg_toggle_action_get_active(EGG_TOGGLE_ACTION(action)));
}
static void
loqui_actions_toggle_command_mode_cb(EggAction *action, LoquiApp *app)
{
        remark_entry_set_command_mode(REMARK_ENTRY(app->remark_entry), egg_toggle_action_get_active(EGG_TOGGLE_ACTION(action)));
}
static void
loqui_actions_previous_updated_channel_cb(EggAction *action, LoquiApp *app)
{
	channel_tree_select_prev_channel(app->channel_tree, TRUE);
}
static void
loqui_actions_next_updated_channel_cb(EggAction *action, LoquiApp *app)
{
	channel_tree_select_next_channel(app->channel_tree, TRUE);	
}
static void
loqui_actions_previous_channel_cb(EggAction *action, LoquiApp *app)
{
	channel_tree_select_prev_channel(app->channel_tree, FALSE);
}
static void
loqui_actions_next_channel_cb(EggAction *action, LoquiApp *app)
{
	channel_tree_select_next_channel(app->channel_tree, FALSE);
}
static void
loqui_actions_join_cb(EggAction *action, LoquiApp *app)
{
	command_dialog_join(GTK_WINDOW(app),
			    account_manager_get_current_account(account_manager_get()));
}
static void
loqui_actions_part_cb(EggAction *action, LoquiApp *app)
{
	AccountManager *manager;

	manager = account_manager_get();
	command_dialog_part(GTK_WINDOW(app),
			    account_manager_get_current_account(manager),
			    account_manager_get_current_channel(manager));
}
static void
loqui_actions_set_topic_cb(EggAction *action, LoquiApp *app)
{
	AccountManager *manager;

	manager = account_manager_get();
	command_dialog_topic(GTK_WINDOW(app),
			     account_manager_get_current_account(manager),
			     account_manager_get_current_channel(manager));
}
static void
loqui_actions_nick_cb(EggAction *action, LoquiApp *app)
{
	AccountManager *manager;

	manager = account_manager_get();
	command_dialog_nick(GTK_WINDOW(app),
			    account_manager_get_current_account(manager));
}
static void
loqui_actions_away_info_cb(EggAction *action, LoquiApp *app)
{
	AccountManager *manager;
	Channel *channel;

	manager = account_manager_get();	
	channel = account_manager_get_current_channel(manager);
	if (channel)
		account_fetch_away_information(channel->account, channel);
	else
		gtkutils_msgbox_info(GTK_MESSAGE_ERROR, _("Not selected a channel!"));
}
static void
loqui_actions_start_private_talk_cb(EggAction *action, LoquiApp *app)
{
	command_dialog_private_talk(GTK_WINDOW(app),
				    account_manager_get_current_account(account_manager_get()));
}
