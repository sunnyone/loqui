/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * libloqui -- Chat/IM client library for GLib <http://launchpad.net/loqui/>
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

#include "loqui_member.h"

enum {
        LAST_SIGNAL
};

enum {
	PROP_0,
	PROP_USER,
	PROP_IS_CHANNEL_OPERATOR,
	PROP_SPEAKABLE,
	PROP_LAST_MESSAGE_TIME,
        LAST_PROP
};

static GObjectClass *parent_class = NULL;

/* static guint loqui_member_signals[LAST_SIGNAL] = { 0 }; */

static void loqui_member_class_init(LoquiMemberClass *klass);
static void loqui_member_init(LoquiMember *member);
static void loqui_member_finalize(GObject *object);
static void loqui_member_dispose(GObject *object);

static void loqui_member_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec);
static void loqui_member_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec);

GType
loqui_member_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(LoquiMemberClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) loqui_member_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(LoquiMember),
				0,              /* n_preallocs */
				(GInstanceInitFunc) loqui_member_init
			};
		
		type = g_type_register_static(G_TYPE_OBJECT,
					      "LoquiMember",
					      &our_info,
					      0);
	}
	
	return type;
}
static void 
loqui_member_finalize(GObject *object)
{
	LoquiMember *member;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_MEMBER(object));

        member = LOQUI_MEMBER(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize)(object);
}
static void 
loqui_member_dispose(GObject *object)
{
	LoquiMember *member;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_MEMBER(object));

        member = LOQUI_MEMBER(object);

	LOQUI_G_OBJECT_UNREF_UNLESS_NULL(member->user);

        if (G_OBJECT_CLASS(parent_class)->dispose)
                (* G_OBJECT_CLASS(parent_class)->dispose)(object);
}
static void
loqui_member_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec)
{
        LoquiMember *member;        

        member = LOQUI_MEMBER(object);

        switch (param_id) {
	case PROP_USER:
		g_value_set_object(value, member->user);
		break;
	case PROP_IS_CHANNEL_OPERATOR:
		g_value_set_boolean(value, member->is_channel_operator);
		break;
	case PROP_SPEAKABLE:
		g_value_set_boolean(value, member->speakable);
		break;
	case PROP_LAST_MESSAGE_TIME:
		g_value_set_long(value, member->last_message_time);
		break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}
static void
loqui_member_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec)
{
        LoquiMember *member;        

        member = LOQUI_MEMBER(object);
        switch (param_id) {
	case PROP_USER:
		loqui_member_set_user(member, g_value_get_object(value));
		break;
	case PROP_IS_CHANNEL_OPERATOR:
		loqui_member_set_is_channel_operator(member, g_value_get_boolean(value));
		break;
	case PROP_SPEAKABLE:
		loqui_member_set_speakable(member, g_value_get_boolean(value));
		break;
	case PROP_LAST_MESSAGE_TIME:
		loqui_member_set_last_message_time(member, (time_t) g_value_get_long(value));
		break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}
static void
loqui_member_class_init(LoquiMemberClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = loqui_member_finalize;
        object_class->dispose = loqui_member_dispose;
        object_class->get_property = loqui_member_get_property;
        object_class->set_property = loqui_member_set_property;

	g_object_class_install_property(object_class,
					PROP_USER,
					g_param_spec_object("user",
							    _("User"),
							    _("User"),
							    LOQUI_TYPE_USER, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

	g_object_class_install_property(object_class,
					PROP_IS_CHANNEL_OPERATOR,
					g_param_spec_boolean("is_channel_operator",
							     _("ChannelOperator"),
							     _("ChannelOperator"),
							     FALSE, G_PARAM_READWRITE));
	g_object_class_install_property(object_class,
					PROP_SPEAKABLE,
					g_param_spec_boolean("speakable",
							     _("Speakable or not"),
							     _("Has voice privillege or not"),
							     FALSE, G_PARAM_READWRITE));
	g_object_class_install_property(object_class,
					PROP_LAST_MESSAGE_TIME,
					g_param_spec_long("last_message_time",
							 _("Last message time"),
							 _("Last message time"),
							 G_MINLONG,
							 G_MAXLONG,
							 0, G_PARAM_READWRITE));
}
static void 
loqui_member_init(LoquiMember *member)
{
}
LoquiMember*
loqui_member_new(LoquiUser *user)
{
        LoquiMember *member;

	member = g_object_new(loqui_member_get_type(), "user", user, NULL);
	
        return member;
}
void
loqui_member_set_user(LoquiMember *member, LoquiUser *user)
{
        g_return_if_fail(member != NULL);
        g_return_if_fail(LOQUI_IS_MEMBER(member));

	LOQUI_G_OBJECT_UNREF_UNLESS_NULL(member->user);

	if (user) {
		g_object_ref(user);
		member->user = user;
	}
	g_object_notify(G_OBJECT(member), "user");
}
LoquiUser*
loqui_member_get_user(LoquiMember *member)
{
        g_return_val_if_fail(member != NULL, NULL);
        g_return_val_if_fail(LOQUI_IS_MEMBER(member), NULL);

	return member->user;
}
void
loqui_member_set_is_channel_operator(LoquiMember *member, gboolean is_channel_operator)
{
	g_return_if_fail(member != NULL);
        g_return_if_fail(LOQUI_IS_MEMBER(member));

	if (member->is_channel_operator == is_channel_operator)
		return;

	member->is_channel_operator = is_channel_operator;
	g_object_notify(G_OBJECT(member), "is_channel_operator");
}
gboolean
loqui_member_get_is_channel_operator(LoquiMember *member)
{
        g_return_val_if_fail(member != NULL, 0);
        g_return_val_if_fail(LOQUI_IS_MEMBER(member), 0);

	return member->is_channel_operator;
}
void
loqui_member_set_speakable(LoquiMember *member, gboolean speakable)
{
	g_return_if_fail(member != NULL);
        g_return_if_fail(LOQUI_IS_MEMBER(member));

	if (member->speakable == speakable)
		return;

	member->speakable = speakable;
	g_object_notify(G_OBJECT(member), "speakable");
}
gboolean
loqui_member_get_speakable(LoquiMember *member)
{
        g_return_val_if_fail(member != NULL, 0);
        g_return_val_if_fail(LOQUI_IS_MEMBER(member), 0);

	return member->speakable;
}
void
loqui_member_set_last_message_time(LoquiMember *member, time_t last_message_time)
{
	g_return_if_fail(member != NULL);
        g_return_if_fail(LOQUI_IS_MEMBER(member));

	member->last_message_time = last_message_time;
	g_object_notify(G_OBJECT(member), "last_message_time");
}
time_t
loqui_member_get_last_message_time(LoquiMember *member)
{
        g_return_val_if_fail(member != NULL, 0);
        g_return_val_if_fail(LOQUI_IS_MEMBER(member), 0);

	return member->last_message_time;
}
