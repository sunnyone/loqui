/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * libloqui -- Chat/IM client library for GLib
 * Copyright (C) 2002-2004 Yoichi Imai <sunnyone41@gmail.com>
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

#include "loqui_channel_buffer.h"
#include <libloqui/loqui-message-text.h>

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
        
	GPtrArray *member_ptr_array;
	GHashTable *user_hash; /* key: user, value: position + 1 */

	gboolean is_updated;
	gboolean is_updated_weak; /* for NOTICE */
	gboolean has_unread_keyword;

	gchar *name;
	gchar *topic;

	LoquiChannelBuffer *buffer;

	GCompareFunc sort_func;

	gint op_number; /* cache */

	gint id;
	gint position;

	gboolean do_sort;

        LoquiChannelEntryPrivate *priv;
};

struct _LoquiChannelEntryClass
{
        GObjectClass parent_class;
	
	/* RUN_LAST */
	void (* add) (LoquiChannelEntry *chent, LoquiMember *member);
	void (* remove) (LoquiChannelEntry *chent, LoquiMember *member);
	void (* append_message_text) (LoquiChannelEntry *chent, LoquiMessageText *msgtext);

	/* RUN_FIRST */
	void (* reordered) (LoquiChannelEntry *chent);
};

GType loqui_channel_entry_get_type(void) G_GNUC_CONST;

LoquiChannelEntry* loqui_channel_entry_new(void);

void loqui_channel_entry_add_member(LoquiChannelEntry *chent, LoquiMember *member);
void loqui_channel_entry_remove_member_by_user(LoquiChannelEntry *chent, LoquiUser *user);
void loqui_channel_entry_clear_member(LoquiChannelEntry *chent);
LoquiMember *loqui_channel_entry_get_member_by_user(LoquiChannelEntry *entry, LoquiUser *user);
gint loqui_channel_entry_get_member_pos(LoquiChannelEntry *chent, LoquiMember *member);

void loqui_channel_entry_set_sort_func(LoquiChannelEntry *chent, GCompareFunc func);
void loqui_channel_entry_sort(LoquiChannelEntry *chent);

LoquiMember *loqui_channel_entry_get_nth_member(LoquiChannelEntry *chent, gint n);
gint loqui_channel_entry_get_member_number(LoquiChannelEntry *chent);

void loqui_channel_entry_set_buffer(LoquiChannelEntry *chent, LoquiChannelBuffer *buffer);
LoquiChannelBuffer *loqui_channel_entry_get_buffer(LoquiChannelEntry *chent);

void loqui_channel_entry_set_is_updated(LoquiChannelEntry *chent, gboolean is_updated);
gboolean loqui_channel_entry_get_is_updated(LoquiChannelEntry *chent);

void loqui_channel_entry_set_is_updated_weak(LoquiChannelEntry *chent, gboolean is_updated_weak);
gboolean loqui_channel_entry_get_is_updated_weak(LoquiChannelEntry *chent);

void loqui_channel_entry_set_has_unread_keyword(LoquiChannelEntry *chent, gboolean has_unread_keyword);
gboolean loqui_channel_entry_get_has_unread_keyword(LoquiChannelEntry *chent);

void loqui_channel_entry_set_as_read(LoquiChannelEntry *chent);
gboolean loqui_channel_entry_get_whether_unread(LoquiChannelEntry *chent);

void loqui_channel_entry_set_do_sort(LoquiChannelEntry *chent, gboolean do_sort);
gboolean loqui_channel_entry_get_do_sort(LoquiChannelEntry *chent);

LOQUI_DEFINE_READER_GENERIC_PROTOTYPE(LoquiChannelEntry, loqui_channel_entry, op_number, gint);

LOQUI_DEFINE_ACCESSOR_GENERIC_PROTOTYPE(LoquiChannelEntry, loqui_channel_entry, position, gint);
LOQUI_DEFINE_ACCESSOR_GENERIC_PROTOTYPE(LoquiChannelEntry, loqui_channel_entry, id, gint);

LOQUI_DEFINE_ACCESSOR_CONST_STRING_PROTOTYPE(LoquiChannelEntry, loqui_channel_entry, name);
LOQUI_DEFINE_ACCESSOR_CONST_STRING_PROTOTYPE(LoquiChannelEntry, loqui_channel_entry, topic);

void loqui_channel_entry_append_message_text(LoquiChannelEntry *chent, LoquiMessageText *msgtext);

G_END_DECLS

#endif /* __LOQUI_CHANNEL_ENTRY_H__ */
