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
#include "utils.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "main.h"
#include <sys/types.h>
#include <sys/stat.h>

void debug_print(const gchar *format, ...)
{
	va_list args;
	gchar *str;

	if(!debug_mode)
		return;

	va_start(args, format);
	str = g_strdup_vprintf(format, args);
	va_end(args);
	
	g_free(str);	
}
void debug_puts(const gchar *format, ...)
{
	va_list args;
	gchar *str;

	if(!debug_mode)
		return;

	va_start(args, format);
	str = g_strdup_vprintf(format, args);
	va_end(args);
	
	g_print("%s\n", str);
	g_free(str);
}

gchar *utils_remove_return_code(gchar *str)
{
        register gchar *s;

        if (!*str) return str;

        for (s = str + strlen(str) - 1;
             s >= str && (*s == '\n' || *s == '\r');
             s--)
                *s = '\0';

        return str;
}

/* copied from Sylpheed. (c) 2002, Hiroyuki Yamamoto. */
gint make_dir(const gchar *dir)
{
        if (mkdir(dir, S_IRWXU) < 0) {
                FILE_OP_ERROR(dir, "mkdir");
                return -1;
        }
        if (chmod(dir, S_IRWXU) < 0)
                FILE_OP_ERROR(dir, "chmod");

        return 0;
}
