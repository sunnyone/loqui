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
#ifndef __BUFFER_MENU_H__
#define __BUFFER_MENU_H__

#include <gtk/gtk.h>

#include "account.h"
#include "channel.h"

GtkWidget *buffer_menu_add_account(GtkMenuShell *menu, Account *account);
void buffer_menu_update_account(GtkMenuShell *menu, Account *account);
void buffer_menu_remove_account(GtkMenuShell *menu, Account *account);

GtkWidget *buffer_menu_add_channel(GtkMenuShell *menu, Channel *channel);
void buffer_menu_update_channel(GtkMenuShell *menu, Channel *channel);
void buffer_menu_remove_channel(GtkMenuShell *menu, Channel *channel);

#endif /* __BUFFER_MENU_H__ */
