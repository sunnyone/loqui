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
#include <ctype.h>
#include "intl.h"

const gchar* uri_prefix_list[] = { "http://", "https://", "ftp://", "mailto:", "ttp://", NULL };

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

gchar *
utils_get_iso8601_date_string(time_t t)
{
	return utils_strftime_epoch("%Y-%m-%d %H:%M:%S", t);
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

/*
  Example:
                              v *start_uri
  utils_search_uri("hoge hoge http://hogefuga  aaaa", &got_uri, &start_uri, &end_uri);
                                            ^*end_uri
  got_uri = "http://hoge fuga" (malloc'ed)
*/
gboolean
utils_search_uri(const gchar *buf, gchar **got_uri,
		 const gchar **start_uri, const gchar **end_uri)
{
	int i;
	const gchar *tmp = NULL, *tmp_start_uri = NULL, *cur, *prefix = NULL, *used_prefix = NULL;
	const gchar *start_uri_ptr;

	cur = buf;
	for(i = 0; prefix = uri_prefix_list[i], prefix != NULL; i++) {
		tmp = strstr(cur, prefix);
		if(tmp != NULL && (tmp_start_uri == NULL || tmp_start_uri > tmp)) {
			used_prefix = prefix;
			tmp_start_uri = tmp;
		}
	}
	if(tmp_start_uri == NULL || used_prefix == NULL)
		return FALSE;

	cur = start_uri_ptr = tmp_start_uri;
	
	cur += strlen(used_prefix);
	while (*cur) {
		if(!isascii(*cur) ||
		   !g_ascii_isgraph(*cur) ||
		   strchr("()<>\"", *cur))
			break;
		cur++;
	}
	cur--;

	if(start_uri != NULL)
		*start_uri = start_uri_ptr;
	if(end_uri != NULL)
		*end_uri = cur;
	if(got_uri != NULL) {
		*got_uri = g_malloc0(cur - start_uri_ptr + 2);
		memcpy(*got_uri, start_uri_ptr, cur - start_uri_ptr + 1);
	}

	return TRUE;
}

gchar *
utils_strftime_epoch(const gchar *format, time_t t)
{
	struct tm tm;
	struct tm *tm_p;
	
	tm_p = &tm;
	/* cygwin's localtime_r ignores timezone! */
#if defined(HAVE_LOCALTIME_R) && !defined(__CYGWIN__)
	localtime_r(&t, tm_p);
#else
	tm_p = localtime(&t);
#endif
	return utils_strftime(format, tm_p);
}

#define TIME_DEFAULT_BUFSIZE 128
gchar *
utils_strftime(const gchar *format, struct tm *time)
{
	gchar *buf, *buf2;
	gint len, format_len;
	gint size;
	
	if(format == NULL)
		return NULL;
		
	format_len = strlen(format);
	if (format_len == 0)
		return NULL;

	size = TIME_DEFAULT_BUFSIZE;
	
	do {
		buf = g_malloc(size + 1);
		buf[0] = '.';
		len = strftime(buf, size, format, time);
		if (buf[0] == '\0')
			return buf;
		
		if (len > 0) {
			buf2 = g_locale_to_utf8(buf, -1, NULL, NULL, NULL);
			g_free(buf);
			return buf2;
		}
		
		g_free(buf);
		size *= 2;
	/* tricky way ... it assumes len is not 1024 times bigger than format_len */	
	} while (size < 1024 * format_len);
	
	return NULL;
}
void
utils_g_list_foreach_swapped(GList *list, GFunc func, gpointer user_data)
{
	GList *cur;

	for (cur = list; cur != NULL; cur = cur->next)
		(*func)(user_data, cur->data);
}

gboolean
utils_strcase_equal(gconstpointer a, gconstpointer b)
{
	return (g_ascii_strcasecmp(a, b) == 0);
}
guint
utils_strcase_hash(gconstpointer v)
{
       return g_str_hash(g_ascii_strdown(v, -1));
}
void
utils_g_ptr_array_insert_sort(GPtrArray *array, gint sort_start_pos, GCompareFunc sort_func)
{
	gint i, j;
	gpointer *pdata = array->pdata;
	gpointer tmp;

	for (i = sort_start_pos; i < array->len; i++) {
		for (j = 0; j < i; j++) {
			if (sort_func(&pdata[j], &pdata[i]) > 0)
				break;
		}
		if (i != j) {
			tmp = pdata[i];
			g_memmove(pdata + j + 1, pdata + j, (i - j) * sizeof(gpointer));
			pdata[j] = tmp;
		}
	}
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
