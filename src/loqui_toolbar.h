/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui -- IRC client for Gtk2 <http://loqui.good-day.net/>
 * Copyright (C) 2003 Yoichi Imai <yoichi@silver-forest.com>
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
#ifndef __LOQUI_TOOLBAR_H__
#define __LOQUI_TOOLBAR_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define LOQUI_TYPE_TOOLBAR                 (loqui_toolbar_get_type ())
#define LOQUI_TOOLBAR(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), LOQUI_TYPE_TOOLBAR, LoquiToolbar))
#define LOQUI_TOOLBAR_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), LOQUI_TYPE_TOOLBAR, LoquiToolbarClass))
#define LOQUI_IS_TOOLBAR(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LOQUI_TYPE_TOOLBAR))
#define LOQUI_IS_TOOLBAR_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), LOQUI_TYPE_TOOLBAR))
#define LOQUI_TOOLBAR_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), LOQUI_TYPE_TOOLBAR, LoquiToolbarClass))

typedef struct _LoquiToolbar            LoquiToolbar;
typedef struct _LoquiToolbarClass       LoquiToolbarClass;

typedef struct _LoquiToolbarPrivate     LoquiToolbarPrivate;

struct _LoquiToolbar
{
        GtkToolbar parent;
        
        LoquiToolbarPrivate *priv;

	GtkWidget *toggle_scroll;
};

struct _LoquiToolbarClass
{
        GtkToolbarClass parent_class;
};


GtkType loqui_toolbar_get_type (void) G_GNUC_CONST;

GtkWidget* loqui_toolbar_new(gpointer data);
void loqui_toolbar_set_toggle_scrolling_without_signal_emission(LoquiToolbar *toolbar, gboolean is_scroll);

G_END_DECLS

#endif /* __LOQUI_TOOLBAR_H__ */
