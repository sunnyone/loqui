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
#ifndef __LOQUI_CHANNEL_ENTRY_UI_H__
#define __LOQUI_CHANNEL_ENTRY_UI_H__
#include "loqui_app.h"
#include "loqui_channel_entry.h"
#include "loqui_channel_entry_action.h"

void loqui_channel_entry_ui_attach_channel_entry_action(LoquiApp *app, LoquiChannelEntry *channel_entry);
void loqui_channel_entry_ui_add_account(LoquiApp *app, LoquiAccount *account, const gchar *path, const gchar *data_prefix);
void loqui_channel_entry_ui_remove_account(LoquiApp *app, LoquiAccount *account, const gchar *path, const gchar *data_prefix);

void loqui_channel_entry_ui_add_channel(LoquiApp *app, LoquiChannel *channel, const gchar *path, const gchar *data_prefix);
void loqui_channel_entry_ui_remove_channel(LoquiApp *app, LoquiChannel *channel, const gchar *data_prefix);

#endif /* __LOQUI_CHANNEL_ENTRY_UI_H__ */
