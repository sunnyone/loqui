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
#ifndef __ACCOUNT_DIALOG_H__
#define __ACCOUNT_DIALOG_H__

#include <gtk/gtk.h>
#include <loqui_account.h>
#include <loqui_account_manager.h>

G_BEGIN_DECLS

#define TYPE_ACCOUNT_DIALOG                 (account_dialog_get_type ())
#define ACCOUNT_DIALOG(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_ACCOUNT_DIALOG, AccountDialog))
#define ACCOUNT_DIALOG_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_ACCOUNT_DIALOG, AccountDialogClass))
#define IS_ACCOUNT_DIALOG(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_ACCOUNT_DIALOG))
#define IS_ACCOUNT_DIALOG_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_ACCOUNT_DIALOG))
#define ACCOUNT_DIALOG_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_ACCOUNT_DIALOG, AccountDialogClass))

typedef struct _AccountDialog            AccountDialog;
typedef struct _AccountDialogClass       AccountDialogClass;

typedef struct _AccountDialogPrivate     AccountDialogPrivate;

struct _AccountDialog
{
        GtkDialog parent;
        
        AccountDialogPrivate *priv;
};

struct _AccountDialogClass
{
        GtkDialogClass parent_class;
};


GtkType account_dialog_get_type (void) G_GNUC_CONST;

GtkWidget* account_dialog_new(LoquiProfileAccount *profile);

void account_dialog_open_add_dialog(GtkWindow *parent, LoquiAccountManager *manager);
void account_dialog_open_configure_dialog(GtkWindow *parent, LoquiAccountManager *manager, LoquiAccount *account);
void account_dialog_open_remove_dialog(GtkWindow *parent, LoquiAccountManager *manager, LoquiAccount *account);

G_END_DECLS

#endif /* __ACCOUNT_DIALOG_H__ */
