/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui -- Chat client for Gtk2 <http://launchpad.net/loqui/>
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
#ifndef __LOQUI_DROPDOWN_BOX_H__
#define __LOQUI_DROPDOWN_BOX_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define LOQUI_TYPE_DROPDOWN_BOX                 (loqui_dropdown_box_get_type ())
#define LOQUI_DROPDOWN_BOX(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), LOQUI_TYPE_DROPDOWN_BOX, LoquiDropdownBox))
#define LOQUI_DROPDOWN_BOX_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), LOQUI_TYPE_DROPDOWN_BOX, LoquiDropdownBoxClass))
#define LOQUI_IS_DROPDOWN_BOX(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LOQUI_TYPE_DROPDOWN_BOX))
#define LOQUI_IS_DROPDOWN_BOX_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), LOQUI_TYPE_DROPDOWN_BOX))
#define LOQUI_DROPDOWN_BOX_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), LOQUI_TYPE_DROPDOWN_BOX, LoquiDropdownBoxClass))

typedef struct _LoquiDropdownBox            LoquiDropdownBox;
typedef struct _LoquiDropdownBoxClass       LoquiDropdownBoxClass;

typedef struct _LoquiDropdownBoxPrivate     LoquiDropdownBoxPrivate;

struct _LoquiDropdownBox
{
        GtkHBox parent;
        
	GtkWidget *drop_button;
	GtkWidget *drop_widget;

	GtkWidget *menu;

        LoquiDropdownBoxPrivate *priv;
};

struct _LoquiDropdownBoxClass
{
        GtkHBoxClass parent_class;
};


GType loqui_dropdown_box_get_type(void) G_GNUC_CONST;

/* if null, arrow is used */
GtkWidget* loqui_dropdown_box_new(GtkWidget *drop_widget);

void loqui_dropdown_box_set_menu(LoquiDropdownBox *dbox, GtkWidget *menu);
GtkWidget *loqui_dropdown_box_get_menu(LoquiDropdownBox *dbox);
void loqui_dropdown_box_remove_menu(LoquiDropdownBox *dbox);

G_END_DECLS

#endif /* __LOQUI_DROPDOWN_BOX_H__ */
