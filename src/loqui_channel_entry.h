/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * libloqui -- Chat/IM library with GNet/GObject <http://loqui.good-day.net/>
 * Copyright (C) 2004 Yoichi Imai <yoichi@silver-forest.com>
 *
 * This Library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with the Gnome Library; see the file COPYING.LIB.  If not,
 * write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#ifndef __LOQUI_CHANNEL_ENTRY_H__
#define __LOQUI_CHANNEL_ENTRY_H__

#include <glib-object.h>
#include "loqui_user.h"
#include "loqui_member.h"

G_BEGIN_DECLS

#define LOQUI_TYPE_CHANNEL_ENTRY                 (loqui_channel_entry_get_type ())
#define LOQUI_CHANNEL_ENTRY(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), LOQUI_TYPE_CHANNEL_ENTRY, LoquiChannelEntry))
#define LOQUI_CHANNEL_ENTRY_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), LOQUI_TYPE_CHANNEL_ENTRY, LoquiChannelEntryClass))
#define LOQUI_IS_CHANNEL_ENTRY(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LOQUI_TYPE_CHANNEL_ENTRY))
#define LOQUI_IS_CHANNEL_ENTRY_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), LOQUI_TYPE_CHANNEL_ENTRY))
#define LOQUI_CHANNEL_ENTRY_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), LOQUI_TYPE_CHANNEL_ENTRY, LoquiChannelEntryClass))

typedef struct _LoquiChannelEntry            LoquiChannelEntry;
typedef struct _LoquiChannelEntryClass       LoquiChannelEntryClass;

typedef struct _LoquiChannelEntryPrivate     LoquiChannelEntryPrivate;

struct _LoquiChannelEntry
{
        GObject parent;
        
	GArray *member_array;
	GHashTable *user_hash; /* key: user, value: position + 1 */

	GCompareFunc sort_func;

        LoquiChannelEntryPrivate *priv;
};

struct _LoquiChannelEntryClass
{
        GObjectClass parent_class;
	
	/* RUN_LAST */
	void (* remove) (LoquiChannelEntry *chent, LoquiMember *member);

	/* RUN_FIRST */
	void (* inserted) (LoquiChannelEntry *chent, LoquiMember *member, gint pos);
	void (* reordered) (LoquiChannelEntry *chent);
};

GType loqui_channel_entry_get_type(void) G_GNUC_CONST;

LoquiChannelEntry* loqui_channel_entry_new(void);

void loqui_channel_entry_add(LoquiChannelEntry *chent, LoquiMember *member);
void loqui_channel_entry_remove(LoquiChannelEntry *chent, LoquiUser *user);
LoquiMember *loqui_channel_entry_get_member(LoquiChannelEntry *entry, LoquiUser *user);

void loqui_channel_entry_set_sort_func(LoquiChannelEntry *chent, GCompareFunc func);
void loqui_channel_entry_sort(LoquiChannelEntry *chent);

G_END_DECLS

#endif /* __LOQUI_CHANNEL_ENTRY_H__ */
