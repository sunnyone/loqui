/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui -- Chat client for Gtk2 <http://loqui.good-day.net/>
 * Copyright (C) 2004 Yoichi Imai <yoichi@silver-forest.com>
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
#include "codeconv_ja.h"
#include <ctype.h>
#include <string.h>

/* if you want to use EUC-JP-MS table, set it */
#define ICONV_EUC_JP_CODESET "EUC-JP"

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
codeconv_ja_jis_to_utf8(const gchar *input)
{
	const gchar *cur;
	gboolean shift_out_mode = FALSE;
	GString *string;
	guchar c, c1, c2;
	gchar euc_jp_kanji[EUC_JP_CHAR_MAX_LEN+1];
	gchar utf8char[UTF8_CHAR_MAX_LEN+1];
	gchar *src, *dst;
	gsize inbytes_left, outbytes_left;
	ISO2022JPAreaType area_type;
	GIConv cd;

	cd = g_iconv_open("UTF-8", ICONV_EUC_JP_CODESET);
	if (cd < 0)
		return NULL;

	string = g_string_new_len(NULL, strlen(input));
	cur = input;
	area_type = ISO_2022_JP_TYPE_ASCII;
	while (*cur != '\0') {
		if (*cur == ISO_2022_ESC) {
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
		c = *cur;
		switch (area_type) {
		case ISO_2022_JP_TYPE_JISX_0208_1983:
		case ISO_2022_JP_TYPE_JISX_0208_1978:
			cur++;
			c2 = *cur;
			if (c2 == '\0')
				break;
			euc_jp_kanji[0] = c | 0x80;
			euc_jp_kanji[1] = c2 | 0x80;
			euc_jp_kanji[2] = 0;
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
