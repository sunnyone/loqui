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

#include "loqui_app_info.h"
#include "loqui_channel_entry_utils.h"
#include "loqui_statusbar.h"
#include "loqui_channelbar.h"
#include "intl.h"
#include "prefs_general.h"
#include <string.h>

enum {
        LAST_SIGNAL
};

enum {
        LAST_PROP
};

struct _LoquiAppInfoPrivate
{
	gboolean is_pending_update_string;
	
	LoquiApp *app;
};

static GObjectClass *parent_class = NULL;

/* static guint loqui_app_info_signals[LAST_SIGNAL] = { 0 }; */

static GObject* loqui_app_info_constructor(GType type, guint n_props, GObjectConstructParam *props);

static void loqui_app_info_class_init(LoquiAppInfoClass *klass);
static void loqui_app_info_init(LoquiAppInfo *appinfo);
static void loqui_app_info_finalize(GObject *object);
static void loqui_app_info_dispose(GObject *object);

static void loqui_app_info_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec);
static void loqui_app_info_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec);

static void loqui_app_info_channel_entry_notify_is_updated_cb(LoquiChannelEntry *chent, GParamSpec *pspec, LoquiAppInfo *appinfo);

GType
loqui_app_info_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(LoquiAppInfoClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) loqui_app_info_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(LoquiAppInfo),
				0,              /* n_preallocs */
				(GInstanceInitFunc) loqui_app_info_init
			};
		
		type = g_type_register_static(G_TYPE_OBJECT,
					      "LoquiAppInfo",
					      &our_info,
					      0);
	}
	
	return type;
}
static GObject*
loqui_app_info_constructor(GType type, guint n_props, GObjectConstructParam *props)
{
        GObject *object;
        GObjectClass *object_class = G_OBJECT_CLASS(parent_class);

	object = object_class->constructor(type, n_props, props);

	return object;
}
static void 
loqui_app_info_finalize(GObject *object)
{
	LoquiAppInfo *appinfo;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_APP_INFO(object));

        appinfo = LOQUI_APP_INFO(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize)(object);

	g_free(appinfo->priv);
}
static void 
loqui_app_info_dispose(GObject *object)
{
	LoquiAppInfo *appinfo;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_APP_INFO(object));

        appinfo = LOQUI_APP_INFO(object);

	G_FREE_UNLESS_NULL(appinfo->cache_account_name);
	G_FREE_UNLESS_NULL(appinfo->cache_channel_name);
	G_FREE_UNLESS_NULL(appinfo->cache_topic);
	G_FREE_UNLESS_NULL(appinfo->cache_member_number);
	G_FREE_UNLESS_NULL(appinfo->cache_op_number);
	G_FREE_UNLESS_NULL(appinfo->cache_channel_mode);
	G_FREE_UNLESS_NULL(appinfo->cache_updated_entry_number);
	G_FREE_UNLESS_NULL(appinfo->cache_updated_private_talk_number);

	if (appinfo->ltf_title) {
		loqui_title_format_free(appinfo->ltf_title);
		appinfo->ltf_title = NULL;
	}
	if (appinfo->ltf_statusbar) {
		loqui_title_format_free(appinfo->ltf_statusbar);
		appinfo->ltf_statusbar = NULL;
	}

        if (G_OBJECT_CLASS(parent_class)->dispose)
                (* G_OBJECT_CLASS(parent_class)->dispose)(object);
}
static void
loqui_app_info_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec)
{
        LoquiAppInfo *appinfo;        

        appinfo = LOQUI_APP_INFO(object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}
static void
loqui_app_info_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec)
{
        LoquiAppInfo *appinfo;        

        appinfo = LOQUI_APP_INFO(object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}

static void
loqui_app_info_class_init(LoquiAppInfoClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);

	object_class->constructor = loqui_app_info_constructor; 
        object_class->finalize = loqui_app_info_finalize;
        object_class->dispose = loqui_app_info_dispose;
        object_class->get_property = loqui_app_info_get_property;
        object_class->set_property = loqui_app_info_set_property;
}
static void 
loqui_app_info_init(LoquiAppInfo *appinfo)
{
	LoquiAppInfoPrivate *priv;

	priv = g_new0(LoquiAppInfoPrivate, 1);

	appinfo->priv = priv;
}
static void
loqui_app_info_channel_entry_notify_is_updated_cb(LoquiChannelEntry *chent, GParamSpec *pspec, LoquiAppInfo *appinfo)
{
	LoquiAppInfoPrivate *priv;
	gboolean updated;
	gint delta;
	
	g_return_if_fail(chent != NULL);
	g_return_if_fail(LOQUI_IS_CHANNEL_ENTRY(chent));

	priv = appinfo->priv;
	
	updated = loqui_channel_entry_get_is_updated(chent);
	
	delta = updated ? +1 : -1;

	appinfo->updated_entry_number += delta;

	if (LOQUI_CHANNEL(chent) && loqui_channel_get_is_private_talk(LOQUI_CHANNEL(chent)))
		appinfo->updated_private_talk_number += delta;
	
	loqui_app_info_update_updated_entry_number(appinfo);
	loqui_app_info_update_updated_private_talk_number(appinfo);

	loqui_app_info_update_string_idle(appinfo);
}
static void
loqui_app_info_channel_entry_notify_member_number_cb(LoquiChannelEntry *chent, GParamSpec *pspec, LoquiAppInfo *appinfo)
{
	loqui_app_info_update_member_number(appinfo, chent);
	loqui_app_info_update_string_idle(appinfo);
}
static void
loqui_app_info_channel_entry_notify_op_number_cb(LoquiChannelEntry *chent, GParamSpec *pspec, LoquiAppInfo *appinfo)
{
	loqui_app_info_update_op_number(appinfo, chent);
	loqui_app_info_update_string_idle(appinfo);
}
static void
loqui_app_info_channel_entry_notify_topic_cb(LoquiChannelEntry *chent, GParamSpec *pspec, LoquiAppInfo *appinfo)
{
	loqui_app_info_update_topic(appinfo, chent);
	loqui_app_info_update_string_idle(appinfo);
}
static void
loqui_app_info_user_self_notify_nick_cb(LoquiUser *user_self, GParamSpec *pspec, LoquiAppInfo *appinfo)
{
	LoquiAccount *account;

	account = g_object_get_data(G_OBJECT(user_self), LOQUI_ACCOUNT_USER_SELF_ACCOUNT_KEY);
	g_return_if_fail(account != NULL);

	loqui_app_info_update_nick(appinfo, account);
	loqui_app_info_update_string_idle(appinfo);
}
static void
loqui_app_info_user_self_notify_away_cb(LoquiUser *user_self, GParamSpec *pspec, LoquiAppInfo *appinfo)
{
	LoquiAccount *account;

	account = g_object_get_data(G_OBJECT(user_self), LOQUI_ACCOUNT_USER_SELF_ACCOUNT_KEY);
	g_return_if_fail(account != NULL);

	loqui_app_info_update_away(appinfo, account);
	loqui_app_info_update_string_idle(appinfo);
}
static void
loqui_app_info_account_notify_name_cb(LoquiAccount *account, GParamSpec *pspec, LoquiAppInfo *appinfo)
{
	loqui_app_info_update_account_name(appinfo, account);
	loqui_app_info_update_string_idle(appinfo);	
}
static void
loqui_app_info_channel_notify_name_cb(LoquiChannel *channel, GParamSpec *pspec, LoquiAppInfo *appinfo)
{
	loqui_app_info_update_channel_name(appinfo, channel);
	loqui_app_info_update_string_idle(appinfo);
}
static void
loqui_app_info_channel_mode_changed_cb(LoquiChannel *channel, LoquiAppInfo *appinfo)
{
	loqui_app_info_update_channel_mode(appinfo, channel);
	loqui_app_info_update_string_idle(appinfo);
}
LoquiAppInfo*
loqui_app_info_new(LoquiApp *app)
{
        LoquiAppInfo *appinfo;
	LoquiAppInfoPrivate *priv;

	appinfo = g_object_new(loqui_app_info_get_type(), NULL);
	
        priv = appinfo->priv;
	priv->app = app;

	loqui_app_info_set_title_format_title(appinfo, NULL);
	loqui_app_info_set_title_format_statusbar(appinfo, NULL);
	
        return appinfo;
}
void
loqui_app_info_update_account_name(LoquiAppInfo *appinfo, LoquiAccount *account)
{
	G_FREE_UNLESS_NULL(appinfo->cache_account_name);
	if (account)
		appinfo->cache_account_name = g_strdup(loqui_profile_account_get_name(loqui_account_get_profile(account)));

	loqui_statusbar_update_account_name(LOQUI_STATUSBAR(appinfo->priv->app->statusbar), account);
}
void
loqui_app_info_update_nick(LoquiAppInfo *appinfo, LoquiAccount *account)
{
	G_FREE_UNLESS_NULL(appinfo->cache_nick);
	if (account)
		appinfo->cache_nick = g_strdup(loqui_user_get_nick(loqui_account_get_user_self(account)));
	
	loqui_statusbar_update_nick(LOQUI_STATUSBAR(appinfo->priv->app->statusbar), account);
}
void
loqui_app_info_update_preset(LoquiAppInfo *appinfo, LoquiAccount *account)
{
	loqui_statusbar_update_preset_menu(LOQUI_STATUSBAR(appinfo->priv->app->statusbar), account);
}
void
loqui_app_info_update_away_list(LoquiAppInfo *appinfo, LoquiAccount *account)
{
	loqui_statusbar_update_away_menu(LOQUI_STATUSBAR(appinfo->priv->app->statusbar), account);
}
void
loqui_app_info_update_away(LoquiAppInfo *appinfo, LoquiAccount *account)
{
	loqui_statusbar_update_away_icon(LOQUI_STATUSBAR(appinfo->priv->app->statusbar), account);
}
void
loqui_app_info_update_channel_name(LoquiAppInfo *appinfo, LoquiChannel *channel)
{
	G_FREE_UNLESS_NULL(appinfo->cache_channel_name);
	if (channel)
		appinfo->cache_channel_name = g_strdup(loqui_channel_entry_get_name(LOQUI_CHANNEL_ENTRY(channel)));
}
void
loqui_app_info_update_topic(LoquiAppInfo *appinfo, LoquiChannelEntry *chent)
{
	G_FREE_UNLESS_NULL(appinfo->cache_topic);
	if (chent)
		appinfo->cache_topic = g_strdup(loqui_channel_entry_get_topic(chent));

	loqui_channelbar_update_topic(LOQUI_CHANNELBAR(appinfo->priv->app->channelbar), chent);
}
void
loqui_app_info_update_member_number(LoquiAppInfo *appinfo, LoquiChannelEntry *chent)
{
	G_FREE_UNLESS_NULL(appinfo->cache_member_number);
	if (chent)
		appinfo->cache_member_number = g_strdup_printf("%d",
							       loqui_channel_entry_get_member_number(chent));

	loqui_channelbar_update_member_number(LOQUI_CHANNELBAR(appinfo->priv->app->channelbar), chent);
}
void
loqui_app_info_update_op_number(LoquiAppInfo *appinfo, LoquiChannelEntry *chent)
{
	G_FREE_UNLESS_NULL(appinfo->cache_op_number);
	if (chent)
		appinfo->cache_op_number = g_strdup_printf("%d",
							   loqui_channel_entry_get_op_number(chent));

	loqui_channelbar_update_op_number(LOQUI_CHANNELBAR(appinfo->priv->app->channelbar), chent);
}
void
loqui_app_info_update_channel_mode(LoquiAppInfo *appinfo, LoquiChannel *channel)
{
	G_FREE_UNLESS_NULL(appinfo->cache_channel_mode);
	if (channel)
		appinfo->cache_channel_mode = loqui_channel_get_mode(channel);

	loqui_channelbar_update_channel_mode(LOQUI_CHANNELBAR(appinfo->priv->app->channelbar), channel);
}
void
loqui_app_info_update_updated_entry_number(LoquiAppInfo *appinfo)
{
	G_FREE_UNLESS_NULL(appinfo->cache_updated_entry_number);
	appinfo->cache_updated_entry_number = g_strdup_printf("%d",
							      appinfo->updated_entry_number);
}
void
loqui_app_info_update_updated_private_talk_number(LoquiAppInfo *appinfo)
{
	G_FREE_UNLESS_NULL(appinfo->cache_updated_private_talk_number);
	appinfo->cache_updated_private_talk_number = g_strdup_printf("%d",
								     appinfo->updated_private_talk_number);
}
void
loqui_app_info_update_all(LoquiAppInfo *appinfo, LoquiChannelEntry *chent)
{
	LoquiAccount *account;
	LoquiChannel *channel;

	loqui_channel_entry_utils_separate(chent, &account, &channel);

	loqui_app_info_update_account_name(appinfo, account);
	loqui_app_info_update_nick(appinfo, account);
	loqui_app_info_update_preset(appinfo, account);
	loqui_app_info_update_away_list(appinfo, account);
	loqui_app_info_update_away(appinfo, account);	

	loqui_app_info_update_channel_name(appinfo, channel);
	loqui_app_info_update_channel_mode(appinfo, channel);

	loqui_app_info_update_topic(appinfo, chent);
	loqui_app_info_update_member_number(appinfo, chent);
	loqui_app_info_update_op_number(appinfo, chent);

	loqui_app_info_update_updated_entry_number(appinfo);
	loqui_app_info_update_updated_private_talk_number(appinfo);

	loqui_channelbar_update_channel_entry_label(LOQUI_CHANNELBAR(appinfo->priv->app->channelbar), chent);

	loqui_app_info_update_string(appinfo);
}
static gboolean
loqui_app_info_update_string_for_idle(LoquiAppInfo *appinfo)
{
	loqui_app_info_update_string(appinfo);

	appinfo->priv->is_pending_update_string = FALSE;
	return FALSE;
}
void
loqui_app_info_update_string_idle(LoquiAppInfo *appinfo)
{
	if (!appinfo->priv->is_pending_update_string) {
		appinfo->priv->is_pending_update_string = TRUE;
		g_idle_add((GSourceFunc) loqui_app_info_update_string_for_idle, appinfo);
	}
}
void
loqui_app_info_update_string(LoquiAppInfo *appinfo)
{
	LoquiAppInfoPrivate *priv;
	gchar *buf;

	g_return_if_fail(appinfo != NULL);
	g_return_if_fail(LOQUI_IS_APP_INFO(appinfo));

	priv = appinfo->priv;

#define SET_VARIABLES(ltf) loqui_title_format_register_variables(ltf, \
								 "channel_name", appinfo->cache_channel_name, \
								 "account_name", appinfo->cache_account_name, \
								 "topic", appinfo->cache_topic, \
								 "channel_mode", appinfo->cache_channel_mode, \
								 "member_number", appinfo->cache_member_number, \
								 "op_number", appinfo->cache_op_number, \
								 "nick", appinfo->cache_nick, \
								 "version", VERSION, \
								 "updated_entry_number", appinfo->cache_updated_entry_number, \
								 "updated_private_talk_number", appinfo->cache_updated_private_talk_number, \
								 NULL)

	SET_VARIABLES(appinfo->ltf_title);
	buf = loqui_title_format_fetch(appinfo->ltf_title);
	gtk_window_set_title(GTK_WINDOW(priv->app), buf);
	g_free(buf);

	SET_VARIABLES(appinfo->ltf_statusbar);
	buf = loqui_title_format_fetch(appinfo->ltf_statusbar);
	loqui_statusbar_set_default(LOQUI_STATUSBAR(priv->app->statusbar), buf);
	g_free(buf);
}
void
loqui_app_info_current_channel_entry_changed(LoquiAppInfo *appinfo, LoquiChannelEntry *old_chent, LoquiChannelEntry *new_chent)
{
	LoquiAppInfoPrivate *priv;
	LoquiApp *app;
	LoquiAccount *old_account, *account;
	LoquiChannel *old_channel, *channel;

	g_return_if_fail(appinfo != NULL);
	g_return_if_fail(LOQUI_IS_APP_INFO(appinfo));

	priv = appinfo->priv;
	app = priv->app;

	loqui_channel_entry_utils_separate(old_chent, &old_account, &old_channel);
	loqui_channel_entry_utils_separate(new_chent, &account, &channel);

	if (old_chent) {
		g_signal_handlers_disconnect_by_func(old_chent, loqui_app_info_channel_entry_notify_member_number_cb, appinfo);
		g_signal_handlers_disconnect_by_func(old_chent, loqui_app_info_channel_entry_notify_op_number_cb, appinfo);
	}
	if (old_account) {
		g_signal_handlers_disconnect_by_func(old_account, loqui_app_info_user_self_notify_nick_cb, appinfo);
		g_signal_handlers_disconnect_by_func(old_account, loqui_app_info_user_self_notify_away_cb, appinfo);
		g_signal_handlers_disconnect_by_func(old_account, loqui_app_info_account_notify_name_cb, appinfo);
	}
	if (old_channel) {
		g_signal_handlers_disconnect_by_func(old_channel, loqui_app_info_account_notify_name_cb, appinfo);
		g_signal_handlers_disconnect_by_func(old_channel, loqui_app_info_channel_mode_changed_cb, appinfo);
	}

	/* TODO: check difference between old and new */

	loqui_app_info_update_account_name(appinfo, account);
	loqui_app_info_update_nick(appinfo, account);
	loqui_app_info_update_preset(appinfo, account);
	loqui_app_info_update_away_list(appinfo, account);
	loqui_app_info_update_away(appinfo, account);	

	loqui_app_info_update_channel_name(appinfo, channel);
	loqui_app_info_update_channel_mode(appinfo, channel);

	loqui_app_info_update_topic(appinfo, new_chent);
	loqui_app_info_update_member_number(appinfo, new_chent);
	loqui_app_info_update_op_number(appinfo, new_chent);

	loqui_channelbar_update_channel_entry_label(LOQUI_CHANNELBAR(appinfo->priv->app->channelbar), new_chent);

	if (new_chent) {
		g_signal_connect(G_OBJECT(new_chent), "notify::member-number",
				 G_CALLBACK(loqui_app_info_channel_entry_notify_member_number_cb), appinfo);
		g_signal_connect(G_OBJECT(new_chent), "notify::op-number",
				 G_CALLBACK(loqui_app_info_channel_entry_notify_op_number_cb), appinfo);
		g_signal_connect(G_OBJECT(new_chent), "notify::topic",
				 G_CALLBACK(loqui_app_info_channel_entry_notify_topic_cb), appinfo);
	}
	if (account) {
		g_signal_connect(G_OBJECT(account->user_self), "notify::nick",
				 G_CALLBACK(loqui_app_info_user_self_notify_nick_cb), appinfo);
		g_signal_connect(G_OBJECT(account->user_self), "notify::away",
				 G_CALLBACK(loqui_app_info_user_self_notify_away_cb), appinfo);
		g_signal_connect(G_OBJECT(account->user_self), "notify::away-message",
				 G_CALLBACK(loqui_app_info_user_self_notify_away_cb), appinfo);
		g_signal_connect(G_OBJECT(account), "notify::name",
				 G_CALLBACK(loqui_app_info_account_notify_name_cb), appinfo);
	}
	if (channel) {
		g_signal_connect(G_OBJECT(channel), "notify::name",
				 G_CALLBACK(loqui_app_info_channel_notify_name_cb), appinfo);
		g_signal_connect(G_OBJECT(channel), "mode_changed",
				 G_CALLBACK(loqui_app_info_channel_mode_changed_cb), appinfo);
	}
	loqui_app_info_update_string_idle(appinfo);
}
void
loqui_app_info_channel_entry_added(LoquiAppInfo *appinfo, LoquiChannelEntry *chent)
{
	g_signal_connect(G_OBJECT(chent), "notify::is-updated",
			 G_CALLBACK(loqui_app_info_channel_entry_notify_is_updated_cb), appinfo);
}
void
loqui_app_info_channel_entry_removed(LoquiAppInfo *appinfo, LoquiChannelEntry *chent)
{
	g_signal_handlers_disconnect_by_func(chent, loqui_app_info_channel_entry_notify_is_updated_cb, appinfo);
	if (loqui_channel_entry_get_is_updated(chent)) {
		appinfo->updated_entry_number--;
		loqui_app_info_update_updated_entry_number(appinfo);
		loqui_app_info_update_string_idle(appinfo);
	}
}
void
loqui_app_info_account_added(LoquiAppInfo *appinfo, LoquiAccount *account)
{
}
void
loqui_app_info_account_removed(LoquiAppInfo *appinfo, LoquiAccount *account)
{
}
void
loqui_app_info_channel_added(LoquiAppInfo *appinfo, LoquiChannel *channel)
{
}
void
loqui_app_info_channel_removed(LoquiAppInfo *appinfo, LoquiChannel *channel)
{
	if (loqui_channel_get_is_private_talk(channel) && loqui_channel_entry_get_is_updated(LOQUI_CHANNEL_ENTRY(channel))) {
		appinfo->updated_private_talk_number--;
		loqui_app_info_update_updated_private_talk_number(appinfo);
		loqui_app_info_update_string_idle(appinfo);
	}
}
void
loqui_app_info_set_title_format_title(LoquiAppInfo *appinfo, LoquiTitleFormat *ltf)
{
	gboolean is_default_title_format_title_valid;

	if (appinfo->ltf_title) {
		loqui_title_format_free(appinfo->ltf_title);
	}
	if (ltf) {
		appinfo->ltf_title = ltf;
	} else {
		appinfo->ltf_title = loqui_title_format_new();
		is_default_title_format_title_valid = loqui_title_format_parse(appinfo->ltf_title, LOQUI_APP_INFO_DEFAULT_TITLE_FORMAT_TITLE, NULL);
		g_assert(is_default_title_format_title_valid);
	}
}
void
loqui_app_info_set_title_format_statusbar(LoquiAppInfo *appinfo, LoquiTitleFormat *ltf)
{
	gboolean is_default_title_format_statusbar_valid;

	if (appinfo->ltf_statusbar) {
		loqui_title_format_free(appinfo->ltf_statusbar);
	}
	if (ltf) {
		appinfo->ltf_statusbar = ltf;
	} else {
		appinfo->ltf_statusbar = loqui_title_format_new();
		is_default_title_format_statusbar_valid = loqui_title_format_parse(appinfo->ltf_statusbar, LOQUI_APP_INFO_DEFAULT_TITLE_FORMAT_STATUSBAR, NULL);
		g_assert(is_default_title_format_statusbar_valid);
	}
}
void
loqui_app_info_load_from_prefs_general(LoquiAppInfo *appinfo)
{
	LoquiTitleFormat *ltf;

#define LOAD_TITLE_FORMAT(_pref, _name, _setter) {\
	if (strlen(_pref) > 0) { \
		ltf = loqui_title_format_new(); \
		if (!loqui_title_format_parse(ltf, _pref, NULL)) { \
			g_warning("Invalid title format: default is used for %s.", _name); \
			loqui_title_format_free(ltf); \
			_setter(appinfo, NULL); \
		} else { \
			_setter(appinfo, ltf); \
		} \
	} \
}

	LOAD_TITLE_FORMAT(prefs_general.title_format_title, "title", loqui_app_info_set_title_format_title);
	LOAD_TITLE_FORMAT(prefs_general.title_format_statusbar, "statusbar", loqui_app_info_set_title_format_statusbar);
}
