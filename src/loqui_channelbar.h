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
#ifndef __LOQUI_CHANNELBAR_H__
#define __LOQUI_CHANNELBAR_H__

#include <gtk/gtk.h>

#include "account.h"
#include "loqui_channel.h"
#include "loqui_app.h"

G_BEGIN_DECLS

#define LOQUI_TYPE_CHANNELBAR                 (loqui_channelbar_get_type ())
#define LOQUI_CHANNELBAR(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), LOQUI_TYPE_CHANNELBAR, LoquiChannelbar))
#define LOQUI_CHANNELBAR_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), LOQUI_TYPE_CHANNELBAR, LoquiChannelbarClass))
#define LOQUI_IS_CHANNELBAR(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LOQUI_TYPE_CHANNELBAR))
#define LOQUI_IS_CHANNELBAR_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), LOQUI_TYPE_CHANNELBAR))
#define LOQUI_CHANNELBAR_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), LOQUI_TYPE_CHANNELBAR, LoquiChannelbarClass))

typedef struct _LoquiChannelbar            LoquiChannelbar;
typedef struct _LoquiChannelbarClass       LoquiChannelbarClass;

typedef struct _LoquiChannelbarPrivate     LoquiChannelbarPrivate;

struct _LoquiChannelbar
{
        GtkHBox parent;
        
        LoquiChannelbarPrivate *priv;
};

struct _LoquiChannelbarClass
{
        GtkHBoxClass parent_class;
};


GtkType loqui_channelbar_get_type (void) G_GNUC_CONST;

GtkWidget* loqui_channelbar_new(LoquiApp *app, GtkWidget *menu_dropdown, GtkToggleAction *toggle_scroll_action);

void loqui_channelbar_add_account(LoquiChannelbar *channelbar, Account *account);
void loqui_channelbar_update_account(LoquiChannelbar *channelbar, Account *account);
void loqui_channelbar_remove_account(LoquiChannelbar *channelbar, Account *account);
void loqui_channelbar_add_channel(LoquiChannelbar *channelbar, LoquiChannel *channel);
void loqui_channelbar_remove_channel(LoquiChannelbar *channelbar, LoquiChannel *channel);
void loqui_channelbar_update_channel(LoquiChannelbar *channelbar, LoquiChannel *channel);
void loqui_channelbar_set_current_channel(LoquiChannelbar *channelbar, LoquiChannel *channel);
void loqui_channelbar_set_current_account(LoquiChannelbar *channelbar, Account *account);

G_END_DECLS

#endif /* __LOQUI_CHANNELBAR_H__ */
