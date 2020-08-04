/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui -- IRC client for Gtk2 <http://launchpad.net/loqui/>
 * Copyright (C) 2003 Yoichi Imai <sunnyone41@gmail.com>
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
#ifndef __REMARK_ENTRY_H__
#define __REMARK_ENTRY_H__

#include <gtk/gtk.h>
#include "loqui_app.h"

G_BEGIN_DECLS

#define TYPE_REMARK_ENTRY                 (remark_entry_get_type ())
#define REMARK_ENTRY(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_REMARK_ENTRY, RemarkEntry))
#define REMARK_ENTRY_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_REMARK_ENTRY, RemarkEntryClass))
#define IS_REMARK_ENTRY(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_REMARK_ENTRY))
#define IS_REMARK_ENTRY_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_REMARK_ENTRY))
#define REMARK_ENTRY_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_REMARK_ENTRY, RemarkEntryClass))

typedef struct _RemarkEntry            RemarkEntry;
typedef struct _RemarkEntryClass       RemarkEntryClass;

typedef struct _RemarkEntryPrivate     RemarkEntryPrivate;

struct _RemarkEntry
{
        GtkHBox parent;

        GtkWidget *entry;

        RemarkEntryPrivate *priv;
};

struct _RemarkEntryClass
{
        GtkHBoxClass parent_class;

	void (* call_history)(RemarkEntry *entry,
			      gint count);
	void (* scroll_channel_textview)(RemarkEntry *entry,
					 gint pages);
	void (* scroll_common_textview)(RemarkEntry *entry,
					gint pages);
        void (* complete_nick)(RemarkEntry *entry);
};

GType remark_entry_get_type (void) G_GNUC_CONST;

GtkWidget* remark_entry_new(LoquiApp *app, GtkToggleAction *toggle_command_action);
G_CONST_RETURN gchar *remark_entry_get_text(RemarkEntry *entry);
void remark_entry_clear_text(RemarkEntry *entry);

void remark_entry_set_multiline(RemarkEntry *entry, gboolean is_multiline);
gboolean remark_entry_get_multiline(RemarkEntry *entry);

void remark_entry_set_command_mode(RemarkEntry *entry, gboolean command_mode);
gboolean remark_entry_get_command_mode(RemarkEntry *entry);

G_END_DECLS

#endif /* __REMARK_ENTRY_H__ */
