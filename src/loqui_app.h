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

#include <gnome.h>
#include "loqui_menu.h"
#include "account.h"

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
        GnomeApp parent;
        LoquiAppPrivate *priv;

	GAsyncQueue *error_connections;

	LoquiMenu *menu;
	GtkWidget *channel_book;
	GtkWidget *common_text;
	GtkWidget *nick_list;
	GtkWidget *channel_tree;
};

struct _LoquiAppClass
{
        GnomeAppClass parent_class;
};

GType        loqui_app_get_type             (void) G_GNUC_CONST;

GtkWidget*      loqui_app_new                 (void);

LoquiApp *loqui_app_get_main_app(void);
void loqui_app_set_topic(LoquiApp *app, const gchar *str);

void loqui_app_connect_with_fallback(LoquiApp *app, Account *account);

G_END_DECLS

#endif /* __LOQUI_APP_H__ */
