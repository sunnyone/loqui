/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * libloqui -- Chat/IM client library for GLib <http://loqui.good-day.net/>
 * Copyright (C) 2004 Yoichi Imai <yoichi@silver-forest.com>
 *
 * This Library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with the Gnome Library; see the file COPYING.LIB.  If not,
 * write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#include "config.h"
#include "loqui_codeconv_tools.h"
#include "loqui_codeconv.h"

#include <ctype.h>
#include <string.h>

/* if you want to use EUC-JP-MS table, set it */
#define ICONV_EUC_JP_CODESET "EUC-JP"

gunichar jis_13ku_table[] = { 0x0000, 0x2460, 0x2461, 0x2462, 0x2463, 0x2464, 0x2465, 0x2466,
			      0x2467, 0x2468, 0x2469, 0x246a, 0x246b, 0x246c, 0x246d, 0x246e,
			      0x246f, 0x2470, 0x2471, 0x2472, 0x2473, 0x2160, 0x2161, 0x2162,
			      0x2163, 0x2164, 0x2165, 0x2166, 0x2167, 0x2168, 0x2169, 0x0000,
			      0x3349, 0x3314, 0x3322, 0x334d, 0x3318, 0x3327, 0x3303, 0x3336,
			      0x3351, 0x3357, 0x330d, 0x3326, 0x3323, 0x332b, 0x334a, 0x333b,
			      0x339c, 0x339d, 0x339e, 0x338e, 0x338f, 0x33c4, 0x33a1, 0x0000,
			      0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x337b,
			      0x301d, 0x301f, 0x2116, 0x33cd, 0x2121, 0x32a4, 0x32a5, 0x32a6,
			      0x32a7, 0x32a8, 0x3231, 0x3232, 0x3239, 0x337e, 0x337d, 0x337c,
			      0x2252, 0x2261, 0x222b, 0x222e, 0x2211, 0x221a, 0x22a5, 0x2220,
			      0x221f, 0x22bf, 0x2235, 0x2229, 0x222a, 0x0000, 0x0000, 0x0000};

typedef enum {
        ISO_2022_JP_TYPE_INVALID,
        ISO_2022_JP_TYPE_ASCII,          /* ESC ( B */
	ISO_2022_JP_TYPE_JISX_0201_1976, /* Roman, ESC ( J */
	ISO_2022_JP_TYPE_JISX_0208_1978, /* Kanji, ESC $ @ */
	ISO_2022_JP_TYPE_JISX_0208_1983, /* Kanji, ESC $ B */
	ISO_2022_JP_TYPE_JISX_0201_KANA, /* informal, halfwidth-kana, ESC ( I */
} ISO2022JPAreaType;

#define ISO_2022_JP_SHIFT_OUT 0x0E
#define ISO_2022_JP_SHIFT_IN  0x0F
#define ISO_2022_ESC          0x1B
#define HALFWIDTH_KATAKANA_UCS4_OFFSET 0xFF60
#define HALFWIDTH_KATAKANA_7BIT_OFFSET 0x20
#define HALFWIDTH_KATAKANA_8BIT_OFFSET 0xA0
#define IS_ISO_2022_JP_8BIT_HALFWIDTH_KANA(c) ((0xA0 < c && c <= 0xDF))
#define IS_ISO_2022_JP_7BIT_HALFWIDTH_KANA(c) ((0x20 < c && c <= 0x5F))

#define UTF8_CHAR_MAX_LEN 6
#define EUC_JP_CHAR_MAX_LEN 3

gchar *
loqui_codeconv_tools_jis_to_utf8(LoquiCodeConv *codeconv, gboolean is_to_server, const gchar *input, GError **error)
{
	const gchar *cur;
	gboolean shift_out_mode = FALSE;
	GString *string;
	guchar c, c1, c2;
	gchar euc_jp_kanji[EUC_JP_CHAR_MAX_LEN+1];
	gchar utf8char[UTF8_CHAR_MAX_LEN+1];
	gchar *src, *dst;
	gunichar u;
	gsize inbytes_left, outbytes_left;
	ISO2022JPAreaType area_type;
	GIConv cd;

	if (is_to_server) {
		gchar *buf;

		buf = g_convert_with_iconv(input, strlen(input)+1, codeconv->cd_to_server,
					   NULL, NULL, error);
		return buf;
	}

	
	cd = g_iconv_open("UTF-8", ICONV_EUC_JP_CODESET);
	if (cd < 0) {
		g_set_error(error,
			    LOQUI_CODECONV_ERROR,
			    LOQUI_CODECONV_ERROR_FAILED_OPEN_ICONV,
			    "Can't open iconv (%s)", ICONV_EUC_JP_CODESET);
		return NULL;
	}

	string = g_string_new_len(NULL, strlen(input));
	cur = input;
	area_type = ISO_2022_JP_TYPE_ASCII;
	while ((c = *cur) != '\0') {
		if (c == '\r' || c == '\n') {
			g_string_append_c(string, c);
			if (c == '\n')
				area_type = ISO_2022_JP_TYPE_ASCII;
			cur++;
			continue;
		}

		if (c == ISO_2022_ESC) {
			cur++;
			if (*cur == '\0')
				break;
			c1 = *cur;
			cur++;
			if (*cur == '\0')
				break;
			c2 = *cur;
			cur++;
	
			if (c1 == '(') {
				switch (c2) {
				case 'J':
					area_type = ISO_2022_JP_TYPE_JISX_0201_1976;
					break;
				case 'I':
					area_type = ISO_2022_JP_TYPE_JISX_0201_KANA;
					break;
				case 'B':
				default:
					area_type = ISO_2022_JP_TYPE_ASCII;
				}
			} else if (c1 == '$') {
				switch (c2) {
				case '@':
					area_type = ISO_2022_JP_TYPE_JISX_0208_1978;
					break;
				case 'B':
					area_type = ISO_2022_JP_TYPE_JISX_0208_1983;
					break;
				default:
					area_type = ISO_2022_JP_TYPE_ASCII;
				}
			}
			continue;
		}

		switch (area_type) {
		case ISO_2022_JP_TYPE_JISX_0208_1983:
		case ISO_2022_JP_TYPE_JISX_0208_1978:
			if (*(cur + 1) == ISO_2022_ESC)
				break;

			cur++;
			c1 = *cur;
			if (c1 == '\0')
				break;
			c |= 0x80;
			c1 |= 0x80;
				
			euc_jp_kanji[0] = c;
			euc_jp_kanji[1] = c1;
			euc_jp_kanji[2] = 0;

			if (c == 0xad && 0xa1 <= c1 && c1 <= 0xfc) { /* 13ku NEC special charaters */
				u = jis_13ku_table[c1 - 0xa0];
				if (u == 0)
					g_string_append(string, "[?]");
				else
					g_string_append_unichar(string, u);
				break;
			}

			inbytes_left = 3;
			outbytes_left = UTF8_CHAR_MAX_LEN+1;
			src = euc_jp_kanji;
			dst = utf8char;
			if (g_iconv(cd, &src, &inbytes_left, &dst, &outbytes_left) == -1) {
				g_string_append(string, "[?]");
			} else {
				g_string_append(string, utf8char);
			}
			break;
		case ISO_2022_JP_TYPE_JISX_0201_KANA:
			if (IS_ISO_2022_JP_7BIT_HALFWIDTH_KANA(c)) {
				g_string_append_unichar(string,
							(gunichar) HALFWIDTH_KATAKANA_UCS4_OFFSET -
							HALFWIDTH_KATAKANA_7BIT_OFFSET +
							c);
			} else {
				g_string_append(string, "[!]");
			}
			break;
		case ISO_2022_JP_TYPE_JISX_0201_1976:
			if (c == 0x5c) { /* yen mark */
				g_string_append_unichar(string, (gunichar) 0xc2a5);
			} else if (c == '~') { /* upper line */
				g_string_append_unichar(string, (gunichar) 0xe280be);
			} else if (c == ISO_2022_JP_SHIFT_OUT) {
				shift_out_mode = TRUE;
			} else if (c == ISO_2022_JP_SHIFT_IN) {
				shift_out_mode = FALSE;
			} else if (shift_out_mode && IS_ISO_2022_JP_7BIT_HALFWIDTH_KANA(c)) {
				g_string_append_unichar(string,
							(gunichar) HALFWIDTH_KATAKANA_UCS4_OFFSET -
							HALFWIDTH_KATAKANA_7BIT_OFFSET +
							c);
			} else if (IS_ISO_2022_JP_8BIT_HALFWIDTH_KANA(c)) {
				g_string_append_unichar(string,
							(gunichar) HALFWIDTH_KATAKANA_UCS4_OFFSET -
							HALFWIDTH_KATAKANA_8BIT_OFFSET +
							c);
			} else if (isascii(c)) {
				g_string_append_c(string, c);
			} else {
				g_string_append(string, "[!]");
			}
			break;
		case ISO_2022_JP_TYPE_ASCII:
		default:
			if (isascii(c)) {
				g_string_append_c(string, c);
			} else {
				g_string_append(string, "[!]");
			}
		}
		cur++;
	}

	g_iconv_close(cd);

	return g_string_free(string, FALSE);
}

typedef struct UnicharPair {
	gunichar orig;
	gunichar ms;
} UnicharPair;

UnicharPair ms_diff_table[] = { {0x301c, 0xff5e},
				{0, 0} };

gchar *
loqui_codeconv_tools_utf8_from_ms_table(const gchar *str)
{
	int i, len;
	gunichar u;
	GString *dest;
	const gchar *p;

	if (str == NULL)
		return NULL;
	len = strlen(str);
	if (len == 0)
		return g_strdup("");

	dest = g_string_sized_new(len);

	p = str;
	do {
		u = g_utf8_get_char(p);
		for (i = 0; ms_diff_table[i].ms != 0; i++) {
			if (u == ms_diff_table[i].ms) {
				u = ms_diff_table[i].orig;
			}
		}
		g_string_append_unichar(dest, u);
	} while ((p = g_utf8_next_char(p)) != NULL);

	return g_string_free(dest, FALSE);
}

gchar *
loqui_codeconv_tools_utf8_to_ms_table(const gchar *str)
{

	int i, len;
	gunichar u;
	GString *dest;
	const gchar *p;

	if (str == NULL)
		return NULL;
	len = strlen(str);
	if (len == 0)
		return g_strdup("");

	dest = g_string_sized_new(len);

	p = str;
	do {
		u = g_utf8_get_char(p);
		for (i = 0; ms_diff_table[i].ms != 0; i++) {
			if (u == ms_diff_table[i].orig) {
				u = ms_diff_table[i].ms;
			}
		}
		g_string_append_unichar(dest, u);
	} while ((p = g_utf8_next_char(p)) != NULL);

	return g_string_free(dest, FALSE);
}
