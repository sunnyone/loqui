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
#ifndef __UTILS_H__
#define __UTILS_H__
#include <glib.h>

#define G_FREE_UNLESS_NULL(str) { \
 if(str != NULL) { \
    g_free(str); \
    str = NULL; \
 } \
}

#define DEBUG_TIMER_START(timer) { \
   timer = g_timer_new(); \
   g_timer_start(timer); \
}
#define DEBUG_TIMER_STOP(timer, name) { \
   g_timer_stop(timer); \
   g_print("elapsed(%s): %.7f s\n", name, g_timer_elapsed(timer, NULL)); \
   g_timer_destroy(timer); \
}

void debug_puts(const gchar *format, ...) G_GNUC_PRINTF(1, 2);
void debug_print(const gchar *format, ...) G_GNUC_PRINTF(1, 2);

gchar *utils_remove_return_code(gchar *str);

GSList *utils_line_separated_text_to_slist(gchar *str);
gchar *utils_line_separated_text_from_slist(GSList *slist);

/* copied from Sylpheed. (c) 2002, Hiroyuki Yamamoto. */
gint make_dir(const gchar *dir);

#define FILE_OP_ERROR(file, func) \
{ \
        fprintf(stderr, "%s: ", file); \
        perror(func); \
}

#endif /* __UTILS_H__ */
