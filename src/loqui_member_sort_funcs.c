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
loqui_member_sort_funcs_nick(LoquiMember *ma, LoquiMember *mb)
{
	return strcmp(ma->user->nick, mb->user->nick);
}
gint
loqui_member_sort_funcs_power(LoquiMember *ma, LoquiMember *mb)
{
	gint score_a, score_b;

	MEMBER_GET_POWER_SCORE(ma, &score_a);
	MEMBER_GET_POWER_SCORE(mb, &score_b);
	return score_b - score_a;
}
gint
loqui_member_sort_funcs_last_message_time(LoquiMember *ma, LoquiMember *mb)
{
	return ma->last_message_time - mb->last_message_time;
}
gint
loqui_member_sort_funcs_away(LoquiMember *ma, LoquiMember *mb)
{
	return mb->user->away - ma->user->away;
}

gint
loqui_member_sort_funcs_nick_power_away(LoquiMember *ma, LoquiMember *mb)
{
	return loqui_member_sort_funcs_nick(ma, mb) ||
		loqui_member_sort_funcs_power(ma, mb) ||
		loqui_member_sort_funcs_away(ma, mb);
}
gint
loqui_member_sort_funcs_away_power_nick(LoquiMember *ma, LoquiMember *mb)
{
	return loqui_member_sort_funcs_away(ma, mb) ||
		loqui_member_sort_funcs_power(ma, mb) ||
		loqui_member_sort_funcs_nick(ma, mb);
}

gint
loqui_member_sort_funcs_time_away_power_nick(LoquiMember *ma, LoquiMember *mb)
{
	return loqui_member_sort_funcs_last_message_time(ma, mb) ||
		loqui_member_sort_funcs_away(ma, mb) ||
		loqui_member_sort_funcs_power(ma, mb) ||
		loqui_member_sort_funcs_nick(ma, mb);
}
gint
loqui_member_sort_funcs_time_nick_power_away(LoquiMember *ma, LoquiMember *mb)
{
	return loqui_member_sort_funcs_last_message_time(ma, mb) ||
		loqui_member_sort_funcs_nick(ma, mb) ||
		loqui_member_sort_funcs_power(ma, mb) ||
		loqui_member_sort_funcs_away(ma, mb);
}

