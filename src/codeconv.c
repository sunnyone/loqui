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

#include "codeconv.h"
#include "prefs_general.h"
#include "intl.h"
#include <string.h>
#include <locale.h>
#include <errno.h>

static gchar *server_codeset = NULL;
#define GTK_CODESET "UTF-8"
#define BUFFER_LEN 2048

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
	
	num = prefs_general.codeconv;
	if(num > length) {
		g_warning(_("the setting of codeconv was invalid; now 'Auto Detection' is selected."));
		num = 0;
	}
	
	if(num == -1) {
		server_codeset = prefs_general.codeset;
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
	gchar *tmp;
	gchar buf[BUFFER_LEN+1];
	gsize original_len;
	GString *string;

	GIConv cd;
	gchar *inbuf;
	gsize in_left;
	gchar *outbuf;
        gsize out_left;
	size_t ret;

	if(input == NULL)
		return NULL;

	if(server_codeset == NULL || strlen(server_codeset) == 0)
		return g_strdup(input);

	/* we use a compilicated way to handle broken characters */
	cd = g_iconv_open(GTK_CODESET, server_codeset);
	if(cd == NULL)
		return NULL;
	
	string = g_string_new(NULL);
	original_len = strlen(input);

	in_left = original_len;
	inbuf = (gchar *) input;

	do {
		outbuf = buf;
		out_left = BUFFER_LEN;

		ret = g_iconv(cd, &inbuf, &in_left, &outbuf, &out_left);

		if(outbuf - buf > 0)
			string = g_string_append_len(string, buf, outbuf - buf);

		if(ret == -1) {
			switch(errno) {
			case EILSEQ:
				inbuf++;
				in_left--;
				string = g_string_append(string, "[?]");
				break;
			case EINVAL:
				string = g_string_append(string, "[?]");
				ret = 0; /* exit the loop */
				break;
			case E2BIG:
				break;
			default:
				g_warning(_("Unknown error occurs in code convertion."));
				return NULL;
			}
		}
	} while(ret == -1);

	/* to append terminating characters */
	outbuf = buf;
	out_left = BUFFER_LEN;
	g_iconv(cd, NULL, NULL, &outbuf, &out_left);
	if(outbuf - buf > 0)
		string = g_string_append_len(string, outbuf, outbuf - buf);

	string = g_string_append_c(string, '\0');
	g_iconv_close(cd);

	tmp = string->str;
	g_string_free(string, FALSE);
	return tmp;
}
