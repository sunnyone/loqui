/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui -- Chat client for Gtk
 * Copyright (C) 2004 Yoichi Imai <sunnyone41@gmail.com>
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
#ifndef __LOQUI_CHANNEL_TEXT_VIEW_H__
#define __LOQUI_CHANNEL_TEXT_VIEW_H__

#include <gtk/gtk.h>
#include "loqui_channel_buffer_gtk.h"

G_BEGIN_DECLS

#define LOQUI_TYPE_CHANNEL_TEXT_VIEW                 (loqui_channel_text_view_get_type ())
#define LOQUI_CHANNEL_TEXT_VIEW(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), LOQUI_TYPE_CHANNEL_TEXT_VIEW, LoquiChannelTextView))
#define LOQUI_CHANNEL_TEXT_VIEW_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), LOQUI_TYPE_CHANNEL_TEXT_VIEW, LoquiChannelTextViewClass))
#define LOQUI_IS_CHANNEL_TEXT_VIEW(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LOQUI_TYPE_CHANNEL_TEXT_VIEW))
#define LOQUI_IS_CHANNEL_TEXT_VIEW_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), LOQUI_TYPE_CHANNEL_TEXT_VIEW))
#define LOQUI_CHANNEL_TEXT_VIEW_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), LOQUI_TYPE_CHANNEL_TEXT_VIEW, LoquiChannelTextViewClass))

typedef struct _LoquiChannelTextView            LoquiChannelTextView;
typedef struct _LoquiChannelTextViewClass       LoquiChannelTextViewClass;

typedef struct _LoquiChannelTextViewPrivate     LoquiChannelTextViewPrivate;

#include "loqui_app.h"
#include "loqui_channel_buffer_gtk.h"

struct _LoquiChannelTextView
{
        GtkTextView parent;

	GtkWidget *scrolled_window;
        gboolean is_scroll;
	gboolean auto_switch_scrolling;

        LoquiChannelTextViewPrivate *priv;
};

struct _LoquiChannelTextViewClass
{
        GtkTextViewClass parent_class;

	void (* scrolled_to_end) (LoquiChannelTextView *chview);
};


GType loqui_channel_text_view_get_type(void) G_GNUC_CONST;

GtkWidget* loqui_channel_text_view_new(LoquiApp *app);

void loqui_channel_text_view_set_channel_buffer(LoquiChannelTextView *chview, LoquiChannelBufferGtk *buffer);

void loqui_channel_text_view_set_is_scroll(LoquiChannelTextView *textview, gboolean is_scroll);
gboolean loqui_channel_text_view_get_is_scroll(LoquiChannelTextView *textview);

void loqui_channel_text_view_scroll_to_end(LoquiChannelTextView *chview);
void loqui_channel_text_view_scroll_to_end_if_enabled(LoquiChannelTextView *chview);

void loqui_channel_text_view_set_auto_switch_scrolling(LoquiChannelTextView *textview, gboolean auto_switch_scrolling);
gboolean loqui_channel_text_view_get_auto_switch_scrolling(LoquiChannelTextView *textview);

void loqui_channel_text_view_scroll(LoquiChannelTextView *chview, GtkMovementStep step, gint count);

G_END_DECLS

#endif /* __LOQUI_CHANNEL_TEXT_VIEW_H__ */
