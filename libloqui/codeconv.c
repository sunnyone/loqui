/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui -- IRC client for Gtk2 <http://loqui.good-day.net/>
 * Copyright (C) 2003 Yoichi Imai <yoichi@silver-forest.com>
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
#include <ctype.h>
#include <locale.h>
#include <errno.h>
#include "utils.h"

#include "loqui_codeconv_tools.h"

#define GTK_CODESET "UTF-8"
#define BUFFER_LEN 2048

/* tell me other languages if you know */
/* if you changed this table, make sure to change codeconv.h */
CodeConvDef conv_table[] = {
	{N_("Auto Detection"), NULL,       NULL, NULL}, /* for the setting */
	{N_("No conv"),        NULL,       NULL, NULL}, /* for the setting */
	{N_("Custom"),         NULL,       NULL, NULL}, /* for the setting */
	{N_("Japanese"),       "ja_JP",    "ISO-2022-JP", loqui_codeconv_tools_jis_to_utf8},
};

struct _CodeConvPrivate
{
	CodeSetType code_type;
	CodeConvFunc func;
	gchar *server_codeset;
	GIConv cd_to;
	GIConv cd_from;
};

static GObjectClass *parent_class = NULL;
#define PARENT_TYPE G_TYPE_OBJECT

static void codeconv_class_init(CodeConvClass *klass);
static void codeconv_init(CodeConv *codeconv);
static void codeconv_finalize(GObject *object);

static void codeconv_set_codeset_internal(CodeConv *codeconv, const gchar *codeset);

GType
codeconv_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(CodeConvClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) codeconv_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(CodeConv),
				0,              /* n_preallocs */
				(GInstanceInitFunc) codeconv_init
			};
		
		type = g_type_register_static(PARENT_TYPE,
					      "CodeConv",
					      &our_info,
					      0);
	}
	
	return type;
}
static void
codeconv_class_init(CodeConvClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = codeconv_finalize;
}
static void 
codeconv_init(CodeConv *codeconv)
{
	CodeConvPrivate *priv;

	priv = g_new0(CodeConvPrivate, 1);

	codeconv->priv = priv;
	codeconv_set_codeset_type(codeconv, CODESET_TYPE_NO_CONV);
}
static void 
codeconv_finalize(GObject *object)
{
	CodeConv *codeconv;

        g_return_if_fail(object != NULL);
        g_return_if_fail(IS_CODECONV(object));

        codeconv = CODECONV(object);

	G_FREE_UNLESS_NULL(codeconv->priv->server_codeset);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize) (object);

	g_free(codeconv->priv);
}

CodeConv*
codeconv_new(void)
{
        CodeConv *codeconv;

	codeconv = g_object_new(codeconv_get_type(), NULL);
	
	return codeconv;
}

void
codeconv_set_codeset_type(CodeConv *codeconv, CodeSetType type)
{
	int i;
	gchar *ctype;
	const gchar *codeset;
	CodeConvPrivate *priv;

        g_return_if_fail(codeconv != NULL);
        g_return_if_fail(IS_CODECONV(codeconv));
	g_return_if_fail(type < N_CODESET_TYPE);

	priv = codeconv->priv;
	
	priv->code_type = type;
	switch (type) {
	case CODESET_TYPE_AUTO_DETECTION:
		ctype = setlocale(LC_CTYPE, NULL);
		codeset = NULL;
		if (ctype) {
			for (i = CODESET_TYPE_CUSTOM+1; i < N_CODESET_TYPE; i++) {
				if (strstr(ctype, conv_table[i].locale) != NULL) {
					codeset = conv_table[i].codeset;
					priv->func = conv_table[i].func;
					break;
				}
			}
		}
		codeconv_set_codeset_internal(codeconv, codeset);
		break;
	case CODESET_TYPE_NO_CONV:
		codeconv_set_codeset_internal(codeconv, NULL);
		break;
	case CODESET_TYPE_CUSTOM:
		break;
	default:
		priv->func = conv_table[type].func;
		codeconv_set_codeset_internal(codeconv, conv_table[type].codeset);
		break;
	}
}
CodeSetType
codeconv_get_codeset_type(CodeConv *codeconv)
{
        g_return_val_if_fail(codeconv != NULL, 0);
        g_return_val_if_fail(IS_CODECONV(codeconv), 0);
	
	return codeconv->priv->code_type;
}
static void
codeconv_set_codeset_internal(CodeConv *codeconv, const gchar *codeset)
{
	CodeConvPrivate *priv;

        g_return_if_fail(codeconv != NULL);
        g_return_if_fail(IS_CODECONV(codeconv));

	priv = codeconv->priv;

	G_FREE_UNLESS_NULL(priv->server_codeset);

	if(priv->cd_from)
		g_iconv_close(priv->cd_from);
	priv->cd_from = NULL;

	if(priv->cd_to)
		g_iconv_close(priv->cd_to);
	priv->cd_to = NULL;

	if(!codeset)
		return;

	priv->server_codeset = g_strdup(codeset);

	priv->cd_from = g_iconv_open(GTK_CODESET, codeset);
	priv->cd_to = g_iconv_open(codeset, GTK_CODESET);
}
void
codeconv_set_codeset(CodeConv *codeconv, const gchar *codeset)
{
        g_return_if_fail(codeconv != NULL);
        g_return_if_fail(IS_CODECONV(codeconv));
	g_return_if_fail(codeconv->priv->code_type == CODESET_TYPE_CUSTOM);
	g_return_if_fail(codeset != NULL);

	if(strlen(codeset) == 0) {
		g_warning(_("Invalid codeset string (length zero)"));
		return;
	}

	codeconv_set_codeset_internal(codeconv, codeset);
}
G_CONST_RETURN gchar *
codeconv_get_codeset(CodeConv *codeconv)
{
        g_return_val_if_fail(codeconv != NULL, NULL);
        g_return_val_if_fail(IS_CODECONV(codeconv), NULL);

	if(codeconv->priv->code_type == CODESET_TYPE_NO_CONV)
		return GTK_CODESET;
	else
		return codeconv->priv->server_codeset;
}

gchar *
codeconv_to_server(CodeConv *codeconv, const gchar *input)
{
	CodeConvPrivate *priv;
	gchar *output;
	GError *error = NULL;

        g_return_val_if_fail(codeconv != NULL, NULL);
        g_return_val_if_fail(IS_CODECONV(codeconv), NULL);

	priv = codeconv->priv;

	if(input == NULL)
		return NULL;
	if(priv->code_type == CODESET_TYPE_NO_CONV || !priv->cd_to)
		return g_strdup(input);

	output = g_convert_with_iconv(input, strlen(input)+1, priv->cd_to,
				      NULL, NULL, &error);
	if(error != NULL) {
		g_warning(_("Code convartion error: %s"), error->message);
		g_error_free(error);
	}

	return output;
}

static gchar *
codeconv_convert(CodeConv *codeconv, const gchar *input, GIConv cd)
{
	gchar *tmp;
	gchar buf[BUFFER_LEN+1];
	gsize original_len;
	GString *string;

	gchar *inbuf;
	gsize in_left;
	gchar *outbuf;
        gsize out_left;
	size_t ret;
		
	/* we use a compilicated way to handle broken characters */
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
				g_warning(_("Unknown error occured in code convertion."));
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

	tmp = string->str;
	g_string_free(string, FALSE);
	
	return tmp;	
}
gchar *
codeconv_to_local(CodeConv *codeconv, const gchar *input)
{
	CodeConvPrivate *priv;
	gchar *buf;
	
        g_return_val_if_fail(codeconv != NULL, NULL);
        g_return_val_if_fail(IS_CODECONV(codeconv), NULL);

	priv = codeconv->priv;

	if(input == NULL)
		return NULL;
	if(priv->code_type == CODESET_TYPE_NO_CONV || !priv->cd_from)
		return g_strdup(input);

	if(priv->func) {
		buf = priv->func(input);
	} else {
		buf = codeconv_convert(codeconv, input, priv->cd_from);
	}
	
	return buf;
}
