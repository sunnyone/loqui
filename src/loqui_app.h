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

#include "channel_tree.h"
#include "nick_list.h"

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

#include "loqui_menu.h"

struct _LoquiApp
{
        GtkWindow parent;

        LoquiAppPrivate *priv;

	GAsyncQueue *error_connections;

	LoquiMenu *menu;
	GtkWidget *channelbar;
	GtkWidget *statusbar;
	
	GtkWidget *remark_entry;
	
	GtkWidget *channel_textview;
	NickList *nick_list;
	ChannelTree *channel_tree;
};

struct _LoquiAppClass
{
        GtkWindowClass parent_class;
};

GType        loqui_app_get_type             (void) G_GNUC_CONST;

GtkWidget*      loqui_app_new                 (void);

void loqui_app_update_info(LoquiApp *app, 
			   gboolean is_account_changed, Account *account,
			   gboolean is_channel_changed, Channel *channel);

void loqui_app_set_focus(LoquiApp *app);

void loqui_app_set_channel_buffer(LoquiApp *app, ChannelBuffer *buffer);
ChannelBuffer *loqui_app_get_channel_buffer(LoquiApp *app);

void loqui_app_set_common_buffer(LoquiApp *app, ChannelBuffer *buffer);

void loqui_app_set_show_statusbar(LoquiApp *app, gboolean show);
void loqui_app_set_show_channelbar(LoquiApp *app, gboolean show);

void loqui_app_scroll_channel_buffer(LoquiApp *app);
void loqui_app_scroll_common_buffer(LoquiApp *app);

void loqui_app_scroll_page_channel_buffer(LoquiApp *app, gint pages);
void loqui_app_scroll_page_common_buffer(LoquiApp *app, gint pages);

void loqui_app_get_current_widget_editing_status(LoquiApp *app, gboolean *cutable, gboolean *copiable, gboolean *pastable,
						 gboolean *clearable, gboolean *findable);

void loqui_app_current_widget_cut(LoquiApp *app);
void loqui_app_current_widget_copy(LoquiApp *app);
void loqui_app_current_widget_paste(LoquiApp *app);
void loqui_app_current_widget_clear(LoquiApp *app);

void loqui_app_current_widget_find(LoquiApp *app);
void loqui_app_current_widget_find_next(LoquiApp *app);

G_END_DECLS

#endif /* __LOQUI_APP_H__ */
