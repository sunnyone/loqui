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
#ifndef __PREFS_DIALOG_H__
#define __PREFS_DIALOG_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define TYPE_PREFS_DIALOG                 (prefs_dialog_get_type ())
#define PREFS_DIALOG(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_PREFS_DIALOG, PrefsDialog))
#define PREFS_DIALOG_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_PREFS_DIALOG, PrefsDialogClass))
#define IS_PREFS_DIALOG(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_PREFS_DIALOG))
#define IS_PREFS_DIALOG_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_PREFS_DIALOG))
#define PREFS_DIALOG_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_PREFS_DIALOG, PrefsDialogClass))

typedef struct _PrefsDialog            PrefsDialog;
typedef struct _PrefsDialogClass       PrefsDialogClass;

typedef struct _PrefsDialogPrivate     PrefsDialogPrivate;

struct _PrefsDialog
{
        GtkDialog parent;
        
        PrefsDialogPrivate *priv;
};

struct _PrefsDialogClass
{
        GtkDialogClass parent_class;
};


GtkType prefs_dialog_get_type (void) G_GNUC_CONST;

GtkWidget* prefs_dialog_new (void);

void prefs_dialog_open(GtkWindow *parent);

G_END_DECLS

#endif /* __PREFS_DIALOG_H__ */
