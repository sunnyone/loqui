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
#ifndef __ACCOUNT_LIST_DIALOG_H__
#define __ACCOUNT_LIST_DIALOG_H__

#include <gtk/gtk.h>
#include <loqui_account_manager.h>

G_BEGIN_DECLS

#define TYPE_ACCOUNT_LIST_DIALOG                 (account_list_dialog_get_type ())
#define ACCOUNT_LIST_DIALOG(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_ACCOUNT_LIST_DIALOG, AccountListDialog))
#define ACCOUNT_LIST_DIALOG_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_ACCOUNT_LIST_DIALOG, AccountListDialogClass))
#define IS_ACCOUNT_LIST_DIALOG(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_ACCOUNT_LIST_DIALOG))
#define IS_ACCOUNT_LIST_DIALOG_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_ACCOUNT_LIST_DIALOG))
#define ACCOUNT_LIST_DIALOG_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_ACCOUNT_LIST_DIALOG, AccountListDialogClass))

typedef struct _AccountListDialog            AccountListDialog;
typedef struct _AccountListDialogClass       AccountListDialogClass;

typedef struct _AccountListDialogPrivate     AccountListDialogPrivate;

struct _AccountListDialog
{
        GtkDialog parent;
        
        AccountListDialogPrivate *priv;
};

struct _AccountListDialogClass
{
        GtkDialogClass parent_class;
};

#define ACCOUNT_LIST_DIALOG_RESPONSE_CONNECT 1

GtkType account_list_dialog_get_type (void) G_GNUC_CONST;

GtkWidget* account_list_dialog_new(LoquiAccountManager *manager, gboolean with_connect_button);

void account_list_dialog_open(GtkWindow *parent, LoquiAccountManager *manager);
void account_list_dialog_open_for_connect(GtkWindow *parent, LoquiAccountManager *manager);

G_END_DECLS

#endif /* __ACCOUNT_LIST_DIALOG_H__ */
