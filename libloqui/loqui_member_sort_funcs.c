/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#include "config.h"
#include "loqui_member_sort_funcs.h"
#include "loqui_member.h"
#include <string.h>

#define MEMBER_GET_POWER_SCORE(member, sc_ptr) { \
  if (member->is_channel_operator) \
     *sc_ptr = 2; \
  else if (member->speakable) \
     *sc_ptr = 1; \
  else \
     *sc_ptr = 0; \
}

gint
loqui_member_sort_funcs_nick(LoquiMember **ma_p, LoquiMember **mb_p)
{
	return strcmp((*ma_p)->user->nick_key, (*mb_p)->user->nick_key);
}
gint
loqui_member_sort_funcs_power(LoquiMember **ma_p, LoquiMember **mb_p)
{
	gint score_a, score_b;

	MEMBER_GET_POWER_SCORE((*ma_p), &score_a);
	MEMBER_GET_POWER_SCORE((*mb_p), &score_b);
	return score_b - score_a;
}
gint
loqui_member_sort_funcs_last_message_time(LoquiMember **ma_p, LoquiMember **mb_p)
{
	return (*mb_p)->last_message_time - (*ma_p)->last_message_time;
}
gint
loqui_member_sort_funcs_away(LoquiMember **ma_p, LoquiMember **mb_p)
{
	return (*mb_p)->user->away - (*ma_p)->user->away;
}
gint
loqui_member_sort_funcs_power_nick(LoquiMember **ma_p, LoquiMember **mb_p)
{
	register gint i;

	if ((i = loqui_member_sort_funcs_power(ma_p, mb_p)) != 0)
		return i;
	return loqui_member_sort_funcs_nick(ma_p, mb_p);
}
gint
loqui_member_sort_funcs_away_nick(LoquiMember **ma_p, LoquiMember **mb_p)
{
	register gint i;

	if ((i = loqui_member_sort_funcs_away(ma_p, mb_p)) != 0)
		return i;
	return loqui_member_sort_funcs_nick(ma_p, mb_p);
}
gint
loqui_member_sort_funcs_power_away_nick(LoquiMember **ma_p, LoquiMember **mb_p)
{
	register gint i;

	if ((i = loqui_member_sort_funcs_power(ma_p, mb_p)) != 0)
		return i;
	if ((i = loqui_member_sort_funcs_away(ma_p, mb_p)) != 0)
		return i;
	return loqui_member_sort_funcs_nick(ma_p, mb_p);
}
gint
loqui_member_sort_funcs_away_power_nick(LoquiMember **ma_p, LoquiMember **mb_p)
{
	register gint i;

	if ((i = loqui_member_sort_funcs_away(ma_p, mb_p)) != 0)
		return i;
	if ((i = loqui_member_sort_funcs_power(ma_p, mb_p)) != 0)
		return i;
	return loqui_member_sort_funcs_nick(ma_p, mb_p);
}
gint
loqui_member_sort_funcs_time_away_power_nick(LoquiMember **ma_p, LoquiMember **mb_p)
{
	register gint i;

	if ((i = loqui_member_sort_funcs_last_message_time(ma_p, mb_p)) != 0)
		return i;
	if ((i = loqui_member_sort_funcs_away(ma_p, mb_p)) != 0)
		return i;
	if ((i = loqui_member_sort_funcs_power(ma_p, mb_p)) != 0)
		return i;
	return loqui_member_sort_funcs_nick(ma_p, mb_p);
}
gint
loqui_member_sort_funcs_time_power_away_nick(LoquiMember **ma_p, LoquiMember **mb_p)
{
	register gint i;

	if ((i = loqui_member_sort_funcs_last_message_time(ma_p, mb_p)) != 0)
		return i;
	if ((i = loqui_member_sort_funcs_power(ma_p, mb_p)) != 0)
		return i;
	if ((i = loqui_member_sort_funcs_away(ma_p, mb_p)) != 0)
		return i;
	return loqui_member_sort_funcs_nick(ma_p, mb_p);
}
gint
loqui_member_sort_funcs_time_nick(LoquiMember **ma_p, LoquiMember **mb_p)
{
	register gint i;

	if ((i = loqui_member_sort_funcs_last_message_time(ma_p, mb_p)) != 0)
		return i;
	return loqui_member_sort_funcs_nick(ma_p, mb_p);
}

