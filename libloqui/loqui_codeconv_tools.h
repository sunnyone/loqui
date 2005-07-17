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
#ifndef __LOQUI_CODECONV_TOOLS_H__
#define __LOQUI_CODECONV_TOOLS_H__

#include <loqui_codeconv.h>

gchar *loqui_codeconv_tools_jis_to_utf8(LoquiCodeConv *codeconv, gboolean is_to_server, const gchar *input, GError **error);

gchar *loqui_codeconv_tools_utf8_from_ms_table(const gchar *str);
gchar *loqui_codeconv_tools_utf8_to_ms_table(const gchar *str);

#endif /* __LOQUI_CODECONV_TOOLS_H__ */
