/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui -- IRC client for GNOME2 <http://loqui.good-day.net/>
 * Copyright (C) 2002 Yoichi Imai <yoichi@silver-forest.com>
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

#include <string.h>
#include <stdarg.h>
#include "main.h"


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

gchar *
utils_gconf_get_basename(const gchar *path)
{
	gchar *s;

	g_return_val_if_fail(path != NULL, NULL);

	s = strrchr(path, '/');
	if(s == NULL)
		return NULL;
	s++;

	return g_strdup(s);
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
gchar *strncpy2(gchar *dest, const gchar *src, size_t n)
{
        register gchar c;
        gchar *s = dest;

        do {
                if (--n == 0) {
                        *dest = '\0';
                        return s;
                }
                c = *src++;
                *dest++ = c;
        } while (c != '\0');

        /* don't do zero fill */
        return s;
}

/* Similar to `strstr' but this function ignores the case of both strings.  */
gchar *strcasestr(const gchar *haystack, const gchar *needle)
{
        register size_t haystack_len, needle_len;

        haystack_len = strlen(haystack);
        needle_len   = strlen(needle);

        if (haystack_len < needle_len || needle_len == 0)
                return NULL;

        while (haystack_len >= needle_len) {
                if (!strncasecmp(haystack, needle, needle_len))
                        return (gchar *)haystack;
                else {
                        haystack++;
                        haystack_len--;
                }
        }

        return NULL;
}
