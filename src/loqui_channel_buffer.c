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
#include "config.h"

#include "loqui_channel_buffer.h"

enum {
	SIGNAL_APPEND_MESSAGE_TEXT,
	LAST_SIGNAL
};

static guint loqui_channel_buffer_signals[LAST_SIGNAL] = { 0 };

static void loqui_channel_buffer_base_init(gpointer object_class);

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


GType
loqui_channel_buffer_get_type(void)
{
        static GType type = 0;

        if (type == 0) {
                static const GTypeInfo info = {
                        sizeof (LoquiChannelBufferIface),
                        loqui_channel_buffer_base_init,   /* base_init */
                        NULL,   /* base_finalize */
                        NULL,   /* class_init */
                        NULL,   /* class_finalize */
                        NULL,   /* class_data */
                        0,
                        0,      /* n_preallocs */
                        NULL    /* instance_init */
                };
                type = g_type_register_static(G_TYPE_INTERFACE, "LoquiChannelBuffer", &info, 0);
        }

        return type;
}

void
loqui_channel_buffer_append_message_text(LoquiChannelBuffer *chbuf, LoquiMessageText *msgtext)
{
	g_signal_emit(chbuf, loqui_channel_buffer_signals[SIGNAL_APPEND_MESSAGE_TEXT], 0, msgtext);
}
