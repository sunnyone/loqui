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
#ifndef __LOQUI_TRANSFER_WINDOW_H__
#define __LOQUI_TRANSFER_WINDOW_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define LOQUI_TYPE_TRANSFER_WINDOW                 (loqui_transfer_window_get_type ())
#define LOQUI_TRANSFER_WINDOW(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), LOQUI_TYPE_TRANSFER_WINDOW, LoquiTransferWindow))
#define LOQUI_TRANSFER_WINDOW_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), LOQUI_TYPE_TRANSFER_WINDOW, LoquiTransferWindowClass))
#define LOQUI_IS_TRANSFER_WINDOW(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LOQUI_TYPE_TRANSFER_WINDOW))
#define LOQUI_IS_TRANSFER_WINDOW_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), LOQUI_TYPE_TRANSFER_WINDOW))
#define LOQUI_TRANSFER_WINDOW_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), LOQUI_TYPE_TRANSFER_WINDOW, LoquiTransferWindowClass))

typedef struct _LoquiTransferWindow            LoquiTransferWindow;
typedef struct _LoquiTransferWindowClass       LoquiTransferWindowClass;

typedef struct _LoquiTransferWindowPrivate     LoquiTransferWindowPrivate;

struct _LoquiTransferWindow
{
        GtkWindow parent;
        
        LoquiTransferWindowPrivate *priv;
};

struct _LoquiTransferWindowClass
{
        GtkWindowClass parent_class;
};


GType loqui_transfer_window_get_type(void) G_GNUC_CONST;

GtkWidget* loqui_transfer_window_new(void);
G_END_DECLS

#endif /* __LOQUI_TRANSFER_WINDOW_H__ */
