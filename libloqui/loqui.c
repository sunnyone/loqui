/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * libloqui -- Chat/IM client library for GLib <http://loqui.good-day.net/>
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
#include "loqui.h"

static gboolean loqui_debug_mode = FALSE;
static gboolean loqui_show_msg_mode = FALSE;
static gboolean loqui_send_status_commands_mode = TRUE;
static gchar *loqui_user_dir = NULL;

void
loqui_set_debug_mode(gboolean debug_mode)
{
	loqui_debug_mode = debug_mode;
}
gboolean
loqui_get_debug_mode(void)
{
	return loqui_debug_mode;
}

void
loqui_set_show_msg_mode(gboolean show_msg_mode)
{
	loqui_show_msg_mode = show_msg_mode;
}

gboolean
loqui_get_show_msg_mode(void)
{
	return loqui_show_msg_mode;
}

void
loqui_set_send_status_commands_mode(gboolean send_status_commands_mode)
{
	loqui_send_status_commands_mode = send_status_commands_mode;
}
gboolean
loqui_get_send_status_commands_mode(void)
{
	return loqui_send_status_commands_mode;
}
void
loqui_set_user_dir(const gchar *path)
{
	const gchar *env_userdir;

	if (loqui_user_dir) {
		g_free(loqui_user_dir);
		loqui_user_dir = NULL;
	}
	if (path) {
		loqui_user_dir = g_strdup(path);
	} else {
		if ((env_userdir = g_getenv(LOQUI_USER_DIR_ENV_KEY)) != NULL)
			loqui_user_dir = g_strdup(env_userdir);
		else
			loqui_user_dir = g_build_filename(g_get_home_dir(), LOQUI_USER_DIR_DEFAULT_BASENAME, NULL);
	}
}
G_CONST_RETURN gchar *
loqui_get_user_dir(void)
{
	if (!loqui_user_dir)
		loqui_set_user_dir(NULL);
	return loqui_user_dir;
}
