/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui StringTokenizer Utility <http://launchpad.net/loqui/>
 * Copyright (C) 2004 Yoichi Imai <sunnyone41@gmail.com>
 *
 * You can use LoquiStringTokenizer on these conditions.
 * (in GPL/LGPL program, LoquiStringTokenizer follows GPL/LGPL.)
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
/* #include "config.h" */
#include "loqui_string_tokenizer.h"

#include <string.h>

/**
   @str String
   @delimiters " \r\n" means [' ', '\r', '\n']
*/
LoquiStringTokenizer *
loqui_string_tokenizer_new(const gchar *str, const gchar *delimiters)
{
	LoquiStringTokenizer *st;

	st = g_new0(LoquiStringTokenizer, 1);

	st->orig_ptr = g_strdup(str);
	st->delimiters = g_strdup(delimiters);
	st->cur = st->orig_ptr;
	
	st->delimiter = '\0';
	st->delimiter_peek = '\0';
	st->next_token_peek = NULL;
	st->next_token = NULL;

	st->skip_whitespaces_after_delimiter = FALSE;

	return st;
}
void
loqui_string_tokenizer_free(LoquiStringTokenizer *st)
{
	if (st->orig_ptr)
		g_free(st->orig_ptr);
	if (st->next_token_peek)
		g_free(st->next_token_peek);
	if (st->next_token)
		g_free(st->next_token);
	if (st->delimiters)
		g_free(st->delimiters);

	g_free(st);
}
void
loqui_string_tokenizer_set_delimiters(LoquiStringTokenizer *st, const gchar *delimiters)
{
	g_return_if_fail(delimiters != NULL);

	if (st->delimiters)
		g_free(st->delimiters);
	st->delimiters = g_strdup(delimiters);

	g_free(st->next_token_peek);
	st->next_token_peek = NULL;
	
	g_free(st->next_token);
	st->next_token = NULL;

	st->delimiter_peek = '\0';
	st->delimiter = '\0';
}
void
loqui_string_tokenizer_set_skip_whitespaces_after_delimiter(LoquiStringTokenizer *st, gboolean skip_whitespaces)
{
	st->skip_whitespaces_after_delimiter = skip_whitespaces;
}
G_CONST_RETURN gchar *
loqui_string_tokenizer_next_token(LoquiStringTokenizer *st, gchar *delimiter_ptr)
{
	g_return_val_if_fail(st != NULL, NULL);

	if (loqui_string_tokenizer_peek_next_token(st, NULL) == NULL)
		return NULL;

	if (st->next_token)
		g_free(st->next_token);
	st->next_token = st->next_token_peek;
	st->next_token_peek = NULL;
	st->delimiter = st->delimiter_peek;
	st->delimiter_peek = '\0';

	st->cur = st->cur_peeked;

	if (st->skip_whitespaces_after_delimiter) {
		if (*st->cur != '\0' && strchr(LOQUI_STRING_TOKENIZER_WHITESPACE, *st->cur))
			st->cur++;
	}

	if (delimiter_ptr)
		*delimiter_ptr = st->delimiter;

	return st->next_token;
}
G_CONST_RETURN gchar *
loqui_string_tokenizer_peek_next_token(LoquiStringTokenizer *st, gchar *delimiter_ptr)
{
	gchar *tmp;

	g_return_val_if_fail(st != NULL, NULL);

	if (st->next_token_peek) {
		if (delimiter_ptr)
			*delimiter_ptr = st->delimiter_peek;
		return st->next_token_peek;
	}

	if (*st->cur == '\0') {
		if (delimiter_ptr)
			*delimiter_ptr = '\0';
		return NULL;
	}

	if ((tmp = strpbrk(st->cur, st->delimiters)) != NULL) {
		st->next_token_peek = g_strndup(st->cur, tmp - st->cur);
		st->delimiter_peek = *tmp;
		st->cur_peeked = tmp + 1;
	} else {
		st->next_token_peek = g_strdup(st->cur);
		st->delimiter_peek = '\0';
		st->cur_peeked = st->cur + strlen(st->cur);
	}

	if (delimiter_ptr)
		*delimiter_ptr = st->delimiter_peek;

	return st->next_token_peek;
}
void
loqui_string_tokenizer_skip_char(LoquiStringTokenizer *st)
{
	if (*st->cur == '\0')
		return;

	if (st->next_token_peek)
		g_free(st->next_token_peek);
	if (st->next_token)
		g_free(st->next_token);
	st->next_token_peek = NULL;
	st->next_token = NULL;
	st->delimiter_peek = '\0';
	st->delimiter = '\0';

	st->cur++;
}
gchar
loqui_string_tokenizer_peek_cur_char(LoquiStringTokenizer *st)
{
	return *st->cur;
}
G_CONST_RETURN gchar *
loqui_string_tokenizer_get_original_string(LoquiStringTokenizer *st)
{
	return st->orig_ptr;
}
G_CONST_RETURN gchar *
loqui_string_tokenizer_get_remains(LoquiStringTokenizer *st)
{
	return st->cur;
}
gboolean
loqui_string_tokenizer_has_more_tokens(LoquiStringTokenizer *st)
{
	if (*st->cur == '\0')
		return FALSE;
	return TRUE;
}
gint
loqui_string_tokenizer_count_tokens(LoquiStringTokenizer *st)
{
	gint count;
	const gchar *tmp;

	if (*st->cur == '\0')
		return 0;

	tmp = st->cur;
	count = 1;
	while (tmp != '\0' && (tmp = strpbrk(tmp, st->delimiters)) != NULL) {
		count++;
		tmp++;
	} 

	return count;
}
