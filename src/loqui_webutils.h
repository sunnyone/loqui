/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui -- Chat client for Gtk2 <http://loqui.good-day.net/>
 * Copyright (C) 2004 Yoichi Imai <yoichi@silver-forest.com>
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
#ifndef __LOQUI_WEBUTILS_H__
#define __LOQUI_WEBUTILS_H__

#include <glib-object.h>

typedef void (* LoquiWebCallback) (gchar *text, gsize len, GError *error, gpointer data);

typedef enum {
	WEBUTILS_GET_HEADER,
	WEBUTILS_GET_BODY,
	WEBUTILS_GET_HEADER_AND_BODY
} LoquiWebUtilsGetType;

void loqui_webutils_get(gchar *url, GList *header_list, LoquiWebUtilsGetType get_type, LoquiWebCallback cb, gpointer data);

#endif /* __LOQUI_WEBUTILS_H__ */
