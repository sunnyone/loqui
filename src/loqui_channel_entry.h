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

#include "channel_buffer.h"

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

	gboolean is_updated;
	gchar *name;
	gchar *topic;

	ChannelBuffer *buffer;

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

void loqui_channel_entry_add_member(LoquiChannelEntry *chent, LoquiMember *member);
void loqui_channel_entry_remove_member_by_user(LoquiChannelEntry *chent, LoquiUser *user);
void loqui_channel_entry_clear_member(LoquiChannelEntry *chent);
LoquiMember *loqui_channel_entry_get_member_by_user(LoquiChannelEntry *entry, LoquiUser *user);

void loqui_channel_entry_set_sort_func(LoquiChannelEntry *chent, GCompareFunc func);
void loqui_channel_entry_sort(LoquiChannelEntry *chent);

LoquiMember *loqui_channel_entry_get_nth_member(LoquiChannelEntry *chent, gint n);
gint loqui_channel_entry_get_member_number(LoquiChannelEntry *chent);

void loqui_channel_entry_set_buffer(LoquiChannelEntry *chent, ChannelBuffer *buffer);
ChannelBuffer *loqui_channel_entry_get_buffer(LoquiChannelEntry *chent);

void loqui_channel_entry_set_is_updated(LoquiChannelEntry *chent, gboolean is_updated);
gboolean loqui_channel_entry_get_is_updated(LoquiChannelEntry *chent);

#define LOQUI_CHANNEL_ENTRY_ACCESSOR_STRING(attr_name) \
  ATTR_ACCESSOR_POINTER(g_strdup, g_free, const gchar *, G_CONST_RETURN gchar *, LoquiChannelEntry, loqui_channel_entry, attr_name)
#define LOQUI_CHANNEL_ENTRY_ACCESSOR_STRING_PROTOTYPE(attr_name) \
  ATTR_ACCESSOR_POINTER_PROTOTYPE(const gchar *, G_CONST_RETURN gchar *, LoquiChannelEntry, loqui_channel_entry, attr_name)

#define LOQUI_CHANNEL_ENTRY_ACCESSOR_GENERIC(type, attr_name) \
  ATTR_ACCESSOR_GENERIC(type, 0, LoquiChannelEntry, loqui_channel_entry, attr_name)
#define LOQUI_CHANNEL_ENTRY_ACCESSOR_GENERIC_PROTOTYPE(type, attr_name) \
  ATTR_ACCESSOR_GENERIC_PROTOTYPE(type, LoquiChannelEntry, loqui_channel_entry, attr_name)

LOQUI_CHANNEL_ENTRY_ACCESSOR_STRING_PROTOTYPE(name);
LOQUI_CHANNEL_ENTRY_ACCESSOR_STRING_PROTOTYPE(topic);

G_END_DECLS

#endif /* __LOQUI_CHANNEL_ENTRY_H__ */
