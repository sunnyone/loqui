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
#ifndef __LOQUI_TRAY_ICON_H__
#define __LOQUI_TRAY_ICON_H__

#include <gtk/gtk.h>
#include "eggtrayicon.h"

G_BEGIN_DECLS

#define LOQUI_TYPE_TRAY_ICON                 (loqui_tray_icon_get_type ())
#define LOQUI_TRAY_ICON(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), LOQUI_TYPE_TRAY_ICON, LoquiTrayIcon))
#define LOQUI_TRAY_ICON_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), LOQUI_TYPE_TRAY_ICON, LoquiTrayIconClass))
#define LOQUI_IS_TRAY_ICON(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LOQUI_TYPE_TRAY_ICON))
#define LOQUI_IS_TRAY_ICON_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), LOQUI_TYPE_TRAY_ICON))
#define LOQUI_TRAY_ICON_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), LOQUI_TYPE_TRAY_ICON, LoquiTrayIconClass))

typedef struct _LoquiTrayIcon            LoquiTrayIcon;
typedef struct _LoquiTrayIconClass       LoquiTrayIconClass;

typedef struct _LoquiTrayIconPrivate     LoquiTrayIconPrivate;

#include "loqui_app.h"

struct _LoquiTrayIcon
{
        EggTrayIcon parent;
        
	GtkWidget *image;
	gboolean is_hilighted;

	LoquiApp *app;

        LoquiTrayIconPrivate *priv;
};

struct _LoquiTrayIconClass
{
        EggTrayIconClass parent_class;
};


GType loqui_tray_icon_get_type(void) G_GNUC_CONST;

GtkWidget* loqui_tray_icon_new(LoquiApp *app, GtkMenu *menu);

void loqui_tray_icon_set_hilighted(LoquiTrayIcon *tray_icon, gboolean is_hilighted);
void loqui_tray_icon_blink(LoquiTrayIcon *tray_icon);

G_END_DECLS

#endif /* __LOQUI_TRAY_ICON_H__ */
