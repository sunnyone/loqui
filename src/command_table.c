/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui -- IRC client for Gtk2 <http://loqui.good-day.net/>
 * Copyright (C) 2002-2003 Yoichi Imai <yoichi@silver-forest.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#include "config.h"
#include "command_table.h"

typedef struct {
	gchar *command;
	IRCResponse response;
} CommandItem;

static CommandItem command_table[] = {
	{IRCCommandPing, IRC_COMMAND_PING},
	{IRCCommandPrivmsg, IRC_COMMAND_PRIVMSG},
	{IRCCommandJoin, IRC_COMMAND_JOIN},
	{IRCCommandNotice, IRC_COMMAND_NOTICE},
	{IRCCommandTopic, IRC_COMMAND_TOPIC},
	{IRCCommandMode, IRC_COMMAND_MODE},
	{IRCCommandNick, IRC_COMMAND_NICK},
	{IRCCommandPart, IRC_COMMAND_PART},
	{IRCCommandQuit, IRC_COMMAND_QUIT},
	{IRCCommandKick, IRC_COMMAND_KICK},
	{IRCCommandInvite, IRC_COMMAND_INVITE},
	{IRCCommandError, IRC_COMMAND_ERROR},
	{IRCCommandPass, IRC_COMMAND_PASS},
	{IRCCommandNick, IRC_COMMAND_NICK},
	{IRCCommandUser, IRC_COMMAND_USER},
	{IRCCommandOper, IRC_COMMAND_OPER},
	{IRCCommandSQuit, IRC_COMMAND_SQUIT},
	{IRCCommandStats, IRC_COMMAND_STATS},
	{IRCCommandLinks, IRC_COMMAND_LINKS},
	{IRCCommandTime, IRC_COMMAND_TIME},
	{IRCCommandConnect, IRC_COMMAND_CONNECT},
	{IRCCommandTrace, IRC_COMMAND_TRACE},
	{IRCCommandAdmin, IRC_COMMAND_ADMIN},
	{IRCCommandInfo, IRC_COMMAND_INFO},
	{IRCCommandNotice, IRC_COMMAND_NOTICE},
	{IRCCommandWho, IRC_COMMAND_WHO},
	{IRCCommandWhois, IRC_COMMAND_WHOIS},
	{IRCCommandWhowas, IRC_COMMAND_WHOWAS},
	{IRCCommandKill, IRC_COMMAND_KILL},
	{IRCCommandPing, IRC_COMMAND_PING},
	{IRCCommandPong, IRC_COMMAND_PONG},
	{IRCCommandAway, IRC_COMMAND_AWAY},
	{IRCCommandRehash, IRC_COMMAND_REHASH},
	{IRCCommandRestart, IRC_COMMAND_RESTART},
	{IRCCommandSummon, IRC_COMMAND_SUMMON},
	{IRCCommandUsers, IRC_COMMAND_USERS},
	{IRCCommandUserhost, IRC_COMMAND_USERHOST},
	{IRCCommandIson, IRC_COMMAND_ISON},
	{IRCCommandObject, IRC_COMMAND_OBJECT},
	{NULL, 0}
};

static GHashTable *command_hash = NULL;

void
command_table_init(void)
{
	int i;
	if(!command_hash) {
		command_hash = g_hash_table_new(g_str_hash, g_str_equal);
		for(i = 0; command_table[i].command != NULL; i++) {
			g_hash_table_insert(command_hash,
					    (gpointer) command_table[i].command,
					    GINT_TO_POINTER(command_table[i].response));
		}
	}
}

IRCResponse
command_table_make_command_numeric(const gchar *command)
{
	g_return_val_if_fail(command_hash != NULL, 0);
	return GPOINTER_TO_INT(g_hash_table_lookup(command_hash, command));
}

