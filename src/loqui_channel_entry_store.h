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
#ifndef __LOQUI_CHANNEL_ENTRY_STORE_H__
#define __LOQUI_CHANNEL_ENTRY_STORE_H__

#include <gtk/gtk.h>
#include "loqui_channel_entry.h"

G_BEGIN_DECLS

#define LOQUI_TYPE_CHANNEL_ENTRY_STORE                 (loqui_channel_entry_store_get_type ())
#define LOQUI_CHANNEL_ENTRY_STORE(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), LOQUI_TYPE_CHANNEL_ENTRY_STORE, LoquiChannelEntryStore))
#define LOQUI_CHANNEL_ENTRY_STORE_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), LOQUI_TYPE_CHANNEL_ENTRY_STORE, LoquiChannelEntryStoreClass))
#define LOQUI_IS_CHANNEL_ENTRY_STORE(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LOQUI_TYPE_CHANNEL_ENTRY_STORE))
#define LOQUI_IS_CHANNEL_ENTRY_STORE_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), LOQUI_TYPE_CHANNEL_ENTRY_STORE))
#define LOQUI_CHANNEL_ENTRY_STORE_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), LOQUI_TYPE_CHANNEL_ENTRY_STORE, LoquiChannelEntryStoreClass))

typedef struct _LoquiChannelEntryStore            LoquiChannelEntryStore;
typedef struct _LoquiChannelEntryStoreClass       LoquiChannelEntryStoreClass;

typedef struct _LoquiChannelEntryStorePrivate     LoquiChannelEntryStorePrivate;
enum {
	LOQUI_CHANNEL_ENTRY_STORE_COLUMN_MEMBER,
	LOQUI_CHANNEL_ENTRY_STORE_COLUMN_BASIC_AWAY,
	LOQUI_CHANNEL_ENTRY_STORE_COLUMN_POWER,
	LOQUI_CHANNEL_ENTRY_STORE_COLUMN_NICK,
	LOQUI_CHANNEL_ENTRY_STORE_N_COLUMNS,
};

struct _LoquiChannelEntryStore
{
        GObject parent;
        
	LoquiChannelEntry *chent;

	GType column_types[LOQUI_CHANNEL_ENTRY_STORE_N_COLUMNS];

	gint stamp;

        LoquiChannelEntryStorePrivate *priv;
};

struct _LoquiChannelEntryStoreClass
{
        GObjectClass parent_class;
};


GType loqui_channel_entry_store_get_type(void) G_GNUC_CONST;

LoquiChannelEntryStore* loqui_channel_entry_store_new(LoquiChannelEntry *chent);

/* void loqui_channel_entry_store_set_entry(LoquiChannelEntryStore *store, LoquiChannelEntry *chent);
   LoquiChannelEntry *loqui_channel_entry_store_get_entry(LoquiChannelEntryStore *store); */

G_END_DECLS

#endif /* __LOQUI_CHANNEL_ENTRY_STORE_H__ */
