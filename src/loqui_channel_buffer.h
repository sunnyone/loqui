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
#ifndef __LOQUI_CHANNEL_BUFFER_H__
#define __LOQUI_CHANNEL_BUFFER_H__

/* this is interface */
#include <glib-object.h>
#include "loqui_message_text.h"

G_BEGIN_DECLS

#define LOQUI_TYPE_CHANNEL_BUFFER                 (loqui_channel_buffer_get_type ())
#define LOQUI_CHANNEL_BUFFER(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), LOQUI_TYPE_CHANNEL_BUFFER, LoquiChannelBuffer))
#define LOQUI_IS_CHANNEL_BUFFER(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LOQUI_TYPE_CHANNEL_BUFFER))
#define LOQUI_CHANNEL_BUFFER_GET_IFACE(obj)       (G_TYPE_INSTANCE_GET_INTERFACE ((obj), LOQUI_TYPE_CHANNEL_BUFFER, LoquiChannelBufferIface))

typedef struct _LoquiChannelBuffer            LoquiChannelBuffer; /* dummy */
typedef struct _LoquiChannelBufferIface       LoquiChannelBufferIface;

struct _LoquiChannelBufferIface
{
        GTypeInterface parent;

	void (* append) (LoquiChannelBuffer *chbuf, LoquiMessageText *msgtext);
};

GType loqui_channel_buffer_get_type(void) G_GNUC_CONST;

void loqui_channel_buffer_append_message_text(LoquiChannelBuffer *chbuf, LoquiMessageText *msgtext);

G_END_DECLS

#endif /* __LOQUI_CHANNEL_BUFFER_H__ */
