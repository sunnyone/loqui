/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui TitleFormat Utility <http://loqui.good-day.net/>
 * (foobar2000's title formatting parser)
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
#ifndef __LOQUI_TITLE_FORMAT_H__
#define __LOQUI_TITLE_FORMAT_H__

/* Caution: This is not GObject */

#include <glib.h>

#define LOQUI_TITLE_FORMAT_ERROR loqui_title_format_error_quark()

typedef enum {
	LOQUI_TITLE_FORMAT_ERROR_INVALID_INTERNAL_STRUCTURE,
	LOQUI_TITLE_FORMAT_ERROR_UNTERMINATED_FUNCTION_NAME,
	LOQUI_TITLE_FORMAT_ERROR_INVALID_FUNCTION_NAME,
	LOQUI_TITLE_FORMAT_ERROR_UNTERMINATED_FUNCTION_ARGUMENT,
	LOQUI_TITLE_FORMAT_ERROR_UNDEFINED_FUNCTION,
	LOQUI_TITLE_FORMAT_ERROR_UNTERMINATED_VARIABLE,
	LOQUI_TITLE_FORMAT_ERROR_INVALID_VARIABLE_NAME,
	LOQUI_TITLE_FORMAT_ERROR_UNTERMINATED_QUOTATION,
	LOQUI_TITLE_FORMAT_ERROR_UNTERMINATED_VARIABLE_AREA,
} LoquiTitleFormatError;

typedef struct _LoquiTitleFormat LoquiTitleFormat;

struct _LoquiTitleFormat {
	GHashTable *variable_table;
	GHashTable *function_table;

	GNode *root;
};

typedef gchar* (* LoquiTitleFormatFunction) (GList *arg_list);

GQuark loqui_title_format_error_quark(void);

LoquiTitleFormat *loqui_title_format_new(void);
gboolean loqui_title_format_parse(LoquiTitleFormat *ltf, const gchar *str, GError **error);
gchar *loqui_title_format_fetch(LoquiTitleFormat *ltf);
void loqui_title_format_register_function(LoquiTitleFormat *ltf, const gchar *name, LoquiTitleFormatFunction *func);
void loqui_title_format_register_variable(LoquiTitleFormat *ltf, const gchar *name, const gchar *value);
void loqui_title_format_free(LoquiTitleFormat *ltf);

#endif /* __LOQUI_TITLE_FORMAT_H__ */
