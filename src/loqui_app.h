/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui -- IRC client for Gtk2 <http://loqui.good-day.net/>
 * Copyright (C) 2002-2003 Yoichi Imai <yoichi@silver-forest.com>
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

#ifndef __LOQUI_APP_H__
#define __LOQUI_APP_H__

#include <gtk/gtk.h>
#include "loqui-general-pref-gtk.h"

G_BEGIN_DECLS

#define LOQUI_TYPE_APP                 (loqui_app_get_type ())
#define LOQUI_APP(obj)                 (GTK_CHECK_CAST ((obj), LOQUI_TYPE_APP, LoquiApp))
#define LOQUI_APP_CLASS(klass)         (GTK_CHECK_CLASS_CAST ((klass), LOQUI_TYPE_APP, LoquiAppClass))
#define LOQUI_IS_APP(obj)              (GTK_CHECK_TYPE ((obj), LOQUI_TYPE_APP))
#define LOQUI_IS_APP_CLASS(klass)      (GTK_CHECK_CLASS_TYPE ((klass), LOQUI_TYPE_APP))
#define LOQUI_APP_GET_CLASS(obj)       (GTK_CHECK_GET_CLASS ((obj), LOQUI_TYPE_APP, LoquiAppClass))

typedef struct _LoquiApp            LoquiApp;
typedef struct _LoquiAppClass       LoquiAppClass;

typedef struct _LoquiAppPrivate     LoquiAppPrivate;

#include "nick_list.h"
#include "channel_tree.h"
#include "loqui_account_manager.h"
#include "loqui-tray-icon.h"
#include "loqui_app_info.h"
#include "loqui_channel_buffer_gtk.h"
#include "loqui-transfer-window.h"

struct _LoquiApp
{
        GtkWindow parent;

        LoquiAppPrivate *priv;

	LoquiAppInfo *appinfo;

	LoquiAccountManager *account_manager;

	LoquiChannelEntry *current_channel_entry;

	gboolean is_pending_update_account_info;
	gboolean is_pending_update_channel_info;
	
	guint updated_channel_number;
	guint updated_private_talk_number;

	guint channel_entry_id_max;

	GAsyncQueue *error_connections;

	GtkActionGroup *action_group;
	GtkUIManager *ui_manager;
	GtkActionGroup *channel_entry_group;

	GtkTooltips *tooltips;

	GtkWidget *channelbar;
	GtkWidget *statusbar;
	
	GtkWidget *remark_entry;
	
	GtkWidget *channel_notebook;
	NickList *nick_list;
	ChannelTree *channel_tree;
	GtkWidget *common_textview;

	LoquiTrayIcon *tray_icon;
	LoquiTransferWindow *transfer_window;
};

struct _LoquiAppClass
{
        GtkWindowClass parent_class;
};

GType        loqui_app_get_type             (void) G_GNUC_CONST;

GtkWidget* loqui_app_new(LoquiAccountManager *manager);

gboolean loqui_app_has_toplevel_focus(LoquiApp *app);
gboolean loqui_app_is_obscured(LoquiApp *app);

void loqui_app_grab_focus_if_key_unused(LoquiApp *app, const gchar *class_name, GdkEventKey *event);

void loqui_app_set_common_buffer(LoquiApp *app, LoquiChannelBufferGtk *buffer);

void loqui_app_set_auto_switch_scrolling_channel_buffers(LoquiApp *app, gboolean auto_switch_scrolling);
void loqui_app_set_auto_switch_scrolling_common_buffer(LoquiApp *app, gboolean auto_switch_scrolling);

void loqui_app_set_show_statusbar(LoquiApp *app, gboolean show);
void loqui_app_set_show_channelbar(LoquiApp *app, gboolean show);

void loqui_app_get_current_widget_editing_status(LoquiApp *app, gboolean *cutable, gboolean *copiable, gboolean *pastable,
						 gboolean *clearable, gboolean *findable);

LoquiAccountManager *loqui_app_get_account_manager(LoquiApp *app);

LoquiChannelEntry *loqui_app_get_current_channel_entry(LoquiApp *app);
void loqui_app_set_current_channel_entry(LoquiApp *app, LoquiChannelEntry *chent);

LoquiChannel *loqui_app_get_current_channel(LoquiApp *app);
LoquiAccount *loqui_app_get_current_account(LoquiApp *app);

GtkWidget *loqui_app_get_current_channel_text_view(LoquiApp *app);

gboolean loqui_app_is_current_account(LoquiApp *app, LoquiAccount *account);
gboolean loqui_app_is_current_channel(LoquiApp *app, LoquiChannel *channel);

void loqui_app_set_nick_list_sort_type(LoquiApp *app, PrefSortType sort_type);

G_END_DECLS

#endif /* __LOQUI_APP_H__ */
