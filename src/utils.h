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
#ifndef __UTILS_H__
#define __UTILS_H__
#include <glib.h>

#define STR_FREE_UNLESS_NULL(str) { \
 if(str != NULL) { \
    g_free(str); \
    str = NULL; \
 } \
}

void debug_puts(const gchar *format, ...) G_GNUC_PRINTF(1, 2);
void debug_print(const gchar *format, ...) G_GNUC_PRINTF(1, 2);

gchar *utils_gconf_get_basename(const gchar *path);

gchar *utils_remove_return_code(gchar *str);

/* copied from Sylpheed. (c) 2002, Hiroyuki Yamamoto. */
#define Xstrndup_a(ptr, str, len, iffail) \
{ \
        gchar *__tmp; \
 \
        if ((__tmp = alloca(len + 1)) == NULL) { \
                g_warning("can't allocate memory\n"); \
                iffail; \
        } else { \
                strncpy(__tmp, str, len); \
                __tmp[len] = '\0'; \
        } \
 \
        ptr = __tmp; \
}
gchar *strncpy2         (gchar          *dest,
                         const gchar    *src,
                         size_t          n);
gchar *strcasestr       (const gchar    *haystack,
                         const gchar    *needle);
gint make_dir(const gchar *dir);

#define FILE_OP_ERROR(file, func) \
{ \
        fprintf(stderr, "%s: ", file); \
        perror(func); \
}

#endif /* __UTILS_H__ */
