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
#include "config.h"

#include <libloqui-intl.h>

#include "loqui_channel_entry.h"
#include "loqui-notifier.h"
#include "loqui.h"

#include "loqui-static-core.h"
#include "loqui-general-pref-default.h"
#include "loqui-general-pref-groups.h"

enum {
	SIGNAL_ADD,
	SIGNAL_REMOVE,
	SIGNAL_APPEND_MESSAGE_TEXT,
	SIGNAL_REORDERED,
        LAST_SIGNAL
};

enum {
	PROP_0,
	PROP_NAME,
	PROP_TOPIC,
	PROP_IS_UPDATED,
	PROP_IS_UPDATED_WEAK,
	PROP_HAS_UNREAD_KEYWORD,
	PROP_BUFFER,
	PROP_MEMBER_NUMBER,
	PROP_OP_NUMBER,
	PROP_POSITION,
	PROP_ID,
	PROP_DO_SORT,
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

static void loqui_channel_entry_add_real(LoquiChannelEntry *chent, LoquiMember *member);
static void loqui_channel_entry_remove_real(LoquiChannelEntry *chent, LoquiMember *member);
static void loqui_channel_entry_append_message_text_real(LoquiChannelEntry *chent, LoquiMessageText *msgtext);

static void loqui_channel_entry_member_notify_is_channel_operator_cb(LoquiMember *member, GParamSpec *pspec, LoquiChannelEntry *chent);

static void loqui_channel_entry_member_notify_cb(LoquiMember *member, GParamSpec *pspec, LoquiChannelEntry *chent);
static void loqui_channel_entry_user_notify_cb(LoquiUser *user, GParamSpec *pspec, LoquiChannelEntry *chent);
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
	GSList *member_slist = NULL, *cur;
	gint i;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL_ENTRY(object));

        chent = LOQUI_CHANNEL_ENTRY(object);

	LOQUI_G_FREE_UNLESS_NULL(chent->name);
	LOQUI_G_FREE_UNLESS_NULL(chent->topic);
	LOQUI_G_OBJECT_UNREF_UNLESS_NULL(chent->buffer);

	if (chent->member_ptr_array) {
		for (i = 0; i < chent->member_ptr_array->len; i++) {
			member_slist = g_slist_append(member_slist, g_ptr_array_index(chent->member_ptr_array, i));
		}
		for (cur = member_slist; cur != NULL; cur = cur->next) {
			member = cur->data;
			g_signal_emit(G_OBJECT(chent), loqui_channel_entry_signals[SIGNAL_REMOVE], 0, member);
		}
		g_slist_free(member_slist);
		g_ptr_array_free(chent->member_ptr_array, TRUE);
		chent->member_ptr_array = NULL;
	}
	if (chent->user_hash) {
		g_hash_table_destroy(chent->user_hash);
		chent->user_hash = NULL;
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
	case PROP_IS_UPDATED_WEAK:
		g_value_set_boolean(value, chent->is_updated_weak);
		break;
	case PROP_HAS_UNREAD_KEYWORD:
		g_value_set_boolean(value, chent->has_unread_keyword);
		break;
	case PROP_BUFFER:
		g_value_set_object(value, chent->buffer);
		break;
	case PROP_OP_NUMBER:
		g_value_set_int(value, chent->op_number);
		break;
	case PROP_MEMBER_NUMBER:
		g_value_set_int(value, loqui_channel_entry_get_member_number(chent));
		break;
	case PROP_POSITION:
		g_value_set_int(value, chent->position);
		break;
	case PROP_ID:
		g_value_set_int(value, chent->id);
		break;
	case PROP_DO_SORT:
		g_value_set_boolean(value, chent->do_sort);
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
	case PROP_IS_UPDATED_WEAK:
		loqui_channel_entry_set_is_updated_weak(chent, g_value_get_boolean(value));
		break;
	case PROP_HAS_UNREAD_KEYWORD:
		loqui_channel_entry_set_has_unread_keyword(chent, g_value_get_boolean(value));
		break;
	case PROP_BUFFER:
		loqui_channel_entry_set_buffer(chent, g_value_get_object(value));
		break;
	case PROP_POSITION:
		loqui_channel_entry_set_position(chent, g_value_get_int(value));
		break;
	case PROP_ID:
		loqui_channel_entry_set_id(chent, g_value_get_int(value));
		break;
	case PROP_DO_SORT:
		loqui_channel_entry_set_do_sort(chent, g_value_get_boolean(value));
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
	klass->remove = loqui_channel_entry_remove_real;
	klass->add = loqui_channel_entry_add_real;
	klass->append_message_text = loqui_channel_entry_append_message_text_real;
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
					PROP_IS_UPDATED_WEAK,
					g_param_spec_boolean("is_updated_weak",
							     _("Updated weak"),
							     _("Updated weak or not"),
							     FALSE, G_PARAM_READWRITE));
	g_object_class_install_property(object_class,
					PROP_HAS_UNREAD_KEYWORD,
					g_param_spec_boolean("has_unread_keyword",
							     _("Has unread keyword"),
							     _("Has unread keyword or not"),
							     FALSE, G_PARAM_READWRITE));
	g_object_class_install_property(object_class,
					PROP_BUFFER,
					g_param_spec_object("buffer",
							    _("Buffer"),
							    _("Channel buffer"),
							    G_TYPE_OBJECT, /* LOQUI_TYPE_CHANNEL_BUFFER */
							    G_PARAM_READWRITE));
	g_object_class_install_property(object_class,
					PROP_OP_NUMBER,
					g_param_spec_int("op_number",
							 _("Op Number"),
							 _("the number of channel operators"),
							 0, G_MAXINT,
							 0, G_PARAM_READABLE));
	g_object_class_install_property(object_class,
					PROP_MEMBER_NUMBER,
					g_param_spec_int("member_number",
							 _("Member Number"),
							 _("the number of members"),
							 0, G_MAXINT,
							 0, G_PARAM_READABLE));
	g_object_class_install_property(object_class,
					PROP_POSITION,
					g_param_spec_int("position",
							 _("Position"),
							 _("Position of all channel entries"),
							 -1, G_MAXINT,
							 -1, G_PARAM_READABLE));
	g_object_class_install_property(object_class,
					PROP_ID,
					g_param_spec_int("id",
							 _("ChannelEntryID"),
							 _("ID of channel entry."),
							 -1, G_MAXINT,
							 -1, G_PARAM_READABLE));
	g_object_class_install_property(object_class,
					PROP_DO_SORT,
					g_param_spec_boolean("do_sort",
							     _("Do sort"),
							     _("Do sort or don't sort"),
							     TRUE, G_PARAM_READWRITE));

	loqui_channel_entry_signals[SIGNAL_ADD] = g_signal_new("add",
							       G_OBJECT_CLASS_TYPE(object_class),
							       G_SIGNAL_RUN_LAST,
							       G_STRUCT_OFFSET(LoquiChannelEntryClass, add),
							       NULL, NULL,
							       g_cclosure_marshal_VOID__OBJECT,
							       G_TYPE_NONE, 1,
							       LOQUI_TYPE_MEMBER);
	loqui_channel_entry_signals[SIGNAL_REMOVE] = g_signal_new("remove",
								  G_OBJECT_CLASS_TYPE(object_class),
								  G_SIGNAL_RUN_LAST,
								  G_STRUCT_OFFSET(LoquiChannelEntryClass, remove),
								  NULL, NULL,
								  g_cclosure_marshal_VOID__OBJECT,
								  G_TYPE_NONE, 1,
								  LOQUI_TYPE_MEMBER);
	loqui_channel_entry_signals[SIGNAL_APPEND_MESSAGE_TEXT] = g_signal_new("append_message_text",
									       G_OBJECT_CLASS_TYPE(object_class),
									       G_SIGNAL_RUN_LAST,
									       G_STRUCT_OFFSET(LoquiChannelEntryClass, append_message_text),
									       NULL, NULL,
									       g_cclosure_marshal_VOID__OBJECT,
									       G_TYPE_NONE, 1,
									       LOQUI_TYPE_MESSAGE_TEXT);
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

	chent->member_ptr_array = g_ptr_array_sized_new(LOQUI_CHANNEL_ENTRY_DEFAULT_MEMBER_NUMBER);
	chent->user_hash = g_hash_table_new(g_direct_hash, g_direct_equal);
	
	chent->op_number = 0;
	chent->position = -1;
	chent->id = -1;
	chent->do_sort = TRUE;
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
loqui_channel_entry_update_user_hash(LoquiChannelEntry *chent)
{
	LoquiMember *mcur;
	gint i;

	for (i = 0; i < chent->member_ptr_array->len; i++) {
		mcur = g_ptr_array_index(chent->member_ptr_array, i);
		g_hash_table_replace(chent->user_hash, mcur->user, GINT_TO_POINTER(i + 1));
	}
}
static void
loqui_channel_entry_add_real(LoquiChannelEntry *chent, LoquiMember *member)
{
	LoquiChannelEntryPrivate *priv;

        g_return_if_fail(chent != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL_ENTRY(chent));
        g_return_if_fail(member != NULL);
        g_return_if_fail(LOQUI_IS_MEMBER(member));
	
        priv = chent->priv;

	if (g_hash_table_lookup(chent->user_hash, member->user) != 0) {
                gchar *identifier = loqui_user_get_identifier(member->user);

		g_warning("The user (%s) is already registered to '%s'.",
                          identifier,
                          loqui_channel_entry_get_name(chent));

                g_free(identifier);
		return;
	}

	g_object_ref(member);
	g_ptr_array_add(chent->member_ptr_array, member);
	if (chent->sort_func && chent->do_sort) {
		loqui_utils_g_ptr_array_insert_sort(chent->member_ptr_array, chent->member_ptr_array->len - 1, chent->sort_func);
		loqui_channel_entry_update_user_hash(chent);
	} else {
		g_hash_table_insert(chent->user_hash, member->user, GINT_TO_POINTER(chent->member_ptr_array->len - 1 + 1));
	}

	if (loqui_member_get_is_channel_operator(member)) {
		chent->op_number++;
		g_object_notify(G_OBJECT(chent), "op_number");
	}
	g_signal_connect(G_OBJECT(member), "notify::is-channel-operator",
			 G_CALLBACK(loqui_channel_entry_member_notify_is_channel_operator_cb), chent);
	g_signal_connect(G_OBJECT(member->user), "notify",
			 G_CALLBACK(loqui_channel_entry_user_notify_cb), chent);
	g_signal_connect(G_OBJECT(member), "notify",
			 G_CALLBACK(loqui_channel_entry_member_notify_cb), chent);

	g_object_notify(G_OBJECT(chent), "member-number");
}
static void
loqui_channel_entry_remove_real(LoquiChannelEntry *chent, LoquiMember *member)
{
	gint old_pos;

        g_return_if_fail(chent != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL_ENTRY(chent));

	if (loqui_member_get_is_channel_operator(member)) {
		chent->op_number--;
		g_object_notify(G_OBJECT(chent), "op_number");
	}

	old_pos = GPOINTER_TO_INT(g_hash_table_lookup(chent->user_hash, member->user)) - 1;
	g_return_if_fail(old_pos >= 0);
	g_ptr_array_remove_index(chent->member_ptr_array, old_pos);

	g_hash_table_remove(chent->user_hash, member->user);
	loqui_channel_entry_update_user_hash(chent);

	g_signal_handlers_disconnect_by_func(G_OBJECT(member),
					     loqui_channel_entry_member_notify_is_channel_operator_cb, chent);
	g_signal_handlers_disconnect_by_func(G_OBJECT(member->user),
					     loqui_channel_entry_user_notify_cb, chent);
	g_signal_handlers_disconnect_by_func(G_OBJECT(member),
					     loqui_channel_entry_member_notify_cb, chent);

	g_object_notify(G_OBJECT(chent), "member-number");
	g_object_unref(member);
}
static void
loqui_channel_entry_append_message_text_real(LoquiChannelEntry *chent, LoquiMessageText *msgtext)
{
	LoquiChannelBuffer *buffer;
	LoquiTextType text_type;
	g_return_if_fail(chent != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL_ENTRY(chent));
	
	loqui_notifier_process_message_text(loqui_core_get_notifier(loqui_get_core()), msgtext);

	if (loqui_message_text_get_is_remark(msgtext) && !loqui_message_text_get_is_ignored(msgtext)) {
		if (loqui_message_text_get_text_type(msgtext) == LOQUI_TEXT_TYPE_NOTICE)
			loqui_channel_entry_set_is_updated_weak(chent, TRUE);
		else
			loqui_channel_entry_set_is_updated(chent, TRUE);
		
		text_type = loqui_message_text_get_text_type(msgtext);
		if (loqui_message_text_get_has_highlight_keyword(msgtext) &&
		    (text_type == LOQUI_TEXT_TYPE_NORMAL ||
		     (text_type == LOQUI_TEXT_TYPE_NOTICE &&
		      loqui_pref_get_with_default_boolean(loqui_get_general_pref(),
							  LOQUI_GENERAL_PREF_GROUP_NOTIFICATION, "ExecNotificationByNotice",
							  LOQUI_GENERAL_PREF_DEFAULT_NOTIFICATION_EXEC_NOTIFICATION_BY_NOTICE, NULL)))) {
			loqui_channel_entry_set_has_unread_keyword(chent, TRUE);
		}
	}

	buffer = loqui_channel_entry_get_buffer(chent);
	if (buffer) {
		loqui_channel_buffer_append_message_text(buffer, msgtext);
	}
}
static void
loqui_channel_entry_member_notify_is_channel_operator_cb(LoquiMember *member, GParamSpec *pspec, LoquiChannelEntry *chent)
{
	if (loqui_member_get_is_channel_operator(member)) {
		chent->op_number++;
	} else {
		chent->op_number--;
	}
	g_object_notify(G_OBJECT(chent), "op-number");
}
static void
loqui_channel_entry_member_notify_cb(LoquiMember *member, GParamSpec *pspec, LoquiChannelEntry *chent)
{
	if (chent->sort_func && chent->do_sort) {
		g_ptr_array_remove_index(chent->member_ptr_array, loqui_channel_entry_get_member_pos(chent, member));
		g_ptr_array_add(chent->member_ptr_array, member);
		loqui_utils_g_ptr_array_insert_sort(chent->member_ptr_array, chent->member_ptr_array->len - 1, chent->sort_func);
		loqui_channel_entry_update_user_hash(chent);
		g_signal_emit(chent, loqui_channel_entry_signals[SIGNAL_REORDERED], 0);
	}
}
static void
loqui_channel_entry_user_notify_cb(LoquiUser *user, GParamSpec *pspec, LoquiChannelEntry *chent)
{
	gint pos;
	LoquiMember *member;

	if (chent->sort_func && chent->do_sort) {
		pos = GPOINTER_TO_INT(g_hash_table_lookup(chent->user_hash, user)) - 1;
		g_assert(pos >= 0);
		member = g_ptr_array_index(chent->member_ptr_array, pos);
		g_ptr_array_remove_index(chent->member_ptr_array, pos);
		g_ptr_array_add(chent->member_ptr_array, member);
		loqui_utils_g_ptr_array_insert_sort(chent->member_ptr_array, chent->member_ptr_array->len - 1, chent->sort_func);
		loqui_channel_entry_update_user_hash(chent);
		g_signal_emit(chent, loqui_channel_entry_signals[SIGNAL_REORDERED], 0);
	}
}
void
loqui_channel_entry_add_member(LoquiChannelEntry *chent, LoquiMember *member)
{
	LoquiChannelEntryPrivate *priv;

        g_return_if_fail(chent != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL_ENTRY(chent));
        g_return_if_fail(member != NULL);
        g_return_if_fail(LOQUI_IS_MEMBER(member));
	
        priv = chent->priv;

	g_signal_emit(chent, loqui_channel_entry_signals[SIGNAL_ADD], 0, member);
}
void
loqui_channel_entry_remove_member_by_user(LoquiChannelEntry *chent, LoquiUser *user)
{
	LoquiMember *member;
        g_return_if_fail(chent != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL_ENTRY(chent));
	g_return_if_fail(user != NULL);

	member = loqui_channel_entry_get_member_by_user(chent, user);
	g_return_if_fail(member != NULL);

	g_signal_emit(G_OBJECT(chent), loqui_channel_entry_signals[SIGNAL_REMOVE], 0, member);
}
void
loqui_channel_entry_clear_member(LoquiChannelEntry *chent)
{
	LoquiMember *member;
	GList *list = NULL, *cur;
	int i, num;
	
	num = loqui_channel_entry_get_member_number(chent);
	for (i = 0; i < num; i++)
		list = g_list_append(list, loqui_channel_entry_get_nth_member(chent, i));
	for (cur = list; cur != NULL; cur = cur->next) {
		member = LOQUI_MEMBER(cur->data);
		loqui_channel_entry_remove_member_by_user(chent, member->user);
	}
	g_list_free(list);
}

LoquiMember *
loqui_channel_entry_get_member_by_user(LoquiChannelEntry *chent, LoquiUser *user)
{
	gint pos;

        g_return_val_if_fail(chent != NULL, NULL);
        g_return_val_if_fail(LOQUI_IS_CHANNEL_ENTRY(chent), NULL);
	
	pos = GPOINTER_TO_INT(g_hash_table_lookup(chent->user_hash, user)) - 1;
	if (pos < 0 || chent->member_ptr_array->len <= pos)
		return NULL;
	return g_ptr_array_index(chent->member_ptr_array, pos);
}
void
loqui_channel_entry_sort(LoquiChannelEntry *chent)
{
	LoquiChannelEntryPrivate *priv;

        g_return_if_fail(chent != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL_ENTRY(chent));

        priv = chent->priv;

	if (chent->sort_func)
		g_ptr_array_sort(chent->member_ptr_array, chent->sort_func);

	loqui_channel_entry_update_user_hash(chent);

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
	
	if (n < 0 || chent->member_ptr_array->len <= n) {
		return NULL;
	}

	return g_ptr_array_index(chent->member_ptr_array, n);
}
gint
loqui_channel_entry_get_member_number(LoquiChannelEntry *chent)
{
        g_return_val_if_fail(chent != NULL, 0);
        g_return_val_if_fail(LOQUI_IS_CHANNEL_ENTRY(chent), 0);
	
	return chent->member_ptr_array->len;
}
gint
loqui_channel_entry_get_member_pos(LoquiChannelEntry *chent, LoquiMember *member)
{
	gint pos;

        g_return_val_if_fail(chent != NULL, -1);
        g_return_val_if_fail(LOQUI_IS_CHANNEL_ENTRY(chent), -1);
	g_return_val_if_fail(LOQUI_IS_MEMBER(member), -1);

	pos = GPOINTER_TO_INT(g_hash_table_lookup(chent->user_hash, member->user)) - 1;
	return pos;
}
void
loqui_channel_entry_set_buffer(LoquiChannelEntry *chent, LoquiChannelBuffer *buffer)
{
        LoquiChannelEntryPrivate *priv;

        g_return_if_fail(chent != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL_ENTRY(chent));

        priv = chent->priv;

	LOQUI_G_OBJECT_UNREF_UNLESS_NULL(chent->buffer);

	g_object_ref(buffer);
	chent->buffer = buffer;
	g_object_notify(G_OBJECT(chent), "buffer");
}
LoquiChannelBuffer *
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
loqui_channel_entry_get_is_updated(LoquiChannelEntry *chent)
{
        g_return_val_if_fail(chent != NULL, FALSE);
        g_return_val_if_fail(LOQUI_IS_CHANNEL_ENTRY(chent), FALSE);

	return chent->is_updated;
}
void
loqui_channel_entry_set_is_updated_weak(LoquiChannelEntry *chent, gboolean is_updated_weak)
{
	g_return_if_fail(chent != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL_ENTRY(chent));

	if (chent->is_updated_weak == is_updated_weak)
		return;

	chent->is_updated_weak = is_updated_weak;
	g_object_notify(G_OBJECT(chent), "is_updated_weak");
}
gboolean
loqui_channel_entry_get_is_updated_weak(LoquiChannelEntry *chent)
{
        g_return_val_if_fail(chent != NULL, FALSE);
        g_return_val_if_fail(LOQUI_IS_CHANNEL_ENTRY(chent), FALSE);

	return chent->is_updated_weak;
}
void
loqui_channel_entry_set_has_unread_keyword(LoquiChannelEntry *chent, gboolean has_unread_keyword)
{
	g_return_if_fail(chent != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL_ENTRY(chent));

	if (chent->has_unread_keyword == has_unread_keyword)
		return;

	chent->has_unread_keyword = has_unread_keyword;
	g_object_notify(G_OBJECT(chent), "has_unread_keyword");
}
gboolean
loqui_channel_entry_get_has_unread_keyword(LoquiChannelEntry *chent)
{
        g_return_val_if_fail(chent != NULL, FALSE);
        g_return_val_if_fail(LOQUI_IS_CHANNEL_ENTRY(chent), FALSE);

	return chent->has_unread_keyword;
}
void
loqui_channel_entry_set_as_read(LoquiChannelEntry *chent)
{
	g_return_if_fail(chent != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL_ENTRY(chent));

	loqui_channel_entry_set_is_updated(chent, FALSE);
	loqui_channel_entry_set_is_updated_weak(chent, FALSE);
	loqui_channel_entry_set_has_unread_keyword(chent, FALSE);
}
gboolean
loqui_channel_entry_get_whether_unread(LoquiChannelEntry *chent)
{
	return (loqui_channel_entry_get_is_updated(chent) ||
		loqui_channel_entry_get_is_updated_weak(chent) ||
		loqui_channel_entry_get_has_unread_keyword(chent));
}
void
loqui_channel_entry_set_do_sort(LoquiChannelEntry *chent, gboolean do_sort)
{
	g_return_if_fail(chent != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL_ENTRY(chent));

	if (chent->do_sort == do_sort)
		return;

	chent->do_sort = do_sort;
	if (do_sort)
		loqui_channel_entry_sort(chent);

	g_object_notify(G_OBJECT(chent), "do_sort");
}
gboolean
loqui_channel_entry_get_do_sort(LoquiChannelEntry *chent)
{
        g_return_val_if_fail(chent != NULL, FALSE);
        g_return_val_if_fail(LOQUI_IS_CHANNEL_ENTRY(chent), FALSE);

	return chent->do_sort;
}

LOQUI_DEFINE_READER_GENERIC(LoquiChannelEntry, loqui_channel_entry, op_number, gint);

LOQUI_DEFINE_ACCESSOR_GENERIC(LoquiChannelEntry, loqui_channel_entry, position, gint);
LOQUI_DEFINE_ACCESSOR_GENERIC(LoquiChannelEntry, loqui_channel_entry, id, gint);

LOQUI_DEFINE_ACCESSOR_CONST_STRING(LoquiChannelEntry, loqui_channel_entry, name);
LOQUI_DEFINE_ACCESSOR_CONST_STRING(LoquiChannelEntry, loqui_channel_entry, topic);

void
loqui_channel_entry_append_message_text(LoquiChannelEntry *chent, LoquiMessageText *msgtext)
{
	g_return_if_fail(chent != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL_ENTRY(chent));

	g_signal_emit(G_OBJECT(chent), loqui_channel_entry_signals[SIGNAL_APPEND_MESSAGE_TEXT], 0, msgtext);
}
