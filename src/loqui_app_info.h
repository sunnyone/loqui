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
/* The class updates the app's information with the current account/channel */
#ifndef __LOQUI_APP_INFO_H__
#define __LOQUI_APP_INFO_H__

#include <gtk/gtk.h>
#include "loqui_title_format.h"

G_BEGIN_DECLS

#define LOQUI_TYPE_APP_INFO                 (loqui_app_info_get_type ())
#define LOQUI_APP_INFO(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), LOQUI_TYPE_APP_INFO, LoquiAppInfo))
#define LOQUI_APP_INFO_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), LOQUI_TYPE_APP_INFO, LoquiAppInfoClass))
#define LOQUI_IS_APP_INFO(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LOQUI_TYPE_APP_INFO))
#define LOQUI_IS_APP_INFO_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), LOQUI_TYPE_APP_INFO))
#define LOQUI_APP_INFO_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), LOQUI_TYPE_APP_INFO, LoquiAppInfoClass))

typedef struct _LoquiAppInfo            LoquiAppInfo;
typedef struct _LoquiAppInfoClass       LoquiAppInfoClass;

typedef struct _LoquiAppInfoPrivate     LoquiAppInfoPrivate;

#include "loqui_app.h"

struct _LoquiAppInfo
{
        GObject parent;
        
	LoquiTitleFormat *ltf_title;
	LoquiTitleFormat *ltf_statusbar;

	gchar *cache_account_name;
	gchar *cache_channel_name;
	gchar *cache_topic;
	gchar *cache_channel_mode;
	gchar *cache_member_number;
	gchar *cache_op_number;
	gchar *cache_nick;

	gchar *cache_updated_entry_number;
	gchar *cache_updated_private_talk_number;

	guint updated_entry_number;
	guint updated_private_talk_number;

        LoquiAppInfoPrivate *priv;
};

struct _LoquiAppInfoClass
{
        GObjectClass parent_class;
};


GType loqui_app_info_get_type(void) G_GNUC_CONST;

LoquiAppInfo* loqui_app_info_new(LoquiApp *app);

void loqui_app_info_update_account_name(LoquiAppInfo *appinfo, LoquiAccount *account);
void loqui_app_info_update_nick(LoquiAppInfo *appinfo, LoquiAccount *account);

void loqui_app_info_update_topic(LoquiAppInfo *appinfo, LoquiChannelEntry *chent);
void loqui_app_info_update_channel_name(LoquiAppInfo *appinfo, LoquiChannel *channel);
void loqui_app_info_update_channel_mode(LoquiAppInfo *appinfo, LoquiChannel *channel);
void loqui_app_info_update_member_number(LoquiAppInfo *appinfo, LoquiChannelEntry *chent);
void loqui_app_info_update_op_number(LoquiAppInfo *appinfo, LoquiChannelEntry *chent);

void loqui_app_info_update_preset(LoquiAppInfo *appinfo, LoquiAccount *account);
void loqui_app_info_update_away_list(LoquiAppInfo *appinfo, LoquiAccount *account);
void loqui_app_info_update_away(LoquiAppInfo *appinfo, LoquiAccount *account);

void loqui_app_info_update_updated_entry_number(LoquiAppInfo *appinfo);
void loqui_app_info_update_updated_private_talk_number(LoquiAppInfo *appinfo);

void loqui_app_info_update_string_idle(LoquiAppInfo *appinfo);
void loqui_app_info_update_string(LoquiAppInfo *appinfo);

void loqui_app_info_update_all(LoquiAppInfo *appinfo, LoquiChannelEntry *chent);

void loqui_app_info_current_channel_entry_changed(LoquiAppInfo *appinfo, LoquiChannelEntry *old_chent, LoquiChannelEntry *new_chent);

void loqui_app_info_channel_entry_added(LoquiAppInfo *appinfo, LoquiChannelEntry *chent);
void loqui_app_info_channel_entry_removed(LoquiAppInfo *appinfo, LoquiChannelEntry *chent);

void loqui_app_info_account_added(LoquiAppInfo *appinfo, LoquiAccount *account);
void loqui_app_info_account_removed(LoquiAppInfo *appinfo, LoquiAccount *account);

void loqui_app_info_channel_added(LoquiAppInfo *appinfo, LoquiChannel *channel);
void loqui_app_info_channel_removed(LoquiAppInfo *appinfo, LoquiChannel *channel);

G_END_DECLS

#endif /* __LOQUI_APP_INFO_H__ */
