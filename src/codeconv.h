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
#ifndef __CODECONV_H__
#define __CODECONV_H__

#include <glib.h>

typedef struct _CodeConvDef {
	gchar *title;
	gchar *locale;
	gchar *codeset;
} CodeConvDef;

void codeconv_init(void);
gchar *codeconv_to_server(const gchar *input);
gchar *codeconv_to_local(const gchar *input);

enum {
	CODECONV_AUTO_DETECTION,
	CODECONV_NO_CONV,
	CODECONV_CUSTOM,
	CODECONV_LOCALE_START,
};

extern CodeConvDef conv_table[];

#endif /* __CODECONV_H__ */
