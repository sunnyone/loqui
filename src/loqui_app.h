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
#ifndef __LOQUI_APP_H__
#define __LOQUI_APP_H__

#include "loqui_menu.h"
#include "channel_tree.h"
#include "nick_list.h"
#include "channel_book.h"
#include "channel_text.h"

G_BEGIN_DECLS

#define LOQUI_TYPE_APP                 (loqui_app_get_type ())
#define LOQUI_APP(obj)                 (GTK_CHECK_CAST ((obj), LOQUI_TYPE_APP, LoquiApp))
#define LOQUI_APP_CLASS(klass)         (GTK_CHECK_CLASS_CAST ((klass), LOQUI_TYPE_APP, LoquiAppClass))
#define LOQUI_IS_APP(obj)              (GTK_CHECK_TYPE ((obj), LOQUI_TYPE_APP))
#define LOQUI_IS_APP_CLASS(klass)      (GTK_CHECK_CLASS_TYPE ((klass), LOQUI_TYPE_APP))
#define LOQUI_APP_GET_CLASS(obj)       (GTK_CHECK_GET_CLASS ((obj), LOQUI_TYPE_APP, LoquiAppClass))

typedef struct _LoquiApp            LoquiApp;
typedef struct _LoquiAppClass       LoquiAppClass;

typedef struct _LoquiAppPrivate     LoquiAppPrivate;

struct _LoquiApp
{
        GtkWindow parent;

        LoquiAppPrivate *priv;

	GAsyncQueue *error_connections;

	LoquiMenu *menu;
	ChannelBook *channel_book;
	ChannelText *common_text;
	NickList *nick_list;
	ChannelTree *channel_tree;
};

struct _LoquiAppClass
{
        GtkWindowClass parent_class;
};

GType        loqui_app_get_type             (void) G_GNUC_CONST;

GtkWidget*      loqui_app_new                 (void);

void loqui_app_set_topic(LoquiApp *app, const gchar *str);
gboolean loqui_app_is_scroll(LoquiApp *app);
void loqui_app_set_focus(LoquiApp *app);

G_END_DECLS

#endif /* __LOQUI_APP_H__ */
