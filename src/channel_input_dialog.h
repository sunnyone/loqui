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
#ifndef __CHANNEL_INPUT_DIALOG_H__
#define __CHANNEL_INPUT_DIALOG_H__

#include <gtk/gtk.h>

#include "account.h"
#include "account_manager.h"

G_BEGIN_DECLS

#define TYPE_CHANNEL_INPUT_DIALOG                 (channel_input_dialog_get_type ())
#define CHANNEL_INPUT_DIALOG(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_CHANNEL_INPUT_DIALOG, ChannelInputDialog))
#define CHANNEL_INPUT_DIALOG_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_CHANNEL_INPUT_DIALOG, ChannelInputDialogClass))
#define IS_CHANNEL_INPUT_DIALOG(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_CHANNEL_INPUT_DIALOG))
#define IS_CHANNEL_INPUT_DIALOG_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_CHANNEL_INPUT_DIALOG))
#define CHANNEL_INPUT_DIALOG_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_CHANNEL_INPUT_DIALOG, ChannelInputDialogClass))

typedef struct _ChannelInputDialog            ChannelInputDialog;
typedef struct _ChannelInputDialogClass       ChannelInputDialogClass;

typedef struct _ChannelInputDialogPrivate     ChannelInputDialogPrivate;

#include "loqui_app.h"

typedef enum {
	CHANNEL_HISTORY_NONE,
	CHANNEL_HISTORY_JOINED,
	CHANNEL_HISTORY_SAVED /* same as NONE currently */
} ChannelHistoryType;

typedef void (* ChannelInputFunc) (Account *account, const gchar *channel_text,
				   const gchar *text, gpointer data);

struct _ChannelInputDialog
{
        GtkDialog parent;

	GtkWidget *label;
	GtkWidget *option_menu;
	GtkWidget *combo;
	GtkWidget *entry;

        ChannelInputDialogPrivate *priv;
};

struct _ChannelInputDialogClass
{
        GtkDialogClass parent_class;
};


GtkType channel_input_dialog_get_type (void) G_GNUC_CONST;

GtkWidget* channel_input_dialog_new(AccountManager *manager);

void channel_input_dialog_set_account(ChannelInputDialog *dialog, Account *account);
Account *channel_input_dialog_get_account(ChannelInputDialog *dialog);

void channel_input_dialog_set_channel_history_type(ChannelInputDialog *dialog,
						   ChannelHistoryType history_type);
ChannelHistoryType channel_input_dialog_get_channel_history_type(ChannelInputDialog *dialog);
void channel_input_dialog_open(LoquiApp *app,
			       const gchar *title, const gchar *info_label,
			       ChannelHistoryType history_type, 
			       ChannelInputFunc func, gpointer data,
			       gboolean use_account, Account *account, 
			       gboolean use_channel, const gchar *channel_name, 
			       gboolean use_text, const gchar *default_text);

G_END_DECLS

#endif /* __CHANNEL_INPUT_DIALOG_H__ */
