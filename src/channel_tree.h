/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui -- IRC client for GNOME2
 * Copyright (C) 2002 Yoichi Imai <yoichi@silver-forest.com>
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
#ifndef __CHANNEL_TREE_H__
#define __CHANNEL_TREE_H__

#include <gnome.h>
#include "account.h"

G_BEGIN_DECLS

#define TYPE_CHANNEL_TREE                 (channel_tree_get_type ())
#define CHANNEL_TREE(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_CHANNEL_TREE, ChannelTree))
#define CHANNEL_TREE_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_CHANNEL_TREE, ChannelTreeClass))
#define IS_CHANNEL_TREE(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_CHANNEL_TREE))
#define IS_CHANNEL_TREE_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_CHANNEL_TREE))
#define CHANNEL_TREE_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_CHANNEL_TREE, ChannelTreeClass))

typedef struct _ChannelTree            ChannelTree;
typedef struct _ChannelTreeClass       ChannelTreeClass;

typedef struct _ChannelTreePrivate     ChannelTreePrivate;

struct _ChannelTree
{
        GtkTreeView parent;
        
        ChannelTreePrivate *priv;
};

struct _ChannelTreeClass
{
        GtkTreeViewClass parent_class;
};


GtkType channel_tree_get_type(void) G_GNUC_CONST;

GtkWidget* channel_tree_new(void);

void channel_tree_add_account(ChannelTree *tree, Account *account);

G_END_DECLS

#endif /* __CHANNEL_TREE_H__ */
