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
#ifndef __REMARK_ENTRY_H__
#define __REMARK_ENTRY_H__

#include <gtk/gtk.h>

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
        
        RemarkEntryPrivate *priv;
};

struct _RemarkEntryClass
{
        GtkHBoxClass parent_class;
};


GtkType remark_entry_get_type (void) G_GNUC_CONST;

GtkWidget* remark_entry_new (void);
G_CONST_RETURN gchar *remark_entry_get_text(RemarkEntry *entry);
void remark_entry_clear_text(RemarkEntry *entry);
void remark_entry_set_nick(RemarkEntry *entry, const gchar *nick);
void remark_entry_grab_focus(RemarkEntry *entry);

G_END_DECLS

#endif /* __REMARK_ENTRY_H__ */
