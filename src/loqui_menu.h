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
#ifndef __LOQUI_MENU_H__
#define __LOQUI_MENU_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define LOQUI_TYPE_MENU                 (loqui_menu_get_type ())
#define LOQUI_MENU(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), LOQUI_TYPE_MENU, LoquiMenu))
#define LOQUI_MENU_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), LOQUI_TYPE_MENU, LoquiMenuClass))
#define LOQUI_IS_MENU(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LOQUI_TYPE_MENU))
#define LOQUI_IS_MENU_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), LOQUI_TYPE_MENU))
#define LOQUI_MENU_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), LOQUI_TYPE_MENU, LoquiMenuClass))

typedef struct _LoquiMenu            LoquiMenu;
typedef struct _LoquiMenuClass       LoquiMenuClass;

typedef struct _LoquiMenuPrivate     LoquiMenuPrivate;

#include "loqui_app.h"

struct _LoquiMenu
{
        GObject parent;
        
        LoquiMenuPrivate *priv;
};

struct _LoquiMenuClass
{
        GObjectClass parent_class;
};


GType loqui_menu_get_type(void) G_GNUC_CONST;

LoquiMenu* loqui_menu_new(LoquiApp *app);

GtkWidget* loqui_menu_get_widget(LoquiMenu *menu);

void loqui_menu_set_view_toolbar(LoquiMenu *menu, guint style);
void loqui_menu_set_view_statusbar(LoquiMenu *menu, gboolean show);
void loqui_menu_set_view_channelbar(LoquiMenu *menu, gboolean show);

void loqui_menu_buffers_add_account(LoquiMenu *menu, Account *account);
void loqui_menu_buffers_remove_account(LoquiMenu *menu, Account *account);
void loqui_menu_buffers_update_account(LoquiMenu *menu, Account *account);

void loqui_menu_buffers_add_channel(LoquiMenu *menu, Channel *channel);
void loqui_menu_buffers_remove_channel(LoquiMenu *menu, Channel *channel);
void loqui_menu_buffers_update_channel(LoquiMenu *menu, Channel *channel);

G_END_DECLS

#endif /* __LOQUI_MENU_H__ */
