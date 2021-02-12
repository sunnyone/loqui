/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui -- IRC client for Gtk
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
#ifndef __LOQUI_STATUSBAR_H__
#define __LOQUI_STATUSBAR_H__

#include <gtk/gtk.h>

#include <loqui_channel.h>
#include <loqui_account.h>
#include "loqui_app.h"

G_BEGIN_DECLS

#define LOQUI_TYPE_STATUSBAR                 (loqui_statusbar_get_type ())
#define LOQUI_STATUSBAR(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), LOQUI_TYPE_STATUSBAR, LoquiStatusbar))
#define LOQUI_STATUSBAR_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), LOQUI_TYPE_STATUSBAR, LoquiStatusbarClass))
#define LOQUI_IS_STATUSBAR(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LOQUI_TYPE_STATUSBAR))
#define LOQUI_IS_STATUSBAR_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), LOQUI_TYPE_STATUSBAR))
#define LOQUI_STATUSBAR_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), LOQUI_TYPE_STATUSBAR, LoquiStatusbarClass))

typedef struct _LoquiStatusbar            LoquiStatusbar;
typedef struct _LoquiStatusbarClass       LoquiStatusbarClass;

typedef struct _LoquiStatusbarPrivate     LoquiStatusbarPrivate;

struct _LoquiStatusbar
{
        GtkStatusbar parent;

        LoquiStatusbarPrivate *priv;
};

struct _LoquiStatusbarClass
{
        GtkStatusbarClass parent_class;
};


GType loqui_statusbar_get_type (void) G_GNUC_CONST;

GtkWidget* loqui_statusbar_new(LoquiApp *app, GtkToggleAction *toggle_scroll_action_common_buffer);

void loqui_statusbar_update_current_account(LoquiStatusbar *statusbar, LoquiAccount *account);

void loqui_statusbar_set_default(LoquiStatusbar *statusbar, const gchar *str);
void loqui_statusbar_set_completion(LoquiStatusbar *statusbar, const gchar *str);

void loqui_statusbar_update_account_name(LoquiStatusbar *statusbar, LoquiAccount *account);
void loqui_statusbar_update_nick(LoquiStatusbar *statusbar, LoquiAccount *account);
void loqui_statusbar_update_preset_menu(LoquiStatusbar *statusbar, LoquiAccount *account);
void loqui_statusbar_update_away_menu(LoquiStatusbar *statusbar, LoquiAccount *account);
void loqui_statusbar_update_away_icon(LoquiStatusbar *statusbar, LoquiAccount *account);

G_END_DECLS

#endif /* __LOQUI_STATUSBAR_H__ */
