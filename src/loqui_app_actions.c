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
#include "about.h"
#include "remark_entry.h"
#include "command_dialog.h"
#include "gtkutils.h"

#include "prefs_general.h"
#include "prefs_dialog.h"
#include "account_list_dialog.h"

#include <gtk24backports.h>

#define CTRL "<control>"
#define ALT  "<alt>"
#define SHIFT "<shift>"

static void loqui_app_actions_about_cb(GtkAction *action, LoquiApp *app);
static void loqui_app_actions_common_settings_cb(GtkAction *action, LoquiApp *app);
static void loqui_app_actions_account_settings_cb(GtkAction *action, LoquiApp *app);
static void loqui_app_actions_connect_cb(GtkAction *action, LoquiApp *app);
static void loqui_app_actions_quit_cb(GtkAction *action, LoquiApp *app);
static void loqui_app_actions_edit_menu_cb(GtkAction *action, LoquiApp *app);
static void loqui_app_actions_cut_cb(GtkAction *action, LoquiApp *app);
static void loqui_app_actions_copy_cb(GtkAction *action, LoquiApp *app);
static void loqui_app_actions_paste_cb(GtkAction *action, LoquiApp *app);
static void loqui_app_actions_clear_cb(GtkAction *action, LoquiApp *app);
static void loqui_app_actions_previous_updated_channel_cb(GtkAction *action, LoquiApp *app);
static void loqui_app_actions_next_updated_channel_cb(GtkAction *action, LoquiApp *app);
static void loqui_app_actions_previous_channel_cb(GtkAction *action, LoquiApp *app);
static void loqui_app_actions_next_channel_cb(GtkAction *action, LoquiApp *app);
static void loqui_app_actions_join_cb(GtkAction *action, LoquiApp *app);
static void loqui_app_actions_part_cb(GtkAction *action, LoquiApp *app);
static void loqui_app_actions_set_topic_cb(GtkAction *action, LoquiApp *app);
static void loqui_app_actions_nick_cb(GtkAction *action, LoquiApp *app);
static void loqui_app_actions_away_info_cb(GtkAction *action, LoquiApp *app);
static void loqui_app_actions_start_private_talk_cb(GtkAction *action, LoquiApp *app);

static void loqui_app_actions_toggle_channelbar_cb(GtkAction *action, LoquiApp *app);
static void loqui_app_actions_toggle_statusbar_cb(GtkAction *action, LoquiApp *app);
static void loqui_app_actions_toggle_command_mode_cb(GtkAction *action, LoquiApp *app);
static void loqui_app_actions_toggle_scroll_cb(GtkAction *action, LoquiApp *app);

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
	{"StockChannelEntry",    NULL, "",                NULL, NULL},

	{"FakeToplevel",         NULL, (""),               NULL, NULL, NULL},

        {"Quit",                 GTK_STOCK_QUIT, N_("_Quit"), CTRL"Q", NULL, G_CALLBACK(loqui_app_actions_quit_cb)},

        {"Connect",              NULL, N_("_Connect"), NULL, NULL, G_CALLBACK(loqui_app_actions_connect_cb)},

        {"Cut",                   GTK_STOCK_CUT, N_("_Cut"), CTRL "X", NULL, G_CALLBACK(loqui_app_actions_cut_cb)},
        {"Copy",                  GTK_STOCK_COPY, N_("_Copy"), CTRL "C", NULL, G_CALLBACK(loqui_app_actions_copy_cb)},
        {"Paste",                 GTK_STOCK_PASTE, N_("_Paste"), CTRL "P", NULL, G_CALLBACK(loqui_app_actions_paste_cb)},
        {"PasteWithLinefeedsCut", GTK_STOCK_PASTE, N_("_Paste with Linefeeds Cut"), NULL, NULL, NULL},
        {"Clear",                 GTK_STOCK_CLEAR, N_("_Clear"), NULL, NULL, G_CALLBACK(loqui_app_actions_clear_cb)},

        {"Find",                  GTK_STOCK_FIND, N_("_Find"), CTRL"F", NULL, NULL},
        {"FindAgain",             GTK_STOCK_FIND, N_("_Find Again"), NULL, NULL, NULL},

	{"Join",                  NULL, N_("_Join"), ALT "J", NULL, G_CALLBACK(loqui_app_actions_join_cb)},
        {"StartPrivateTalk",      NULL, N_("_Start Private Talk"), NULL, NULL, G_CALLBACK(loqui_app_actions_start_private_talk_cb)},
        {"Part",                  NULL, N_("_Part"), NULL, NULL, G_CALLBACK(loqui_app_actions_part_cb)},
        {"SetTopic",              NULL, N_("Set _Topic"), ALT "T", NULL, G_CALLBACK(loqui_app_actions_set_topic_cb)},
        {"ChangeNick",            NULL, N_("_Change Nickname"), CTRL ALT "N", NULL, G_CALLBACK(loqui_app_actions_nick_cb)},
        {"RefreshAway",           NULL, N_("_Refresh users' away information of current channel"), CTRL ALT "A", NULL, G_CALLBACK(loqui_app_actions_away_info_cb)},

        {"PreviousUpdatedChannel", GTK_STOCK_GOTO_TOP, N_("_Previous Updated Channel"), CTRL SHIFT "space", NULL, G_CALLBACK(loqui_app_actions_previous_updated_channel_cb)},
        {"NextUpdatedChannel",     GTK_STOCK_GOTO_BOTTOM, N_("_Next Updated Channel"), CTRL "space", NULL, G_CALLBACK(loqui_app_actions_next_updated_channel_cb)},
        {"PreviousChannel",        GTK_STOCK_GO_UP, N_("Previous Channel"),CTRL "Up", NULL, G_CALLBACK(loqui_app_actions_previous_channel_cb)},
        {"NextChannel",            GTK_STOCK_GO_DOWN, N_("Next Channel"), CTRL "Down", NULL, G_CALLBACK(loqui_app_actions_next_channel_cb)},
        {"GeneralSettings",        NULL, N_("_General Settings"), NULL, NULL, G_CALLBACK(loqui_app_actions_common_settings_cb)},
        {"AccountSettings",        NULL, N_("_Account Settings"), NULL, NULL, G_CALLBACK(loqui_app_actions_account_settings_cb)},

        {"About",                  NULL, N_("_About"), NULL, NULL, G_CALLBACK(loqui_app_actions_about_cb)},

        {"StartPrivateTalkSelected", NULL, N_("Start private talk"), NULL, NULL, G_CALLBACK(loqui_app_actions_start_private_talk_selected_cb)},
        {"WhoisSelected",            NULL, N_("Show information [Whois]"), NULL, NULL, G_CALLBACK(loqui_app_actions_whois_selected_cb)},
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
	{LOQUI_ACTION_TOGGLE_SCROLL, NULL, N_("_Scroll buffer"), NULL, N_("Whether scrolling the channel buffer to the end when new message arrived."),G_CALLBACK(loqui_app_actions_toggle_scroll_cb) , TRUE},
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
	prefs_dialog_open(GTK_WINDOW(app));
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

	is_active = gtk_toggle_action_get_active(GTK_TOGGLE_ACTION(action));
	if(is_active)
		loqui_app_scroll_channel_buffer(app);

	app->is_scroll = is_active;
}
static void
loqui_app_actions_toggle_command_mode_cb(GtkAction *action, LoquiApp *app)
{
        remark_entry_set_command_mode(REMARK_ENTRY(app->remark_entry), gtk_toggle_action_get_active(GTK_TOGGLE_ACTION(action)));
}
static void
loqui_app_actions_previous_updated_channel_cb(GtkAction *action, LoquiApp *app)
{
	channel_tree_select_prev_channel(app->channel_tree, TRUE);
}
static void
loqui_app_actions_next_updated_channel_cb(GtkAction *action, LoquiApp *app)
{
	channel_tree_select_next_channel(app->channel_tree, TRUE);	
}
static void
loqui_app_actions_previous_channel_cb(GtkAction *action, LoquiApp *app)
{
	channel_tree_select_prev_channel(app->channel_tree, FALSE);
}
static void
loqui_app_actions_next_channel_cb(GtkAction *action, LoquiApp *app)
{
	channel_tree_select_next_channel(app->channel_tree, FALSE);
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
loqui_app_actions_away_info_cb(GtkAction *action, LoquiApp *app)
{
	Channel *channel;

	channel = loqui_app_get_current_channel(app);
	if (channel)
		account_fetch_away_information(channel->account, channel);
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
