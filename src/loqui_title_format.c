/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui TitleFormat Utility <http://loqui.good-day.net/>
 * Copyright (C) 2004 Yoichi Imai <yoichi@silver-forest.com>
 *
 * You can use TitleFormat on these conditions.
 * (in GPL/LGPL program, TitleFormat follows GPL/LGPL.)
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *  notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *  notice, this list of conditions and the following disclaimer in the
 *  documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *  may be used to endorse or promote products derived from this software
 *  without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/* TODO:
 *  - test function
 *  - multiline
 *  - comment
 *  - standard functions
 */

#include "config.h"
#include "loqui_title_format.h"
#include <string.h>

static gchar * loqui_title_format_parse_internal(LoquiTitleFormat *ltf, const gchar *start, const gchar *end_chars, gchar **end_pos_ptr, gboolean is_braced, GError **error);
static gchar * loqui_title_format_parse_function(LoquiTitleFormat *ltf, const gchar *start, gchar **end_pos_ptr, GError **error);
static gboolean loqui_title_format_validate_symbol_name(const gchar *name);

static gboolean
loqui_title_format_validate_symbol_name(const gchar *name)
{
	const gchar* s;
	
	s = name;
	while(*s != '\0') {
		if(!strchr("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_", *s))
			return FALSE;
		s++;
	}
	return TRUE;
}

static gchar *
loqui_title_format_parse_function(LoquiTitleFormat *ltf, const gchar *start, gchar **end_pos_ptr, GError **error)
{
	const gchar *cur, *tmp;
	gchar *name = NULL;
	gchar *buf = NULL;
	gchar *child_end;
	GList *arg_list = NULL;
	LoquiTitleFormatFunction func;

	g_return_val_if_fail(ltf != NULL, NULL);
	g_return_val_if_fail(*start != '$', NULL);
	
	cur = start; /* "$name(arg1, arg2)" */
	cur++;       /* "name(arg1, arg2)" */

	tmp = cur;
	if ((cur = strchr(tmp, '(')) == NULL) {
		g_set_error(error,
			    LOQUI_TITLE_FORMAT_ERROR,
			    LOQUI_TITLE_FORMAT_ERROR_UNTERMINATED_FUNCTION_NAME,
			    "Unterminated function name: %s", tmp);
		return NULL;
	}
	name = g_strndup(tmp, cur - tmp);
	if (!loqui_title_format_validate_symbol_name(name)) {
		g_set_error(error,
			    LOQUI_TITLE_FORMAT_ERROR,
			    LOQUI_TITLE_FORMAT_ERROR_INVALID_FUNCTION_NAME,
			    "Invalid function name: %s", name);
		goto error;
	}
	/* cur = "(arg1, arg2)" */
	while (*cur != ')') {
		cur++; /* skip ',' (after 2nd loop) or '(' (1st loop) */
		if(!(buf = loqui_title_format_parse_internal(ltf, cur, ",)", &child_end, TRUE, error))) {
			goto error;
		}
		
		if (*child_end == '\0') {
			g_set_error(error,
				    LOQUI_TITLE_FORMAT_ERROR,
				    LOQUI_TITLE_FORMAT_ERROR_UNTERMINATED_FUNCTION_ARGUMENT,
				    "Unterminated function argument: %s", cur);
			goto error;
		}
		cur = child_end;
		arg_list = g_list_append(arg_list, buf);
	}
	buf = NULL;
	if (end_pos_ptr != NULL)
		*end_pos_ptr = (gchar *) cur;
	
	func = g_hash_table_lookup(ltf->function_table, name);
	if (!func) {
			g_set_error(error,
				    LOQUI_TITLE_FORMAT_ERROR,
				    LOQUI_TITLE_FORMAT_ERROR_UNDEFINED_FUNCTION,
				    "Undefined function: %s", name);
			goto error;
	}
	buf = func(arg_list);
	return buf;

 error:
	if (name)
		g_free(name);
	if (buf)
		g_free(buf);
	g_list_foreach(arg_list, (GFunc) g_free, NULL);
	g_list_free(arg_list);
	return NULL;
}
static gchar *
loqui_title_format_parse_internal(LoquiTitleFormat *ltf, const gchar *start, const gchar *end_chars, gchar **end_pos_ptr, gboolean is_braced, GError **error)
{
	GString *buffer;
	const gchar *cur, *s;
	gchar *child_end;
	gint matched_count = 0;
	gchar *buf, *name;

	g_return_val_if_fail(ltf != NULL, NULL);
	g_return_val_if_fail(start != NULL, NULL);
	g_return_val_if_fail(end_chars != NULL, NULL);

	buffer = g_string_new("");

	cur = start;
	while (*cur != '\0') {
		if (strchr(end_chars, *cur))
			break;

		switch (*cur) {
		case '$':
			if(!(buf = loqui_title_format_parse_function(ltf, cur, &child_end, error))) {
				g_string_free(buffer, TRUE);
				return NULL;
			}
			g_string_append(buffer, buf);
			cur = child_end;
			break;
		case '[':
			cur++;
			if(!(buf = loqui_title_format_parse_internal(ltf, cur, "]", &child_end, TRUE, error))) {
				g_string_free(buffer, TRUE);
				return NULL;
			}
			g_string_append(buffer, buf);
			cur = child_end;
			break;
		case '%':
			cur++;
			s = cur;
			if ((cur = strchr(s, '%')) == NULL) {
				g_set_error(error,
					    LOQUI_TITLE_FORMAT_ERROR,
					    LOQUI_TITLE_FORMAT_ERROR_UNTERMINATED_VARIABLE,
					    "Unterminated variable: %%%s", s);
				g_string_free(buffer, TRUE);
				return NULL;
			}
			name = g_strndup(s, cur - s);
			if (!loqui_title_format_validate_symbol_name(name)) {
				g_set_error(error,
					    LOQUI_TITLE_FORMAT_ERROR,
					    LOQUI_TITLE_FORMAT_ERROR_INVALID_VARIABLE_NAME,
					    "Invalid variable name: %s", name);
				g_string_free(buffer, TRUE);
				g_free(name);
				return NULL;
			}
			buf = g_hash_table_lookup(ltf->variable_table, name);
			g_free(name);
			if (buf) {
				g_string_append(buffer, buf);
				matched_count++;
			}
			break;
		case '\'':
			s = cur;
			cur++;
			if ((cur = strchr(cur, '\'')) == NULL) {
				g_set_error(error,
					    LOQUI_TITLE_FORMAT_ERROR,
					    LOQUI_TITLE_FORMAT_ERROR_UNTERMINATED_QUOTATION,
					    "Unterminated quotation %s", s);
				g_string_free(buffer, TRUE);
				return NULL;
			}
			break;
		default:
			g_string_append_c(buffer, *cur);
		}
		cur++;
	}

	if (end_pos_ptr)
		*end_pos_ptr = (gchar *) cur;

	if (is_braced && matched_count == 0) {
		g_string_free(buffer, TRUE);
		return g_strdup("");
	}
	return g_string_free(buffer, FALSE);
}
GQuark
loqui_title_format_error_quark(void)
{
	static GQuark quark = 0;
	if (!quark)
		quark = g_quark_from_static_string("loqui-title-format-error-quark");

	return quark;
}

LoquiTitleFormat *
loqui_title_format_new(void)
{
	LoquiTitleFormat *ltf;

	ltf = g_new0(LoquiTitleFormat, 1);
	ltf->function_table = g_hash_table_new_full(g_str_hash, g_str_equal, (GDestroyNotify) g_free, NULL);
	ltf->variable_table = g_hash_table_new_full(g_str_hash, g_str_equal, (GDestroyNotify) g_free, (GDestroyNotify) g_free);
	return ltf;
}
void
loqui_title_format_register_function(LoquiTitleFormat *ltf, const gchar *name, LoquiTitleFormatFunction *func)
{
	g_return_if_fail(ltf != NULL);

	if (func)
		g_hash_table_insert(ltf->function_table, g_strdup(name), func);
	else
		g_hash_table_remove(ltf->function_table, name);
}
void
loqui_title_format_register_variable(LoquiTitleFormat *ltf, const gchar *name, const gchar *value)
{
	g_return_if_fail(ltf != NULL);

	if (value)
		g_hash_table_insert(ltf->variable_table, g_strdup(name), g_strdup(value));
	else
		g_hash_table_remove(ltf->variable_table, name);
}
gchar *
loqui_title_format_parse(LoquiTitleFormat *ltf, const gchar *str, GError **error)
{
	return loqui_title_format_parse_internal(ltf, str, "", NULL, FALSE, error);
}
void
loqui_title_format_free(LoquiTitleFormat *ltf)
{
	g_hash_table_destroy(ltf->variable_table);
	g_hash_table_destroy(ltf->function_table);
	g_free(ltf);
}
