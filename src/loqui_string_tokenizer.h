/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui StringTokenizer Utility <http://loqui.good-day.net/>
 * Copyright (C) 2004 Yoichi Imai <yoichi@silver-forest.com>
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
#ifndef __LOQUI_STRING_TOKENIZER_H__
#define __LOQUI_STRING_TOKENIZER_H__

/* Caution: This is not GObject */

#include <glib.h>

typedef struct _LoquiStringTokenizer LoquiStringTokenizer;

struct _LoquiStringTokenizer {
	gchar *cur;
	gchar *cur_peeked;

	gchar *orig_ptr;

	gchar *next_token_peek;
	gchar delimiter_peek;

	gchar *next_token;
	gchar delimiter;
	
	gchar *delimiters;
	gboolean skip_whitespaces_after_delimiter;
};

#define LOQUI_STRING_TOKENIZER_WHITESPACE " \t\r\n"

LoquiStringTokenizer *loqui_string_tokenizer_new(const gchar *str, const gchar *delimiters);
void loqui_string_tokenizer_free(LoquiStringTokenizer *st);

void loqui_string_tokenizer_set_delimiters(LoquiStringTokenizer *st, const gchar *delimiters);
void
loqui_string_tokenizer_set_skip_whitespaces_after_delimiter(LoquiStringTokenizer *st, gboolean skip_whitespaces);
G_CONST_RETURN gchar *loqui_string_tokenizer_next_token(LoquiStringTokenizer *st, gchar *delimiter_ptr);
G_CONST_RETURN gchar *loqui_string_tokenizer_peek_next_token(LoquiStringTokenizer *st, gchar *delimiter_ptr);
G_CONST_RETURN gchar *loqui_string_tokenizer_next_token_full(LoquiStringTokenizer *st, gchar *delimiter_ptr, const gchar *delimiters);
G_CONST_RETURN gchar *loqui_string_tokenizer_peek_next_token_full(LoquiStringTokenizer *st, gchar *delimiter_ptr, const gchar *delimiters);
G_CONST_RETURN gchar *loqui_string_tokenizer_get_original_string(LoquiStringTokenizer *st);
G_CONST_RETURN gchar *loqui_string_tokenizer_get_remains(LoquiStringTokenizer *st);

void loqui_string_tokenizer_skip_char(LoquiStringTokenizer *st);
gchar loqui_string_tokenizer_peek_cur_char(LoquiStringTokenizer *st);
gboolean loqui_string_tokenizer_has_more_tokens(LoquiStringTokenizer *st);
gint loqui_string_tokenizer_count_tokens(LoquiStringTokenizer *st);
gint loqui_string_tokenizer_count_tokens_full(LoquiStringTokenizer *st, gchar *delimiters);

#endif /* __LOQUI_STRING_TOKENIZER_H__ */
