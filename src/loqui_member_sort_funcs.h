/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#ifndef __LOQUI_MEMBER_SORT_FUNCS_H__
#define __LOQUI_MEMBER_SORT_FUNCS_H__

#include "loqui_member.h"

gint loqui_member_sort_funcs_nick(LoquiMember **ma_p, LoquiMember **mb_p);
gint loqui_member_sort_funcs_power(LoquiMember **ma_p, LoquiMember **mb_p);
gint loqui_member_sort_funcs_last_message_time(LoquiMember **ma_p, LoquiMember **mb_p);
gint loqui_member_sort_funcs_away(LoquiMember **ma_p, LoquiMember **mb_p);
gint loqui_member_sort_funcs_time_nick(LoquiMember **ma_p, LoquiMember **mb_p);
gint loqui_member_sort_funcs_nick_away_power(LoquiMember **ma_p, LoquiMember **mb_p);
gint loqui_member_sort_funcs_away_power_nick(LoquiMember **ma_p, LoquiMember **mb_p);
gint loqui_member_sort_funcs_time_away_power_nick(LoquiMember **ma_p, LoquiMember **mb_p);
gint loqui_member_sort_funcs_time_nick_away_power(LoquiMember **ma_p, LoquiMember **mb_p);

#endif /* __LOQUI_MEMBER_SORT_FUNCS_H__ */
