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
#include "loqui_app_actions.h"
#include "loqui_stock.h"
#include "intl.h"

#include "account_manager.h"
#include "loqui_account_manager_iter.h"
#include "about.h"
#include "remark_entry.h"
#include "command_dialog.h"
#include "gtkutils.h"

#include "prefs_general.h"
#include "prefs_dialog.h"
#include "account_list_dialog.h"

#include "loqui_channel_text_view.h"

#include <gtk24backports.h>

#define CTRL "<control>"
#define ALT  "<alt>"
#define SHIFT "<shift>"

static void loqui_app_actions_about_cb(GtkAction *action, LoquiApp *app);
static void loqui_app_actions_common_settings_cb(GtkAction *action, LoquiApp *app);
static void loqui_app_actions_account_settings_cb(GtkAction *action, LoquiApp *app);
static void loqui_app_actions_connect_cb(GtkAction *action, LoquiApp *app);

static void loqui_app_actions_connect_current_account_cb(GtkAction *action, LoquiApp *app);
static void loqui_app_actions_disconnect_current_account_cb(GtkAction *action, LoquiApp *app);
static void loqui_app_actions_reconnect_current_account_cb(GtkAction *action, LoquiApp *app);

static void loqui_app_actions_quit_cb(GtkAction *action, LoquiApp *app);
static void loqui_app_actions_edit_menu_cb(GtkAction *action, LoquiApp *app);
static void loqui_app_actions_cut_cb(GtkAction *action, LoquiApp *app);
static void loqui_app_actions_copy_cb(GtkAction *action, LoquiApp *app);
static void loqui_app_actions_paste_cb(GtkAction *action, LoquiApp *app);
static void loqui_app_actions_clear_cb(GtkAction *action, LoquiApp *app);

static void loqui_app_actions_jump_to_previous_keyword_cb(GtkAction *action, LoquiApp *app);
static void loqui_app_actions_jump_to_next_keyword_cb(GtkAction *action, LoquiApp *app);

static void loqui_app_actions_previous_unread_channel_buffer_cb(GtkAction *action, LoquiApp *app);
static void loqui_app_actions_next_unread_channel_buffer_cb(GtkAction *action, LoquiApp *app);
static void loqui_app_actions_previous_channel_buffer_cb(GtkAction *action, LoquiApp *app);
static void loqui_app_actions_next_channel_buffer_cb(GtkAction *action, LoquiApp *app);
static void loqui_app_actions_clear_all_unread_flags_cb(GtkAction *action, LoquiApp *app);

static void loqui_app_actions_join_cb(GtkAction *action, LoquiApp *app);
static void loqui_app_actions_part_cb(GtkAction *action, LoquiApp *app);
static void loqui_app_actions_set_topic_cb(GtkAction *action, LoquiApp *app);
static void loqui_app_actions_nick_cb(GtkAction *action, LoquiApp *app);
static void loqui_app_actions_refresh_cb(GtkAction *action, LoquiApp *app);
static void loqui_app_actions_start_private_talk_cb(GtkAction *action, LoquiApp *app);
static void loqui_app_actions_end_private_talk_cb(GtkAction *action, LoquiApp *app);

static void loqui_app_actions_toggle_channelbar_cb(GtkAction *action, LoquiApp *app);
static void loqui_app_actions_toggle_statusbar_cb(GtkAction *action, LoquiApp *app);
static void loqui_app_actions_toggle_command_mode_cb(GtkAction *action, LoquiApp *app);
static void loqui_app_actions_toggle_scroll_cb(GtkAction *action, LoquiApp *app);
static void loqui_app_actions_toggle_scroll_common_buffer_cb(GtkAction *action, LoquiApp *app);

static void loqui_app_actions_give_op_selected_cb(GtkAction *action, LoquiApp *app);
static void loqui_app_actions_give_voice_selected_cb(GtkAction *action, LoquiApp *app);
static void loqui_app_actions_deprive_op_selected_cb(GtkAction *action, LoquiApp *app);
static void loqui_app_actions_deprive_voice_selected_cb(GtkAction *action, LoquiApp *app);
static void loqui_app_actions_start_private_talk_selected_cb(GtkAction *action, LoquiApp *app);
static void loqui_app_actions_whois_selected_cb(GtkAction *action, LoquiApp *app);
static void loqui_app_actions_ctcp_version_selected_cb(GtkAction *action, LoquiApp *app);
static void loqui_app_actions_ctcp_clientinfo_selected_cb(GtkAction *action, LoquiApp *app);
static void loqui_app_actions_ctcp_userinfo_selected_cb(GtkAction *action, LoquiApp *app);
static void loqui_app_actions_ctcp_ping_selected_cb(GtkAction *action, LoquiApp *app);
static void loqui_app_actions_ctcp_finger_selected_cb(GtkAction *action, LoquiApp *app);
static void loqui_app_actions_ctcp_time_selected_cb(GtkAction *action, LoquiApp *app);

static void loqui_app_actions_nick_list_sort_radio_cb(GtkAction *action, GtkRadioAction *current_action, LoquiApp *app);

static GtkActionEntry loqui_action_entries[] =
{
	/* name, stock_id, label, accelerator, tooltip, callback */
	/* see gdkkeynames.c to find key name */
	{"StockFileMenu",        NULL, N_("_File"),       NULL, NULL},
	{"StockEditMenu",        NULL, N_("_Edit"),       NULL, NULL, G_CALLBACK(loqui_app_actions_edit_menu_cb)},
	{"StockAccountMenu",     NULL, N_("_Account"),    NULL, NULL},
	{"StockChannelMenu",     NULL, N_("_Channel"),    NULL, NULL},
	{"StockUserMenu",        NULL, N_("_User"),       NULL, NULL},
	{"StockViewMenu",        NULL, N_("_View"),       NULL, NULL},
	{"StockBuffersMenu",     NULL, N_("_Buffers"),    NULL, NULL},
	{"StockSettingsMenu",    NULL, N_("_Settings"),   NULL, NULL},
	{"StockHelpMenu",        NULL, N_("_Help"),       NULL, NULL},
	
	{"StockModeMenu",        NULL, N_("Change user mode [Mode]"), NULL, NULL},
	{"StockCTCPMenu",        NULL, N_("CTCP"),        NULL, NULL},
	{"StockNickListSortTypeMenu",  NULL, N_("Sort Type of NickList"),        NULL, NULL},
	{"StockChannelEntry",    NULL, "",                NULL, NULL},

	{"StockTrayIconPopupBuffers", NULL, N_("Buffers"), NULL, NULL},

	{"FakeToplevel",         NULL, (""),               NULL, NULL, NULL},

        {"Quit",                 GTK_STOCK_QUIT, N_("_Quit"), CTRL"Q", NULL, G_CALLBACK(loqui_app_actions_quit_cb)},

        {"Connect",              NULL, N_("_Connect"), NULL, NULL, G_CALLBACK(loqui_app_actions_connect_cb)},

	{LOQUI_ACTION_CONNECT_CURRENT_ACCOUNT, NULL, N_("_Connect Current Account"), NULL, NULL, G_CALLBACK(loqui_app_actions_connect_current_account_cb)},
	{LOQUI_ACTION_RECONNECT_CURRENT_ACCOUNT, NULL, N_("_Reconnect Current Account"), NULL, NULL, G_CALLBACK(loqui_app_actions_reconnect_current_account_cb)},
	{LOQUI_ACTION_DISCONNECT_CURRENT_ACCOUNT, NULL, N_("_Disconnect Current Account"), NULL, NULL, G_CALLBACK(loqui_app_actions_disconnect_current_account_cb)},

        {"Cut",                   GTK_STOCK_CUT, N_("_Cut"), CTRL "X", NULL, G_CALLBACK(loqui_app_actions_cut_cb)},
        {"Copy",                  GTK_STOCK_COPY, N_("_Copy"), CTRL "C", NULL, G_CALLBACK(loqui_app_actions_copy_cb)},
        {"Paste",                 GTK_STOCK_PASTE, N_("_Paste"), CTRL "P", NULL, G_CALLBACK(loqui_app_actions_paste_cb)},
        {"PasteWithLinefeedsCut", GTK_STOCK_PASTE, N_("_Paste with Linefeeds Cut"), NULL, NULL, NULL},
        {"Clear",                 GTK_STOCK_CLEAR, N_("_Clear"), NULL, NULL, G_CALLBACK(loqui_app_actions_clear_cb)},

        {"Find",                  NULL, N_("_Find"), NULL, NULL, NULL},
        {"FindAgain",             NULL, N_("_Find Again"), NULL, NULL, NULL},
	{"JumpToPreviousKeyword", NULL, N_("Jump to Previous Keyword"), NULL, NULL, G_CALLBACK(loqui_app_actions_jump_to_previous_keyword_cb)},
	{"JumpToNextKeyword",     NULL, N_("Jump to Next Keyword"), NULL, NULL, G_CALLBACK(loqui_app_actions_jump_to_next_keyword_cb)},

	{LOQUI_ACTION_JOIN,       GTK_STOCK_ADD, N_("_Join a Channel"), ALT "J", NULL, G_CALLBACK(loqui_app_actions_join_cb)},
        {LOQUI_ACTION_PART,       GTK_STOCK_REMOVE, N_("_Part Current Channel"), NULL, NULL, G_CALLBACK(loqui_app_actions_part_cb)},
        {LOQUI_ACTION_SET_TOPIC,  NULL, N_("Set _Topic of Current Channel"), ALT "T", NULL, G_CALLBACK(loqui_app_actions_set_topic_cb)},
        {LOQUI_ACTION_CHANGE_NICK,NULL, N_("_Change Nickname"), CTRL ALT "N", NULL, G_CALLBACK(loqui_app_actions_nick_cb)},
        {LOQUI_ACTION_REFRESH,NULL, N_("_Refresh Information of Current Channel"), CTRL "R", NULL, G_CALLBACK(loqui_app_actions_refresh_cb)},

        {LOQUI_ACTION_START_PRIVATE_TALK, NULL, N_("_Start Private Talk"), NULL, NULL, G_CALLBACK(loqui_app_actions_start_private_talk_cb)},
	{LOQUI_ACTION_END_PRIVATE_TALK, NULL, N_("_End Current Private Talk"), NULL, NULL, G_CALLBACK(loqui_app_actions_end_private_talk_cb)},

        {"PreviousUnreadChannel", GTK_STOCK_GOTO_TOP, N_("_Previous Unread Channel Buffer"), ALT "Up", NULL, G_CALLBACK(loqui_app_actions_previous_unread_channel_buffer_cb)},
        {"NextUnreadChannel",     GTK_STOCK_GOTO_BOTTOM, N_("_Next Unread Channel Buffer"), ALT "Down", NULL, G_CALLBACK(loqui_app_actions_next_unread_channel_buffer_cb)},
        {"PreviousChannel",        GTK_STOCK_GO_UP, N_("Previous Channel Buffer"), CTRL "Up", NULL, G_CALLBACK(loqui_app_actions_previous_channel_buffer_cb)},
        {"NextChannel",            GTK_STOCK_GO_DOWN, N_("Next Channel Buffer"), CTRL "Down", NULL, G_CALLBACK(loqui_app_actions_next_channel_buffer_cb)},
        {"ClearAllUnreadFlags",    GTK_STOCK_CLEAR, N_("Clear All Unread Flags of Buffers"), NULL, NULL, G_CALLBACK(loqui_app_actions_clear_all_unread_flags_cb)},
        {"GeneralSettings",        NULL, N_("_General Settings"), NULL, NULL, G_CALLBACK(loqui_app_actions_common_settings_cb)},
        {"AccountSettings",        NULL, N_("_Account Settings"), NULL, NULL, G_CALLBACK(loqui_app_actions_account_settings_cb)},

        {"About",                  NULL, N_("_About"), NULL, NULL, G_CALLBACK(loqui_app_actions_about_cb)},

        {"StartPrivateTalkSelected", NULL, N_("Start private talk"), NULL, NULL, G_CALLBACK(loqui_app_actions_start_private_talk_selected_cb)},
        {"WhoisSelected",            NULL, N_("Show Information [Whois]"), NULL, NULL, G_CALLBACK(loqui_app_actions_whois_selected_cb)},
        {"GiveOpSelected",           LOQUI_STOCK_OPERATOR, N_("Give Channel Operator Privilege (+o)"), NULL, NULL, G_CALLBACK(loqui_app_actions_give_op_selected_cb)},
        {"GiveVoiceSelected",        LOQUI_STOCK_SPEAK_ABILITY, N_("Give Voice Privilege (+v)"), NULL, NULL, G_CALLBACK(loqui_app_actions_give_voice_selected_cb)},
        {"DepriveOpSelected",        NULL, N_("Deprive Channel Operator Privilege (-o)"), NULL, NULL, G_CALLBACK(loqui_app_actions_deprive_op_selected_cb)},
        {"DepriveVoiceSelected",     NULL, N_("Deprive Voice Privilege (-v)"), NULL, NULL, G_CALLBACK(loqui_app_actions_deprive_voice_selected_cb)},

        {"CTCPVersionSelected",      NULL, N_("_Version"), NULL, NULL, G_CALLBACK(loqui_app_actions_ctcp_version_selected_cb)},
        {"CTCPClientinfoSelected",   NULL, N_("_Clientinfo"), NULL, NULL, G_CALLBACK(loqui_app_actions_ctcp_clientinfo_selected_cb)},
        {"CTCPUserinfoSelected",     NULL, N_("User_info"), NULL, NULL, G_CALLBACK(loqui_app_actions_ctcp_userinfo_selected_cb)},
        {"CTCPPingSelected",         NULL, N_("_Ping"), NULL, NULL, G_CALLBACK(loqui_app_actions_ctcp_ping_selected_cb)},
        {"CTCPTimeSelected",         NULL, N_("_Time"), NULL, NULL, G_CALLBACK(loqui_app_actions_ctcp_time_selected_cb)},
        {"CTCPFingerSelected",       NULL, N_("_Finger"), NULL, NULL, G_CALLBACK(loqui_app_actions_ctcp_finger_selected_cb)},
};
static GtkToggleActionEntry loqui_toggle_action_entries[] = {
        {"ToggleCommandMode", LOQUI_STOCK_COMMAND, N_("_Toggle Command Mode"),CTRL"slash", N_("Interpret and send the message as command if toggled"), G_CALLBACK(loqui_app_actions_toggle_command_mode_cb), FALSE},
        {"ToggleChannelbar",  NULL, N_("_Channelbar"), NULL, NULL, G_CALLBACK(loqui_app_actions_toggle_channelbar_cb), TRUE},
        {"ToggleStatusbar",   NULL, N_("_Statusbar"), NULL, NULL, G_CALLBACK(loqui_app_actions_toggle_statusbar_cb), TRUE},
	{LOQUI_ACTION_TOGGLE_SCROLL, NULL, N_("_Scroll channel buffer"), NULL, N_("Whether scrolling the channel buffer to the end when new message arrived."),G_CALLBACK(loqui_app_actions_toggle_scroll_cb) , TRUE},
	{LOQUI_ACTION_TOGGLE_SCROLL_COMMON_BUFFER, NULL, N_("_Scroll common buffer"), NULL, N_("Whether scrolling the common buffer to the end when new message arrived."),G_CALLBACK(loqui_app_actions_toggle_scroll_common_buffer_cb) , TRUE},
};
static GtkRadioActionEntry loqui_nick_list_sort_radio_action_entries[] = {
	{"RadioNickListSortNone", NULL, "None", NULL, NULL, PREF_SORT_NONE},
	{"RadioNickListSortNick", NULL, "Nick", NULL, NULL, PREF_SORT_NICK},
	{"RadioNickListSortPowerNick", NULL, "Power > Nick", NULL, NULL, PREF_SORT_POWER_NICK},
	{"RadioNickListSortAwayNick", NULL, "Away > Nick", NULL, NULL, PREF_SORT_AWAY_NICK},
	{"RadioNickListSortPowerAwayNick", NULL, "Power > Away > Nick", NULL, NULL, PREF_SORT_POWER_AWAY_NICK},
	{"RadioNickListSortAwayPowerNick", NULL, "Away > Power > Nick", NULL, NULL, PREF_SORT_AWAY_POWER_NICK},
	{"RadioNickListSortTimeNick", NULL, "Time > Nick", NULL, NULL, PREF_SORT_TIME_NICK},
	{"RadioNickListSortTimeAwayPowerNick", NULL, "Time > Away > Power > Nick", NULL, NULL, PREF_SORT_TIME_AWAY_POWER_NICK},
	{"RadioNickListSortTimePowerAwayNick", NULL, "Time > Power > Away > Nick", NULL, NULL, PREF_SORT_TIME_POWER_AWAY_NICK},
};

GtkActionGroup *
loqui_app_actions_create_group(LoquiApp *app)
{
	GtkActionGroup *action_group;

        action_group = gtk_action_group_new("LoquiApp");
	gtk_action_group_set_translate_func(action_group, gtkutils_menu_translate, NULL, NULL);

	gtk_action_group_add_actions(action_group,
				     loqui_action_entries,
				     G_N_ELEMENTS(loqui_action_entries),
				     app);
	gtk_action_group_add_toggle_actions(action_group,
					    loqui_toggle_action_entries,
					    G_N_ELEMENTS(loqui_toggle_action_entries),
					    app);
	gtk_action_group_add_radio_actions(action_group,
					   loqui_nick_list_sort_radio_action_entries,
					   G_N_ELEMENTS(loqui_nick_list_sort_radio_action_entries),
					   prefs_general.nick_list_sort_type,
					   G_CALLBACK(loqui_app_actions_nick_list_sort_radio_cb),
					   app);
	ACTION_GROUP_ACTION_SET_SENSITIVE(action_group, "PasteWithLinefeedsCut", FALSE);
	ACTION_GROUP_ACTION_SET_SENSITIVE(action_group, "Find", FALSE);
	ACTION_GROUP_ACTION_SET_SENSITIVE(action_group, "FindAgain", FALSE);

	return action_group;
}

void
loqui_app_actions_toggle_action_set_active(LoquiApp *app, const gchar *name, gboolean is_active)
{
	GtkToggleAction *toggle_action;

	toggle_action = GTK_TOGGLE_ACTION(gtk_action_group_get_action(app->action_group, name));
	gtk_toggle_action_set_active(toggle_action, is_active);
}
void
loqui_app_actions_update_sensitivity_related_channel(LoquiApp *app)
{
	LoquiChannelEntry *chent;
	LoquiAccount *account;
	LoquiChannel *channel;
	gboolean is_connected, is_joined, is_private_talk;

	chent = loqui_app_get_current_channel_entry(app);

	if (chent == NULL) {
		ACTION_GROUP_ACTION_SET_SENSITIVE(app->action_group, LOQUI_ACTION_JOIN, FALSE);
		ACTION_GROUP_ACTION_SET_SENSITIVE(app->action_group, LOQUI_ACTION_PART, FALSE);
		ACTION_GROUP_ACTION_SET_SENSITIVE(app->action_group, LOQUI_ACTION_SET_TOPIC, FALSE);
		ACTION_GROUP_ACTION_SET_SENSITIVE(app->action_group, LOQUI_ACTION_CHANGE_NICK, FALSE);
		ACTION_GROUP_ACTION_SET_SENSITIVE(app->action_group, LOQUI_ACTION_REFRESH, FALSE);
		ACTION_GROUP_ACTION_SET_SENSITIVE(app->action_group, LOQUI_ACTION_START_PRIVATE_TALK, FALSE);
		
		ACTION_GROUP_ACTION_SET_SENSITIVE(app->action_group, LOQUI_ACTION_CONNECT_CURRENT_ACCOUNT, FALSE);
		ACTION_GROUP_ACTION_SET_SENSITIVE(app->action_group, LOQUI_ACTION_RECONNECT_CURRENT_ACCOUNT, FALSE);
		ACTION_GROUP_ACTION_SET_SENSITIVE(app->action_group, LOQUI_ACTION_DISCONNECT_CURRENT_ACCOUNT, FALSE);
		return;
	}

	account = loqui_app_get_current_account(app);
	is_connected = (account != NULL && loqui_account_is_connected(account));

	ACTION_GROUP_ACTION_SET_SENSITIVE(app->action_group, LOQUI_ACTION_JOIN, is_connected);
	ACTION_GROUP_ACTION_SET_SENSITIVE(app->action_group, LOQUI_ACTION_CHANGE_NICK, is_connected);
	ACTION_GROUP_ACTION_SET_SENSITIVE(app->action_group, LOQUI_ACTION_START_PRIVATE_TALK, is_connected);
	
	ACTION_GROUP_ACTION_SET_SENSITIVE(app->action_group, LOQUI_ACTION_CONNECT_CURRENT_ACCOUNT, !is_connected);
	ACTION_GROUP_ACTION_SET_SENSITIVE(app->action_group, LOQUI_ACTION_RECONNECT_CURRENT_ACCOUNT, is_connected);
	ACTION_GROUP_ACTION_SET_SENSITIVE(app->action_group, LOQUI_ACTION_DISCONNECT_CURRENT_ACCOUNT, is_connected);


	channel = loqui_app_get_current_channel(app);
	is_joined = (channel != NULL && !loqui_channel_get_is_private_talk(channel) && loqui_channel_get_is_joined(channel));

	ACTION_GROUP_ACTION_SET_SENSITIVE(app->action_group, LOQUI_ACTION_SET_TOPIC, is_joined);
	ACTION_GROUP_ACTION_SET_SENSITIVE(app->action_group, LOQUI_ACTION_PART, is_joined);
	ACTION_GROUP_ACTION_SET_SENSITIVE(app->action_group, LOQUI_ACTION_REFRESH, is_joined);

	is_private_talk = (channel != NULL && loqui_channel_get_is_private_talk(channel));
	ACTION_GROUP_ACTION_SET_SENSITIVE(app->action_group, LOQUI_ACTION_END_PRIVATE_TALK, is_private_talk);
}

/* callbacks */
static void
loqui_app_actions_edit_menu_cb(GtkAction *action, LoquiApp *app)
{
	gboolean cutable, copiable, pastable, clearable, findable;

	loqui_app_get_current_widget_editing_status(app, &cutable, &copiable, &pastable, &clearable, &findable);

	ACTION_GROUP_ACTION_SET_SENSITIVE(app->action_group, "Cut", cutable);
	ACTION_GROUP_ACTION_SET_SENSITIVE(app->action_group, "Paste", pastable);
	ACTION_GROUP_ACTION_SET_SENSITIVE(app->action_group, "Copy", copiable);
	ACTION_GROUP_ACTION_SET_SENSITIVE(app->action_group, "Clear", clearable);
}
static void
loqui_app_actions_about_cb(GtkAction *action, LoquiApp *app)
{
	about_open();
}
static void
loqui_app_actions_common_settings_cb(GtkAction *action, LoquiApp *app)
{
	prefs_dialog_open(app);
}
static void
loqui_app_actions_account_settings_cb(GtkAction *action, LoquiApp *app)
{
	account_list_dialog_open(GTK_WINDOW(app), loqui_app_get_account_manager(app));
}
static void
loqui_app_actions_quit_cb(GtkAction *action, LoquiApp *app)
{
	gtk_main_quit();
}
static void
loqui_app_actions_connect_cb(GtkAction *action, LoquiApp *app)
{
	account_list_dialog_open_for_connect(GTK_WINDOW(app), loqui_app_get_account_manager(app));
}
static void
loqui_app_actions_connect_current_account_cb(GtkAction *action, LoquiApp *app)
{
	LoquiAccount *account;

	account = loqui_app_get_current_account(app);
	if (account)
		loqui_account_connect(account);
}
static void
loqui_app_actions_reconnect_current_account_cb(GtkAction *action, LoquiApp *app)
{
	LoquiAccount *account;

	account = loqui_app_get_current_account(app);
	if (account) {
		loqui_account_disconnect(account);
		loqui_account_connect(account);
	}
}
static void
loqui_app_actions_disconnect_current_account_cb(GtkAction *action, LoquiApp *app)
{
	LoquiAccount *account;

	account = loqui_app_get_current_account(app);
	if (account)
		loqui_account_disconnect(account);
}

static void
loqui_app_actions_cut_cb(GtkAction *action, LoquiApp *app)
{
	GtkWidget *widget;
	GtkTextBuffer *buffer;

        g_return_if_fail(app != NULL);
        g_return_if_fail(LOQUI_IS_APP(app));

	widget = gtk_window_get_focus(GTK_WINDOW(app));

	if(GTK_IS_ENTRY(widget)) {
		gtk_editable_cut_clipboard(GTK_EDITABLE(widget));
	} else if(GTK_IS_TEXT_VIEW(widget)) {
		buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
		gtk_text_buffer_cut_clipboard(buffer,
					      gtk_clipboard_get(GDK_NONE),
					      FALSE);
	}
}
static void
loqui_app_actions_copy_cb(GtkAction *action, LoquiApp *app)
{
	GtkWidget *widget;
	GtkTextBuffer *buffer;

        g_return_if_fail(app != NULL);
        g_return_if_fail(LOQUI_IS_APP(app));

	widget = gtk_window_get_focus(GTK_WINDOW(app));

	if(GTK_IS_ENTRY(widget)) {
		gtk_editable_copy_clipboard(GTK_EDITABLE(widget));
	} else if(GTK_IS_TEXT_VIEW(widget)) {
		buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
		gtk_text_buffer_copy_clipboard(buffer,
					       gtk_clipboard_get(GDK_NONE));
	}
}
static void
loqui_app_actions_paste_cb(GtkAction *action, LoquiApp *app)
{
	GtkWidget *widget;
	GtkTextBuffer *buffer;

        g_return_if_fail(app != NULL);
        g_return_if_fail(LOQUI_IS_APP(app));

	widget = gtk_window_get_focus(GTK_WINDOW(app));

	if(GTK_IS_ENTRY(widget)) {
		gtk_editable_paste_clipboard(GTK_EDITABLE(widget));
	} else if(GTK_IS_TEXT_VIEW(widget)) {
		buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
		gtk_text_buffer_paste_clipboard(buffer,
						gtk_clipboard_get(GDK_NONE),
						NULL, FALSE);
	}
}
static void
loqui_app_actions_clear_cb(GtkAction *action, LoquiApp *app)
{
	GtkWidget *widget;
	GtkTextBuffer *buffer;
	GtkTextIter start, end;

        g_return_if_fail(app != NULL);
        g_return_if_fail(LOQUI_IS_APP(app));

	widget = gtk_window_get_focus(GTK_WINDOW(app));

	if(GTK_IS_ENTRY(widget)) {
		gtk_editable_delete_text(GTK_EDITABLE(widget), 0, -1);
	} else if(GTK_IS_TEXT_VIEW(widget)) {
		buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
		gtk_text_buffer_get_start_iter(buffer, &start);
		gtk_text_buffer_get_end_iter(buffer, &end);
		gtk_text_buffer_delete(buffer, &start, &end);
	}
}

static void
loqui_app_actions_jump_to_previous_keyword_cb(GtkAction *action, LoquiApp *app)
{
	LoquiChannelTextView *chview;
	GtkTextBuffer *buffer;
	GtkTextIter iter;
	GtkTextTag *tag;

	chview = LOQUI_CHANNEL_TEXT_VIEW(loqui_app_get_current_channel_text_view(app));
	if (!chview)
		return;
	
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(chview));
	if (!buffer) {
		g_warning("the LoquiChannelTextView doesn't have text buffer");
		return;
	}

	gtk_text_buffer_get_iter_at_mark(buffer,
					 &iter,
					 gtk_text_buffer_get_insert(buffer));
	gtk_text_buffer_place_cursor(buffer, &iter);

	tag = gtk_text_tag_table_lookup(gtk_text_buffer_get_tag_table(buffer), "highlight");
	g_return_if_fail(tag != NULL);

	/* if iter is in highlight tag, exit it at first */
	if (gtk_text_iter_has_tag(&iter, tag) && !gtk_text_iter_begins_tag(&iter, tag))
		gtk_text_iter_backward_to_tag_toggle(&iter, tag);

	/* find the end of a tagged text */
	if (!gtk_text_iter_backward_to_tag_toggle(&iter, tag))
		return;
	/* find the beginning of the tagged text */
	gtk_text_iter_backward_to_tag_toggle(&iter, tag);

	gtk_text_buffer_place_cursor(buffer, &iter);
	gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(chview), &iter,
				     0.25, FALSE, 0.0, 0.0);
}
static void
loqui_app_actions_jump_to_next_keyword_cb(GtkAction *action, LoquiApp *app)
{
	LoquiChannelTextView *chview;
	GtkTextBuffer *buffer;
	GtkTextIter iter;
	GtkTextTag *tag;

	chview = LOQUI_CHANNEL_TEXT_VIEW(loqui_app_get_current_channel_text_view(app));
	if (!chview)
		return;
	
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(chview));
	if (!buffer) {
		g_warning("the LoquiChannelTextView doesn't have text buffer");
		return;
	}

	gtk_text_buffer_get_iter_at_mark(buffer,
					 &iter,
					 gtk_text_buffer_get_insert(buffer));
	gtk_text_buffer_place_cursor(buffer, &iter);

	tag = gtk_text_tag_table_lookup(gtk_text_buffer_get_tag_table(buffer), "highlight");
	g_return_if_fail(tag != NULL);

	/* if iter is in highlight tag, exit it at first */
	if (gtk_text_iter_has_tag(&iter, tag) && !gtk_text_iter_ends_tag(&iter, tag))
		gtk_text_iter_forward_to_tag_toggle(&iter, tag);

	/* find the beginning of a tagged text */
	if (!gtk_text_iter_forward_to_tag_toggle(&iter, tag))
		return;

	gtk_text_buffer_place_cursor(buffer, &iter);
	gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(chview), &iter,
				     0.25, FALSE, 0.0, 0.0);
}

static void
loqui_app_actions_toggle_channelbar_cb(GtkAction *action, LoquiApp *app)
{
	loqui_app_set_show_channelbar(app, gtk_toggle_action_get_active(GTK_TOGGLE_ACTION(action)));
}
static void
loqui_app_actions_toggle_statusbar_cb(GtkAction *action, LoquiApp *app)
{
	loqui_app_set_show_statusbar(app, gtk_toggle_action_get_active(GTK_TOGGLE_ACTION(action)));
}
static void
loqui_app_actions_toggle_scroll_cb(GtkAction *action, LoquiApp *app)
{
	gboolean is_active;
	GtkWidget *view;

	view = loqui_app_get_current_channel_text_view(app);

	if (view) {
		is_active = gtk_toggle_action_get_active(GTK_TOGGLE_ACTION(action));
		loqui_channel_text_view_set_is_scroll(LOQUI_CHANNEL_TEXT_VIEW(view), is_active);
	}
}
static void
loqui_app_actions_toggle_scroll_common_buffer_cb(GtkAction *action, LoquiApp *app)
{
	gboolean is_active;
	GtkWidget *view;

	view = app->common_textview;

	is_active = gtk_toggle_action_get_active(GTK_TOGGLE_ACTION(action));
	loqui_channel_text_view_set_is_scroll(LOQUI_CHANNEL_TEXT_VIEW(view), is_active);
}

static void
loqui_app_actions_toggle_command_mode_cb(GtkAction *action, LoquiApp *app)
{
        remark_entry_set_command_mode(REMARK_ENTRY(app->remark_entry), gtk_toggle_action_get_active(GTK_TOGGLE_ACTION(action)));
}
static void
loqui_app_actions_previous_unread_channel_buffer_cb(GtkAction *action, LoquiApp *app)
{
	LoquiChannelEntry *chent;

	chent = account_manager_get_previous_channel_entry(loqui_app_get_account_manager(app),
							   loqui_app_get_current_channel_entry(app),
							   TRUE);
	if (chent)
		loqui_app_set_current_channel_entry(app, chent);
}
static void
loqui_app_actions_next_unread_channel_buffer_cb(GtkAction *action, LoquiApp *app)
{
	LoquiChannelEntry *chent;

	chent = account_manager_get_next_channel_entry(loqui_app_get_account_manager(app),
						       loqui_app_get_current_channel_entry(app),
						       TRUE);
	if (chent)
		loqui_app_set_current_channel_entry(app, chent);
}
static void
loqui_app_actions_previous_channel_buffer_cb(GtkAction *action, LoquiApp *app)
{
	LoquiChannelEntry *chent;

	chent = account_manager_get_previous_channel_entry(loqui_app_get_account_manager(app),
							   loqui_app_get_current_channel_entry(app),
							   FALSE);
	if (chent)
		loqui_app_set_current_channel_entry(app, chent);
}
static void
loqui_app_actions_next_channel_buffer_cb(GtkAction *action, LoquiApp *app)
{
	LoquiChannelEntry *chent;

	chent = account_manager_get_next_channel_entry(loqui_app_get_account_manager(app),
						       loqui_app_get_current_channel_entry(app),
						       FALSE);
	if (chent)
		loqui_app_set_current_channel_entry(app, chent);
}
static void
loqui_app_actions_clear_all_unread_flags_cb(GtkAction *action, LoquiApp *app)
{
	LoquiChannelEntry *chent;
	LoquiAccountManagerIter iter;

	loqui_account_manager_iter_init(loqui_app_get_account_manager(app), &iter);
	loqui_account_manager_iter_set_first_channel_entry(&iter);
	while ((chent = loqui_account_manager_iter_channel_entry_next(&iter)) != NULL)
		loqui_channel_entry_set_as_read(chent);
}
static void
loqui_app_actions_join_cb(GtkAction *action, LoquiApp *app)
{
	command_dialog_join(app,
			    loqui_app_get_current_account(app));
}
static void
loqui_app_actions_part_cb(GtkAction *action, LoquiApp *app)
{
	command_dialog_part(app,
			    loqui_app_get_current_account(app),
			    loqui_app_get_current_channel(app));
}
static void
loqui_app_actions_set_topic_cb(GtkAction *action, LoquiApp *app)
{
	command_dialog_topic(app,
			     loqui_app_get_current_account(app),
			     loqui_app_get_current_channel(app));
}
static void
loqui_app_actions_nick_cb(GtkAction *action, LoquiApp *app)
{
	command_dialog_nick(app,
			    loqui_app_get_current_account(app));
}
static void
loqui_app_actions_refresh_cb(GtkAction *action, LoquiApp *app)
{
	LoquiChannel *channel;
	
	channel = loqui_app_get_current_channel(app);
	if (channel) {
		if (!loqui_account_is_connected(channel->account)) {
			gtkutils_msgbox_info(GTK_MESSAGE_ERROR, _("The account is not connected"));
			return;
		}
		loqui_sender_refresh(loqui_account_get_sender(channel->account), channel);
	}
	else
		gtkutils_msgbox_info(GTK_MESSAGE_ERROR, _("Not selected a channel!"));
}
static void
loqui_app_actions_start_private_talk_cb(GtkAction *action, LoquiApp *app)
{
	command_dialog_private_talk(app,
				    loqui_app_get_current_account(app));
}
static void
loqui_app_actions_end_private_talk_cb(GtkAction *action, LoquiApp *app)
{
	LoquiChannel *channel;
	channel = loqui_app_get_current_channel(app);
	if (!channel)
		gtkutils_msgbox_info(GTK_MESSAGE_ERROR, _("Channel is not selected"));
	if (!loqui_channel_get_is_private_talk(channel))
		gtkutils_msgbox_info(GTK_MESSAGE_ERROR, _("Current channel is not private talk"));

	loqui_sender_end_private_talk(loqui_account_get_sender(loqui_channel_get_account(channel)), channel);
}
static void
loqui_app_actions_give_op_selected_cb(GtkAction *action, LoquiApp *app)
{
	nick_list_change_mode_selected(app->nick_list, TRUE, IRC_CHANNEL_MODE_OPERATOR);
}
static void
loqui_app_actions_give_voice_selected_cb(GtkAction *action, LoquiApp *app)
{
	nick_list_change_mode_selected(app->nick_list, TRUE, IRC_CHANNEL_MODE_VOICE);
}
static void
loqui_app_actions_deprive_op_selected_cb(GtkAction *action, LoquiApp *app)
{
	nick_list_change_mode_selected(app->nick_list, FALSE, IRC_CHANNEL_MODE_OPERATOR);
}
static void
loqui_app_actions_deprive_voice_selected_cb(GtkAction *action, LoquiApp *app)
{
	nick_list_change_mode_selected(app->nick_list, FALSE, IRC_CHANNEL_MODE_VOICE);
}
static void
loqui_app_actions_start_private_talk_selected_cb(GtkAction *action, LoquiApp *app)
{
	nick_list_start_private_talk_selected(app->nick_list);
}
static void
loqui_app_actions_whois_selected_cb(GtkAction *action, LoquiApp *app)
{
	nick_list_whois_selected(app->nick_list);
}
static void
loqui_app_actions_ctcp_version_selected_cb(GtkAction *action, LoquiApp *app)
{
	nick_list_ctcp_selected(app->nick_list, IRCCTCPVersion);
}
static void
loqui_app_actions_ctcp_clientinfo_selected_cb(GtkAction *action, LoquiApp *app)
{
	nick_list_ctcp_selected(app->nick_list, IRCCTCPClientInfo);
}
static void
loqui_app_actions_ctcp_userinfo_selected_cb(GtkAction *action, LoquiApp *app)
{
	nick_list_ctcp_selected(app->nick_list, IRCCTCPUserInfo);
}
static void
loqui_app_actions_ctcp_ping_selected_cb(GtkAction *action, LoquiApp *app)
{
	nick_list_ctcp_selected(app->nick_list, IRCCTCPPing);
}
static void
loqui_app_actions_ctcp_finger_selected_cb(GtkAction *action, LoquiApp *app)
{
	nick_list_ctcp_selected(app->nick_list, IRCCTCPFinger);
}
static void
loqui_app_actions_ctcp_time_selected_cb(GtkAction *action, LoquiApp *app)
{
	nick_list_ctcp_selected(app->nick_list, IRCCTCPTime);
}
static void
loqui_app_actions_nick_list_sort_radio_cb(GtkAction *action, GtkRadioAction *current_action, LoquiApp *app)
{
	gint value;

	value = gtk_radio_action_get_current_value(GTK_RADIO_ACTION(current_action));
	loqui_app_set_nick_list_sort_type(app, value);
}
