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
#ifndef __CHANNEL_BAR_H__
#define __CHANNEL_BAR_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define TYPE_CHANNEL_BAR                 (channel_bar_get_type ())
#define CHANNEL_BAR(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_CHANNEL_BAR, ChannelBar))
#define CHANNEL_BAR_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_CHANNEL_BAR, ChannelBarClass))
#define IS_CHANNEL_BAR(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_CHANNEL_BAR))
#define IS_CHANNEL_BAR_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_CHANNEL_BAR))
#define CHANNEL_BAR_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_CHANNEL_BAR, ChannelBarClass))

typedef struct _ChannelBar            ChannelBar;
typedef struct _ChannelBarClass       ChannelBarClass;

typedef struct _ChannelBarPrivate     ChannelBarPrivate;

struct _ChannelBar
{
        GtkToolbar parent;
        
        ChannelBarPrivate *priv;
};

struct _ChannelBarClass
{
        GtkToolbarClass parent_class;
};


GtkType channel_bar_get_type (void) G_GNUC_CONST;

GtkWidget* channel_bar_new (void);

G_END_DECLS

#endif /* __CHANNEL_BAR_H__ */
