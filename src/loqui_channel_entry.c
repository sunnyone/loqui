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
#include "config.h"

#include "intl.h"

#include "loqui_channel_entry.h"
#include "loqui_marshalers.h"

enum {
	SIGNAL_REMOVE,
	SIGNAL_REORDERED,
	SIGNAL_INSERTED,
        LAST_SIGNAL
};

enum {
	PROP_0,
	PROP_NAME,
	PROP_TOPIC,
	PROP_IS_UPDATED,
	PROP_BUFFER,
        LAST_PROP
};

struct _LoquiChannelEntryPrivate
{
};

#define LOQUI_CHANNEL_ENTRY_DEFAULT_MEMBER_NUMBER 10

static GObjectClass *parent_class = NULL;

static guint loqui_channel_entry_signals[LAST_SIGNAL] = { 0 };

static void loqui_channel_entry_class_init(LoquiChannelEntryClass *klass);
static void loqui_channel_entry_init(LoquiChannelEntry *entry);
static void loqui_channel_entry_finalize(GObject *object);
static void loqui_channel_entry_dispose(GObject *object);

static void loqui_channel_entry_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec);
static void loqui_channel_entry_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec);

static void loqui_channel_entry_remove_user_from_hash(LoquiChannelEntry *chent, LoquiUser *user);
static void loqui_channel_entry_real_remove(LoquiChannelEntry *chent, LoquiMember *member);

GType
loqui_channel_entry_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(LoquiChannelEntryClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) loqui_channel_entry_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(LoquiChannelEntry),
				0,              /* n_preallocs */
				(GInstanceInitFunc) loqui_channel_entry_init
			};
		
		type = g_type_register_static(G_TYPE_OBJECT,
					      "LoquiChannelEntry",
					      &our_info,
					      0);
	}
	
	return type;
}
static void 
loqui_channel_entry_finalize(GObject *object)
{
	LoquiChannelEntry *entry;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL_ENTRY(object));

        entry = LOQUI_CHANNEL_ENTRY(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize)(object);

	g_free(entry->priv);
}
static void 
loqui_channel_entry_dispose(GObject *object)
{
	LoquiChannelEntry *chent;
	LoquiMember *member;
	gint i;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL_ENTRY(object));

        chent = LOQUI_CHANNEL_ENTRY(object);

	G_FREE_UNLESS_NULL(chent->name);
	G_FREE_UNLESS_NULL(chent->topic);
	G_OBJECT_UNREF_UNLESS_NULL(chent->buffer);

	for (i = 0; i < chent->member_array->len; i++) {
		member = g_array_index(chent->member_array, LoquiMember *, i);
		g_object_unref(member);
	}

        if (G_OBJECT_CLASS(parent_class)->dispose)
                (* G_OBJECT_CLASS(parent_class)->dispose)(object);
}
static void
loqui_channel_entry_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec)
{
        LoquiChannelEntry *chent;        

        chent = LOQUI_CHANNEL_ENTRY(object);

        switch (param_id) {
	case PROP_NAME:
		g_value_set_string(value, chent->name);
		break;
	case PROP_TOPIC:
		g_value_set_string(value, chent->topic);
		break;
	case PROP_IS_UPDATED:
		g_value_set_boolean(value, chent->is_updated);
		break;
	case PROP_BUFFER:
		g_value_set_object(value, chent->buffer);
		break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}
static void
loqui_channel_entry_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec)
{
        LoquiChannelEntry *chent;

        chent = LOQUI_CHANNEL_ENTRY(object);

        switch (param_id) {
	case PROP_NAME:
		loqui_channel_entry_set_name(chent, g_value_get_string(value));
		break;
	case PROP_TOPIC:
		loqui_channel_entry_set_topic(chent, g_value_get_string(value));
		break;
	case PROP_IS_UPDATED:
		loqui_channel_entry_set_is_updated(chent, g_value_get_boolean(value));
		break;
	case PROP_BUFFER:
		loqui_channel_entry_set_buffer(chent, g_value_get_object(value));
		break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}

static void
loqui_channel_entry_class_init(LoquiChannelEntryClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = loqui_channel_entry_finalize;
        object_class->dispose = loqui_channel_entry_dispose;
        object_class->get_property = loqui_channel_entry_get_property;
        object_class->set_property = loqui_channel_entry_set_property;
	klass->remove = loqui_channel_entry_real_remove;
	klass->inserted = NULL;
	klass->reordered = NULL;

	g_object_class_install_property(object_class,
					PROP_NAME,
					g_param_spec_string("name",
							    _("Name"),
							    _("Name"),
							    NULL, G_PARAM_READWRITE));
	g_object_class_install_property(object_class,
					PROP_TOPIC,
					g_param_spec_string("topic",
							    _("Topic"),
							    _("Topic"),
							    NULL, G_PARAM_READWRITE));
	g_object_class_install_property(object_class,
					PROP_IS_UPDATED,
					g_param_spec_boolean("is_updated",
							     _("Updated"),
							     _("Updated or not"),
							     FALSE, G_PARAM_READWRITE));
	g_object_class_install_property(object_class,
					PROP_BUFFER,
					g_param_spec_object("buffer",
							    _("Buffer"),
							    _("Channel buffer"),
							    TYPE_CHANNEL_BUFFER, G_PARAM_READWRITE));

	loqui_channel_entry_signals[SIGNAL_REMOVE] = g_signal_new("remove",
								  G_OBJECT_CLASS_TYPE(object_class),
								  G_SIGNAL_RUN_LAST,
								  G_STRUCT_OFFSET(LoquiChannelEntryClass, remove),
								  NULL, NULL,
								  g_cclosure_marshal_VOID__OBJECT,
								  G_TYPE_NONE, 1,
								  LOQUI_TYPE_MEMBER);
	loqui_channel_entry_signals[SIGNAL_INSERTED] = g_signal_new("inserted",
								    G_OBJECT_CLASS_TYPE(object_class),
								    G_SIGNAL_RUN_FIRST,
								    G_STRUCT_OFFSET(LoquiChannelEntryClass, inserted),
								    NULL, NULL,
								    _loqui_marshal_VOID__OBJECT_INT,
								    G_TYPE_NONE, 2,
								    LOQUI_TYPE_MEMBER,
								    G_TYPE_INT);
	loqui_channel_entry_signals[SIGNAL_REORDERED] = g_signal_new("reordered",
								     G_OBJECT_CLASS_TYPE(object_class),
								     G_SIGNAL_RUN_FIRST,
								     G_STRUCT_OFFSET(LoquiChannelEntryClass, reordered),
								     NULL, NULL,
								     g_cclosure_marshal_VOID__VOID,
								     G_TYPE_NONE, 0);
}
static void 
loqui_channel_entry_init(LoquiChannelEntry *chent)
{
	LoquiChannelEntryPrivate *priv;

	priv = g_new0(LoquiChannelEntryPrivate, 1);

	chent->priv = priv;

	chent->member_array = g_array_sized_new(FALSE, FALSE, sizeof(gpointer),
						LOQUI_CHANNEL_ENTRY_DEFAULT_MEMBER_NUMBER);
	chent->user_hash = g_hash_table_new(g_direct_hash, g_direct_equal);
}
LoquiChannelEntry*
loqui_channel_entry_new(void)
{
        LoquiChannelEntry *chent;
	LoquiChannelEntryPrivate *priv;

	chent = g_object_new(loqui_channel_entry_get_type(), NULL);
	
        priv = chent->priv;

        return chent;
}
static void
loqui_channel_entry_remove_user_from_hash(LoquiChannelEntry *chent, LoquiUser *user)
{
        g_return_if_fail(chent != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL_ENTRY(chent));

	g_hash_table_remove(chent->user_hash, user);
}
static void
loqui_channel_entry_real_remove(LoquiChannelEntry *chent, LoquiMember *member)
{
        g_return_if_fail(chent != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL_ENTRY(chent));
	
	loqui_channel_entry_remove_user_from_hash(chent, member->user);
	g_object_unref(member);
}
void
loqui_channel_entry_add(LoquiChannelEntry *chent, LoquiMember *member)
{
	LoquiChannelEntryPrivate *priv;
	gint i, pos;

        g_return_if_fail(chent != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL_ENTRY(chent));
        g_return_if_fail(member != NULL);
        g_return_if_fail(LOQUI_IS_MEMBER(member));
	
        priv = chent->priv;

	g_object_ref(member);
	if (chent->sort_func) {
		for (i = 0; i < chent->member_array->len; i++) {
			if (chent->sort_func(g_array_index(chent->member_array, LoquiMember *, i), member) > 0)
				break;
		}
		pos = i;
		if (i < chent->member_array->len)
			g_array_insert_val(chent->member_array, pos, member);
		g_array_append_val(chent->member_array, member);
	} else {
		g_array_append_val(chent->member_array, member);
		pos = chent->member_array->len - 1;
	}
	g_hash_table_insert(chent->user_hash, member->user, GINT_TO_POINTER(pos + 1));

	g_signal_emit(chent, loqui_channel_entry_signals[SIGNAL_INSERTED], 2, member, pos);
}
void
loqui_channel_entry_remove(LoquiChannelEntry *chent, LoquiUser *user)
{
	LoquiMember *member;
        g_return_if_fail(chent != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL_ENTRY(chent));
	
	member = loqui_channel_entry_get_member(chent, user);
	g_signal_emit(G_OBJECT(chent), loqui_channel_entry_signals[SIGNAL_REMOVE], 1, member);
}
LoquiMember *
loqui_channel_entry_get_member(LoquiChannelEntry *chent, LoquiUser *user)
{
	gint pos;

        g_return_val_if_fail(chent != NULL, NULL);
        g_return_val_if_fail(LOQUI_IS_CHANNEL_ENTRY(chent), NULL);
	
	pos = GPOINTER_TO_INT(g_hash_table_lookup(chent->user_hash, user)) - 1;
	if (pos < 0 || chent->member_array->len <= pos)
		return NULL;
	return g_array_index(chent->member_array, LoquiMember *, pos);
}
void
loqui_channel_entry_sort(LoquiChannelEntry *chent)
{
	LoquiChannelEntryPrivate *priv;

        g_return_if_fail(chent != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL_ENTRY(chent));

        priv = chent->priv;

	if (chent->sort_func)
		g_array_sort(chent->member_array, chent->sort_func);

	g_signal_emit(chent, loqui_channel_entry_signals[SIGNAL_REORDERED], 0);
}
void
loqui_channel_entry_set_sort_func(LoquiChannelEntry *chent, GCompareFunc func)
{
        LoquiChannelEntryPrivate *priv;

        g_return_if_fail(chent != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL_ENTRY(chent));

        priv = chent->priv;

	if (chent->sort_func == func) 
		return;

	chent->sort_func = func;
	loqui_channel_entry_sort(chent);
}
LoquiMember *
loqui_channel_entry_get_nth_member(LoquiChannelEntry *chent, gint n)
{
        g_return_val_if_fail(chent != NULL, NULL);
        g_return_val_if_fail(LOQUI_IS_CHANNEL_ENTRY(chent), NULL);
	
	if (n < 0 || chent->member_array->len >= n)
		return NULL;

	return g_array_index(chent->member_array, LoquiMember *, n);
}
gint
loqui_channel_entry_get_member_number(LoquiChannelEntry *chent)
{
        g_return_val_if_fail(chent != NULL, 0);
        g_return_val_if_fail(LOQUI_IS_CHANNEL_ENTRY(chent), 0);
	
	return chent->member_array->len;
}
void
loqui_channel_entry_set_buffer(LoquiChannelEntry *chent, ChannelBuffer *buffer)
{
        LoquiChannelEntryPrivate *priv;

        g_return_if_fail(chent != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL_ENTRY(chent));

        priv = chent->priv;

	G_OBJECT_UNREF_UNLESS_NULL(chent->buffer);

	g_object_ref(buffer);
	chent->buffer = buffer;
	g_object_notify(G_OBJECT(chent), "buffer");
}
ChannelBuffer *
loqui_channel_entry_get_buffer(LoquiChannelEntry *chent)
{
        g_return_val_if_fail(chent != NULL, 0);
        g_return_val_if_fail(LOQUI_IS_CHANNEL_ENTRY(chent), 0);

	return chent->buffer;
}
void
loqui_channel_entry_set_is_updated(LoquiChannelEntry *chent, gboolean is_updated)
{
	g_return_if_fail(chent != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL_ENTRY(chent));

	if (chent->is_updated == is_updated)
		return;

	chent->is_updated = is_updated;
	g_object_notify(G_OBJECT(chent), "is_updated");
}
gboolean
loqui_channel_entry_is_updated(LoquiChannelEntry *chent)
{
        g_return_val_if_fail(chent != NULL, 0);
        g_return_val_if_fail(LOQUI_IS_CHANNEL_ENTRY(chent), 0);

	return chent->is_updated;
}
LOQUI_CHANNEL_ENTRY_ACCESSOR_STRING(topic);
LOQUI_CHANNEL_ENTRY_ACCESSOR_STRING(name);
