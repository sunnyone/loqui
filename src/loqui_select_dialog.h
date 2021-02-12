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
#ifndef __LOQUI_SELECT_DIALOG_H__
#define __LOQUI_SELECT_DIALOG_H__

#include <gtk/gtk.h>
#include "loqui_app.h"

G_BEGIN_DECLS

#define LOQUI_TYPE_SELECT_DIALOG                 (loqui_select_dialog_get_type ())
#define LOQUI_SELECT_DIALOG(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), LOQUI_TYPE_SELECT_DIALOG, LoquiSelectDialog))
#define LOQUI_SELECT_DIALOG_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), LOQUI_TYPE_SELECT_DIALOG, LoquiSelectDialogClass))
#define LOQUI_IS_SELECT_DIALOG(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LOQUI_TYPE_SELECT_DIALOG))
#define LOQUI_IS_SELECT_DIALOG_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), LOQUI_TYPE_SELECT_DIALOG))
#define LOQUI_SELECT_DIALOG_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), LOQUI_TYPE_SELECT_DIALOG, LoquiSelectDialogClass))

typedef struct _LoquiSelectDialog            LoquiSelectDialog;
typedef struct _LoquiSelectDialogClass       LoquiSelectDialogClass;

typedef struct _LoquiSelectDialogPrivate     LoquiSelectDialogPrivate;

struct _LoquiSelectDialog
{
        GtkDialog parent;
        
        LoquiSelectDialogPrivate *priv;
};

struct _LoquiSelectDialogClass
{
        GtkDialogClass parent_class;
};


GType loqui_select_dialog_get_type(void) G_GNUC_CONST;

GtkWidget* loqui_select_dialog_new(LoquiApp *app);
void loqui_select_dialog_construct_channel_entry_list(LoquiSelectDialog *sdialog);

G_END_DECLS

#endif /* __LOQUI_SELECT_DIALOG_H__ */
