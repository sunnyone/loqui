/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui -- IRC client for GNOME2
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

#include "codeconv.h"
#include "loqui_gconf.h"

static gchar *server_codeset = NULL;
#define GTK_CODESET "UTF-8"

/* tell me other languages if you know */
static gchar *conv_table[][3] = {
	{"Auto Detection", NULL,       NULL}, /* for the setting */
	{"No conv",        NULL,       ""},   /* for the setting */
	{"Japanese",       "ja_JP",    "ISO-2022-JP"},
	{NULL, NULL, NULL},
};

void
codeconv_init(void)
{
	int length, i, num;
	gchar *ctype;

	length = 0;
	while(conv_table[length][0] != NULL) length++;
	
	num = eel_gconf_get_integer(LOQUI_GCONF_BASEDIR "codeconv");
	if(num > length) {
		g_warning(_("the setting of codeconv was invalid; now 'Auto Detection' is selected."));
		num = 0;
	}
	
	if(num == -1) {
		server_codeset = eel_gconf_get_string(LOQUI_GCONF_BASEDIR "codeset");
	} else if(num == 0) {
		i = 2;
		ctype = setlocale(LC_CTYPE, NULL);
		if(ctype) {
			while(conv_table[i] != NULL) {
				if(conv_table[i][1] == NULL) continue;
				if(strstr(ctype, conv_table[i][1]) != NULL) {
					server_codeset = conv_table[i][2];
					break;
				}
				i++;
			}
		}
	} else {
		server_codeset = conv_table[num][2];
	}

	if(server_codeset == NULL)
		server_codeset = "";
}

gchar *
codeconv_to_server(const gchar *input)
{
	gchar *output;
	GError *error = NULL;

	if(input == NULL) {
		return NULL;
	}

	if(strlen(server_codeset) == 0)
		return g_strdup(input);

	output = g_convert(input, strlen(input)+1, server_codeset, GTK_CODESET,
			   NULL, NULL, &error);
	if(error != NULL) {
		g_warning(_("Code convartion error: %s"), error->message);
		g_error_free(error);
	}

	return output;
}

gchar *
codeconv_to_local(const gchar *input)
{
	const gchar *cur;
	gchar *tmp;
	gsize original_len;
	gsize len_to_read;
	GError *error = NULL;
	GString *string;
	gboolean add_unknown_mark;
	GIConv cd;

	if(input == NULL) {
		return NULL;
	}

	if(strlen(server_codeset) == 0)
		return g_strdup(input);

	/* we use a compilicated way to handle broken characters */
	cd = g_iconv_open(GTK_CODESET, server_codeset);
	if(cd == NULL)
		return NULL;
	
	string = g_string_new(NULL);
	original_len = strlen(input);
	cur = input;
	while(cur < input + original_len) {
		tmp = g_convert_with_iconv(cur, strlen(cur)+1, cd,
				&len_to_read, NULL, &error);
		if(error == NULL) {
			cur += len_to_read;
			add_unknown_mark = FALSE;
		} else {
			if(error->code != G_CONVERT_ERROR_ILLEGAL_SEQUENCE) {
				g_warning("Code convartion error: %s,%s", error->message, input);
				g_error_free(error);
				g_string_free(string, TRUE);
				return NULL;
			}
			g_error_free(error);
			error = NULL;

			if(len_to_read == 0) {
				tmp = NULL;
			} else {
				tmp = g_convert_with_iconv(cur, len_to_read, cd,
							   NULL, NULL, &error);
				
				if(error != NULL) {
					g_warning("Code convartion error: %s,%s", error->message, input);
					g_error_free(error);
					g_string_free(string, TRUE);
					return NULL;
				}
			}
			add_unknown_mark = TRUE;
			cur += len_to_read+1;
		}
		if(tmp != NULL)
			string = g_string_append(string, tmp);
		if(add_unknown_mark) {
			string = g_string_append(string, "[?]");
		}
	}

	g_iconv_close(cd);

	tmp = string->str;
	g_string_free(string, FALSE);
	return tmp;
}
