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
#ifndef __CONNECT_DIALOG_H__
#define __CONNECT_DIALOG_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define TYPE_CONNECT_DIALOG                 (connect_dialog_get_type ())
#define CONNECT_DIALOG(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_CONNECT_DIALOG, ConnectDialog))
#define CONNECT_DIALOG_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_CONNECT_DIALOG, ConnectDialogClass))
#define IS_CONNECT_DIALOG(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_CONNECT_DIALOG))
#define IS_CONNECT_DIALOG_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_CONNECT_DIALOG))
#define CONNECT_DIALOG_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_CONNECT_DIALOG, ConnectDialogClass))

typedef struct _ConnectDialog            ConnectDialog;
typedef struct _ConnectDialogClass       ConnectDialogClass;

typedef struct _ConnectDialogPrivate     ConnectDialogPrivate;

struct _ConnectDialog
{
        GtkDialog parent;
        
        ConnectDialogPrivate *priv;
};

struct _ConnectDialogClass
{
        GtkDialogClass parent_class;
};


GtkType connect_dialog_get_type (void) G_GNUC_CONST;

GtkWidget* connect_dialog_new (void);
void connect_dialog_open(GtkWindow *parent_window);

G_END_DECLS

#endif /* __CONNECT_DIALOG_H__ */
