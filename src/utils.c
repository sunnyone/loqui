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

#include "intl.h"

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

GList *utils_line_separated_text_to_list(gchar *str)
{
	gchar **str_array;
	GList *list = NULL;
	gint i;

	str_array = g_strsplit(str, "\n", -1);

	for(i = 0; str_array[i] != NULL; i++) {
		if(strlen(str_array[i]) == 0)
			continue;

		list = g_list_append(list, g_strdup(str_array[i]));
	}

	g_strfreev(str_array);

	return list;
}

gchar *utils_line_separated_text_from_list(GList *list)
{
	GString *string;
	GList *cur;
	gchar *str;

	g_return_val_if_fail(list != NULL, NULL);

	string = g_string_new(NULL);

	for(cur = list; cur != NULL; cur = cur->next) {
		g_string_append_printf(string, "%s\n", (gchar *) cur->data);
	}

	str = string->str;
	g_string_free(string, FALSE);

	return str;
}

gchar *utils_format(const gchar *format, ...)
{
	va_list args;
	GString *string;
	GQueue *curly_queue;
	const gchar *cur, *tmp;
	gchar *str;
	gint i;
	const gchar *str_cache[CHAR_MAX];
	gint invisible = 0;

	g_return_val_if_fail(format != NULL, NULL);

	memset(str_cache, 0, CHAR_MAX);

	va_start(args, format);
	while((i = va_arg(args, gint)) != -1) {
		g_return_val_if_fail(i <= CHAR_MAX, NULL);

		str = va_arg(args, gchar *);
		
		str_cache[i] = str;
	}
	va_end(args);

	string = g_string_new_len(NULL, strlen(format));
	curly_queue = g_queue_new();

	cur = format;
	while(*cur != '\0' && ((tmp = strpbrk(cur, "%?}\\")) != NULL)) {
		if(tmp > cur) {
			if(!invisible)
				string = g_string_append_len(string, cur, tmp - cur);
			cur = tmp;
		}
		switch(*cur) {
		case '%':
			cur++;
			if(*cur == '%') {
				string = g_string_append_c(string, '%');
				break;
			}
			i = *cur;
			if(str_cache[i] != NULL)
				string = g_string_append(string, str_cache[i]);
			break;
		case '?':
			if(*cur++ == '\0')
				break;

			i = *cur;

			cur++;
			if(*cur != '{') {
				g_warning(_("?x flag doesn't have { in '%s'"), format);
				break;
			}

			if(str_cache[i] == NULL)
				invisible++;
			g_queue_push_tail(curly_queue, GINT_TO_POINTER(i));
			
			break;
		case '}':
			if(g_queue_is_empty(curly_queue))
				break;

			i = GPOINTER_TO_INT(g_queue_pop_tail(curly_queue));
			/* should free string/queue before this g_return_*, 
			   but it's needless because it should not be called. */
			g_return_val_if_fail(i < CHAR_MAX, NULL);

			if(str_cache[i] == NULL)
				invisible--;

			break;
		case '\\':
			if(*cur++ == '\0')
				break;

			string = g_string_append_c(string, *cur);
			break;
		default:
			g_assert_not_reached();
		}

		if(*cur == '\0')
			break;
		cur++;
	}

	str = string->str;
	g_string_free(string, FALSE);
	g_queue_free(curly_queue);

	return str;
}

gchar *utils_get_iso8601_date_string(time_t t)
{
/* YYYY-MM-DD hh:mm:ss */
#define DATE_LEN 4+1+2+1+2 +1+ 2+1+2+1+2 +5

	gchar buf[DATE_LEN];
	struct tm tm;

	localtime_r(&t, &tm);
	if(strftime(buf, DATE_LEN, "%Y-%m-%d %H:%M:%S", &tm) == 0)
		return NULL;

	return g_strdup(buf);

#undef DATE_LEN
}

static void
add_key_to_list_func(gpointer key, gpointer value, gpointer user_data)
{
	GList **list_ptr;

	list_ptr = (GList **) user_data;

	*list_ptr = g_list_append(*list_ptr, key);
}
static void
add_value_to_list_func(gpointer key, gpointer value, gpointer user_data)
{
	GList **list_ptr;

	list_ptr = (GList **) user_data;

	*list_ptr = g_list_append(*list_ptr, value);
}
GList *
utils_get_key_list_from_hash(GHashTable *hash_table)
{
	GList *list = NULL;

	g_return_val_if_fail(hash_table != NULL, NULL);

	g_hash_table_foreach(hash_table, add_key_to_list_func, &list);

	return list;
}
GList *
utils_get_value_list_from_hash(GHashTable *hash_table)
{
	GList *list = NULL;

	g_return_val_if_fail(hash_table != NULL, NULL);

	g_hash_table_foreach(hash_table, add_value_to_list_func, &list);

	return list;
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
