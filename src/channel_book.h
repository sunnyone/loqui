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
#ifndef __CHANNEL_BOOK_H__
#define __CHANNEL_BOOK_H__

#include "channel_text.h"

G_BEGIN_DECLS

#define TYPE_CHANNEL_BOOK                 (channel_book_get_type ())
#define CHANNEL_BOOK(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_CHANNEL_BOOK, ChannelBook))
#define CHANNEL_BOOK_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_CHANNEL_BOOK, ChannelBookClass))
#define IS_CHANNEL_BOOK(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_CHANNEL_BOOK))
#define IS_CHANNEL_BOOK_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_CHANNEL_BOOK))
#define CHANNEL_BOOK_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_CHANNEL_BOOK, ChannelBookClass))

typedef struct _ChannelBook            ChannelBook;
typedef struct _ChannelBookClass       ChannelBookClass;

typedef struct _ChannelBookPrivate     ChannelBookPrivate;

struct _ChannelBook
{
        GtkNotebook parent;
        
        ChannelBookPrivate *priv;
};

struct _ChannelBookClass
{
        GtkNotebookClass parent_class;
};


GtkType channel_book_get_type(void) G_GNUC_CONST;

GtkWidget* channel_book_new(void);
void channel_book_add_channel_text(ChannelBook *book, ChannelText *text);
void channel_book_remove_channel_text(ChannelBook *book, ChannelText *text);
void channel_book_change_current(ChannelBook *book, ChannelText *text);

G_END_DECLS

#endif /* __CHANNEL_BOOK_H__ */
