/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * libloqui -- Chat/IM client library for GLib
 * Copyright (C) 2004 Yoichi Imai <sunnyone41@gmail.com>
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
#include "loqui_webutils.h"

static gboolean curl_readable_cb(GIOChannel *ioch, GIOCondition condition, gpointer data)
{
	gchar *text;
	gsize len;
	GError *error = NULL;
	GCClosure *closure;
	LoquiWebCallback cb;

	closure = data;

	cb = closure->callback;
	data = ((GClosure *) closure)->data;
	
	if (!g_io_channel_read_to_end(ioch, &text, &len, &error)) {
		cb(NULL, -1, error, data);
	}
	
	cb(text, len, NULL, data);

	g_closure_unref((GClosure *) closure);

	return FALSE;
}

void
loqui_webutils_get(gchar *url, GList *header_list, LoquiWebUtilsGetType get_type, LoquiWebCallback cb, gpointer data)
{
	gint stdout = -1;
	GError *error = NULL;
	GIOChannel *ioch;
	GClosure *closure;
	GList *arguments = NULL, *cur;
	gchar **argv;
	gint i;

	arguments = g_list_append(arguments, "curl");

	switch (get_type) {
	case WEBUTILS_GET_HEADER:
		arguments = g_list_append(arguments, "-I");
		break;
	case WEBUTILS_GET_HEADER_AND_BODY:
		arguments = g_list_append(arguments, "-i");
		break;
	case WEBUTILS_GET_BODY:
	default:
		break;
	}

	for (cur = header_list; cur != NULL; cur = cur->next) {
		arguments = g_list_append(arguments, "-H");
		arguments = g_list_append(arguments, cur->data);
	}

	arguments = g_list_append(arguments, url);

	argv = g_new0(gchar *, g_list_length(arguments) + 1);
	for (cur = arguments, i = 0; cur != NULL; cur = cur->next, i++)
		argv[i] = cur->data;
	
	if (!g_spawn_async_with_pipes(NULL,
				      argv,
				      NULL,
				      G_SPAWN_SEARCH_PATH | G_SPAWN_STDERR_TO_DEV_NULL,
				      NULL,
				      NULL,
				      
				      NULL,
				      NULL,
				      &stdout,
				      NULL,

				      &error)) {
		cb(NULL, -1, error, data);
		return;
	}

	closure = g_cclosure_new((GCallback) cb, data, NULL);

	ioch = g_io_channel_unix_new(stdout);
        g_io_channel_set_encoding(ioch, NULL, NULL);

	g_io_add_watch(ioch, G_IO_IN | G_IO_PRI | G_IO_ERR | G_IO_HUP | G_IO_NVAL, curl_readable_cb, closure);
}
