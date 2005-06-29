/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * libloqui -- Chat/IM client library for GLib <http://loqui.good-day.net/>
 * Copyright (C) 2004 Yoichi Imai <yoichi@silver-forest.com>
 *
 * This Library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with the Gnome Library; see the file COPYING.LIB.  If not,
 * write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#include "config.h"

#include "loqui_channel_buffer.h"
#include "gobject_utils.h"

enum {
	SIGNAL_APPEND_MESSAGE_TEXT,
	LAST_SIGNAL
};

static guint loqui_channel_buffer_signals[LAST_SIGNAL] = { 0 };

LOQUI_DEFINE_INTERFACE(LoquiChannelBuffer, loqui_channel_buffer);

static void
loqui_channel_buffer_base_init(gpointer object_class)
{
        static gboolean initialized = FALSE;

        if (!initialized) {
		loqui_channel_buffer_signals[SIGNAL_APPEND_MESSAGE_TEXT] = g_signal_new("append_message_text",
											LOQUI_TYPE_CHANNEL_BUFFER,
											G_SIGNAL_RUN_LAST,
											G_STRUCT_OFFSET(LoquiChannelBufferIface, append_message_text),
											NULL, NULL,
											g_cclosure_marshal_VOID__OBJECT,
											G_TYPE_NONE, 1,
											LOQUI_TYPE_MESSAGE_TEXT);
		
                initialized = TRUE;
        }
}

void
loqui_channel_buffer_append_message_text(LoquiChannelBuffer *chbuf, LoquiMessageText *msgtext)
{
	g_signal_emit(chbuf, loqui_channel_buffer_signals[SIGNAL_APPEND_MESSAGE_TEXT], 0, msgtext);
}
