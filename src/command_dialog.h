/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui -- IRC client for Gtk2 <http://loqui.good-day.net/>
 * Copyright (C) 2003 Yoichi Imai <yoichi@silver-forest.com>
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
#ifndef __COMMAND_DIALOG_H__
#define __COMMAND_DIALOG_H__

#include <loqui_account.h>
#include "loqui_app.h"

void command_dialog_join(LoquiApp *app, LoquiAccount *account);
void command_dialog_part(LoquiApp *app, LoquiAccount *account, LoquiChannel *channel);
void command_dialog_topic(LoquiApp *app, LoquiAccount *account, LoquiChannel *channel);
void command_dialog_nick(LoquiApp *app, LoquiAccount *account);
void command_dialog_private_talk(LoquiApp *app, LoquiAccount *account);

#endif /* __COMMAND_DIALOG_H__ */
