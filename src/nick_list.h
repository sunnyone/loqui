/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui -- IRC client for Gtk2 <http://loqui.good-day.net/>
 * Copyright (C) 2002-2003 Yoichi Imai <yoichi@silver-forest.com>
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
#ifndef __NICK_LIST_H__
#define __NICK_LIST_H__

#include "channel.h"

G_BEGIN_DECLS

#define TYPE_NICK_LIST                 (nick_list_get_type ())
#define NICK_LIST(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_NICK_LIST, NickList))
#define NICK_LIST_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_NICK_LIST, NickListClass))
#define IS_NICK_LIST(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_NICK_LIST))
#define IS_NICK_LIST_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_NICK_LIST))
#define NICK_LIST_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_NICK_LIST, NickListClass))

typedef struct _NickList            NickList;
typedef struct _NickListClass       NickListClass;

typedef struct _NickListPrivate     NickListPrivate;

#include "loqui_app.h"

struct _NickList
{
        GtkTreeView parent;
        
        NickListPrivate *priv;
};

struct _NickListClass
{
        GtkTreeViewClass parent_class;
};


GtkType nick_list_get_type(void) G_GNUC_CONST;

GtkWidget* nick_list_new(LoquiApp *app, GtkWidget *menu);

void nick_list_set_store(NickList *list, GtkListStore *store);
void nick_list_change_mode_selected(NickList *list, gboolean is_give, IRCModeFlag flag);

void nick_list_start_private_talk_selected(NickList *nick_list);
void nick_list_whois_selected(NickList *nick_list);
void nick_list_ctcp_selected(NickList *nick_list, const gchar *command);

G_END_DECLS

#endif /* __NICK_LIST_H__ */
