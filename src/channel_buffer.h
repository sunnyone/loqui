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
#ifndef __CHANNEL_BUFFER_H__
#define __CHANNEL_BUFFER_H__

#include <gtk/gtk.h>
#include "message_text.h"

G_BEGIN_DECLS

#define TYPE_CHANNEL_BUFFER                 (channel_buffer_get_type ())
#define CHANNEL_BUFFER(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_CHANNEL_BUFFER, ChannelBuffer))
#define CHANNEL_BUFFER_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_CHANNEL_BUFFER, ChannelBufferClass))
#define IS_CHANNEL_BUFFER(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_CHANNEL_BUFFER))
#define IS_CHANNEL_BUFFER_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_CHANNEL_BUFFER))
#define CHANNEL_BUFFER_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_CHANNEL_BUFFER, ChannelBufferClass))

typedef struct _ChannelBuffer            ChannelBuffer;
typedef struct _ChannelBufferClass       ChannelBufferClass;

typedef struct _ChannelBufferPrivate     ChannelBufferPrivate;

struct _ChannelBuffer
{
	GtkTextBuffer buffer;

        ChannelBufferPrivate *priv;
};

struct _ChannelBufferClass
{
        GtkTextBufferClass parent_class;

	GtkTextTagTable *tag_table;
	
	void (* append) (ChannelBuffer *buffer,
			 MessageText *msgtext);
};


GtkType channel_buffer_get_type (void) G_GNUC_CONST;

ChannelBuffer* channel_buffer_new (void);

void channel_buffer_append_message_text(ChannelBuffer *buffer, MessageText *msgtext, 
					gboolean verbose, gboolean exec_notification);
void channel_buffer_set_max_line_number(ChannelBuffer *buffer, guint max_line_number);
void channel_buffer_set_whether_common_buffer(ChannelBuffer *buffer, gboolean is_common_buffer);

G_END_DECLS

#endif /* __CHANNEL_BUFFER_H__ */
