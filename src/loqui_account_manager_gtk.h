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
#ifndef __LOQUI_ACCOUNT_MANAGER_GTK_H__
#define __LOQUI_ACCOUNT_MANAGER_GTK_H__

#include <glib-object.h>
#include "loqui_account_manager.h"
#include "loqui_protocol_manager.h"
#include "loqui_app.h"

G_BEGIN_DECLS

#define LOQUI_TYPE_ACCOUNT_MANAGER_GTK                 (loqui_account_manager_gtk_get_type ())
#define LOQUI_ACCOUNT_MANAGER_GTK(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), LOQUI_TYPE_ACCOUNT_MANAGER_GTK, LoquiAccountManagerGtk))
#define LOQUI_ACCOUNT_MANAGER_GTK_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), LOQUI_TYPE_ACCOUNT_MANAGER_GTK, LoquiAccountManagerGtkClass))
#define LOQUI_IS_ACCOUNT_MANAGER_GTK(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LOQUI_TYPE_ACCOUNT_MANAGER_GTK))
#define LOQUI_IS_ACCOUNT_MANAGER_GTK_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), LOQUI_TYPE_ACCOUNT_MANAGER_GTK))
#define LOQUI_ACCOUNT_MANAGER_GTK_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), LOQUI_TYPE_ACCOUNT_MANAGER_GTK, LoquiAccountManagerGtkClass))

typedef struct _LoquiAccountManagerGtk            LoquiAccountManagerGtk;
typedef struct _LoquiAccountManagerGtkClass       LoquiAccountManagerGtkClass;

typedef struct _LoquiAccountManagerGtkPrivate     LoquiAccountManagerGtkPrivate;

struct _LoquiAccountManagerGtk
{
        LoquiAccountManager parent;

        LoquiApp *app;

        LoquiAccountManagerGtkPrivate *priv;
};

struct _LoquiAccountManagerGtkClass
{
        LoquiAccountManagerClass parent_class;
};


GType loqui_account_manager_gtk_get_type(void) G_GNUC_CONST;

LoquiAccountManagerGtk* loqui_account_manager_gtk_new(LoquiProtocolManager *pmanag);
G_END_DECLS

#endif /* __LOQUI_ACCOUNT_MANAGER_GTK_H__ */
