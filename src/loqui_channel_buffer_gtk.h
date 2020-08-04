/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui -- IRC client for Gtk2 <http://launchpad.net/loqui/>
 * Copyright (C) 2002-2003 Yoichi Imai <sunnyone41@gmail.com>
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
#ifndef __LOQUI_CHANNEL_BUFFER_GTK_H__
#define __LOQUI_CHANNEL_BUFFER_GTK_H__

#include <gtk/gtk.h>
#include <libloqui/loqui-message-text.h>
#include "loqui-pref-partial.h"

G_BEGIN_DECLS

#define LOQUI_TYPE_CHANNEL_BUFFER_GTK                 (loqui_channel_buffer_gtk_get_type ())
#define LOQUI_CHANNEL_BUFFER_GTK(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), LOQUI_TYPE_CHANNEL_BUFFER_GTK, LoquiChannelBufferGtk))
#define LOQUI_CHANNEL_BUFFER_GTK_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), LOQUI_TYPE_CHANNEL_BUFFER_GTK, LoquiChannelBufferGtkClass))
#define LOQUI_IS_CHANNEL_BUFFER_GTK(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LOQUI_TYPE_CHANNEL_BUFFER_GTK))
#define LOQUI_IS_CHANNEL_BUFFER_GTK_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), LOQUI_TYPE_CHANNEL_BUFFER_GTK))
#define LOQUI_CHANNEL_BUFFER_GTK_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), LOQUI_TYPE_CHANNEL_BUFFER_GTK, LoquiChannelBufferGtkClass))

typedef struct _LoquiChannelBufferGtk            LoquiChannelBufferGtk;
typedef struct _LoquiChannelBufferGtkClass       LoquiChannelBufferGtkClass;

typedef struct _LoquiChannelBufferGtkPrivate     LoquiChannelBufferGtkPrivate;

struct _LoquiChannelBufferGtk
{
	GtkTextBuffer buffer;

	gboolean show_account_name;
	gboolean show_channel_name;

	gboolean hover_tag_applied;

        LoquiChannelBufferGtkPrivate *priv;
};

struct _LoquiChannelBufferGtkClass
{
        GtkTextBufferClass parent_class;

	GtkTextTagTable *tag_table;
};


GType loqui_channel_buffer_gtk_get_type(void) G_GNUC_CONST;

LoquiChannelBufferGtk* loqui_channel_buffer_gtk_new(LoquiPrefPartial *ppref_channel_buffer);

void loqui_channel_buffer_gtk_set_max_line_number(LoquiChannelBufferGtk *buffer, guint max_line_number);
void loqui_channel_buffer_gtk_set_whether_common_buffer(LoquiChannelBufferGtk *buffer, gboolean is_common_buffer);

void loqui_channel_buffer_gtk_set_show_account_name(LoquiChannelBufferGtk *buffer, gboolean show_account_name);
gboolean loqui_channel_buffer_gtk_get_show_account_name(LoquiChannelBufferGtk *buffer);

void loqui_channel_buffer_gtk_set_show_channel_name(LoquiChannelBufferGtk *buffer, gboolean show_channel_name);
gboolean loqui_channel_buffer_gtk_get_show_channel_name(LoquiChannelBufferGtk *buffer);

G_END_DECLS

#endif /* __LOQUI_CHANNEL_BUFFER_GTK_H__ */
