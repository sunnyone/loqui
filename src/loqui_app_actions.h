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

#include "loqui_app.h"
#include <gtk24backports.h>

#ifndef __LOQUI_APP_ACTIONS_H__
#define __LOQUI_APP_ACTIONS_H__

#define ACTION_GROUP_ACTION_SET_SENSITIVE(action_group, name, bool) { \
  GtkAction *a; \
  a = gtk_action_group_get_action(action_group, name); \
  if (a) \
    g_object_set(a, "sensitive", bool, NULL); \
}

GtkActionGroup* loqui_app_actions_create_group(LoquiApp *app);
void loqui_app_actions_toggle_action_set_active(LoquiApp *app, const gchar *name, gboolean is_active);
void loqui_app_actions_update_sensitivity_related_channel(LoquiApp *app);

#define LOQUI_ACTION_TOGGLE_SCROLL "ToggleScroll"
#define LOQUI_ACTION_TOGGLE_SCROLL_COMMON_BUFFER "ToggleScrollCommonBuffer"

#define LOQUI_ACTION_CONNECT_CURRENT_ACCOUNT "ConnectCurrentAccount"
#define LOQUI_ACTION_RECONNECT_CURRENT_ACCOUNT "ReconnectCurrentAccount"
#define LOQUI_ACTION_DISCONNECT_CURRENT_ACCOUNT "DisconnectCurrentAccount"

#define LOQUI_ACTION_JOIN "Join"
#define LOQUI_ACTION_PART "Part"
#define LOQUI_ACTION_CHANGE_NICK "ChangeNick"
#define LOQUI_ACTION_SET_TOPIC "SetTopic"
#define LOQUI_ACTION_REFRESH "Refresh"
#define LOQUI_ACTION_START_PRIVATE_TALK "StartPrivateTalk"
#define LOQUI_ACTION_END_PRIVATE_TALK "EndPrivateTalk"

#endif /* __LOQUI_APP_ACTIONS_H__ */
