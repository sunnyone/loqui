/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#ifndef __LOQUI_MEMBER_SORT_FUNCS_H__
#define __LOQUI_MEMBER_SORT_FUNCS_H__

#include "loqui_member.h"

gint loqui_member_sort_funcs_nick(LoquiMember *ma, LoquiMember *mb);
gint loqui_member_sort_funcs_power(LoquiMember *ma, LoquiMember *mb);
gint loqui_member_sort_funcs_last_message_time(LoquiMember *ma, LoquiMember *mb);
gint loqui_member_sort_funcs_away(LoquiMember *ma, LoquiMember *mb);
gint loqui_member_sort_funcs_nick_power_away(LoquiMember *ma, LoquiMember *mb);
gint loqui_member_sort_funcs_away_power_nick(LoquiMember *ma, LoquiMember *mb);
gint loqui_member_sort_funcs_time_away_power_nick(LoquiMember *ma, LoquiMember *mb);
gint loqui_member_sort_funcs_time_nick_power_away(LoquiMember *ma, LoquiMember *mb);

#endif /* __LOQUI_MEMBER_SORT_FUNCS_H__ */
