/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * libloqui -- Chat/IM client library for GLib <http://launchpad.net/loqui/>
 * Copyright (C) 2002-2004 Yoichi Imai <sunnyone41@gmail.com>
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

#include "loqui_codeconv.h"

#include <libloqui-intl.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "loqui-utils.h"

#include "loqui_codeconv_tools.h"

#define GTK_CODESET "UTF-8"
#define BUFFER_LEN 2048

struct _LoquiCodeConvPrivate
{
	LoquiCodeConvFunc func;
};

static GObjectClass *parent_class = NULL;
#define PARENT_TYPE G_TYPE_OBJECT

static void loqui_codeconv_class_init(LoquiCodeConvClass *klass);
static void loqui_codeconv_init(LoquiCodeConv *codeconv);
static void loqui_codeconv_finalize(GObject *object);

#define LOQUI_CODECONV_G_ICONV_INVALIDATE(cd_p) { \
	*cd_p = (GIConv) -1; \
}

#define LOQUI_CODECONV_G_ICONV_IS_VALID(cd) (cd != (GIConv) -1)

GType
loqui_codeconv_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(LoquiCodeConvClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) loqui_codeconv_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(LoquiCodeConv),
				0,              /* n_preallocs */
				(GInstanceInitFunc) loqui_codeconv_init
			};
		
		type = g_type_register_static(PARENT_TYPE,
					      "LoquiCodeConv",
					      &our_info,
					      0);
	}
	
	return type;
}
static void
loqui_codeconv_class_init(LoquiCodeConvClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = loqui_codeconv_finalize;
}
static void 
loqui_codeconv_init(LoquiCodeConv *codeconv)
{
	LoquiCodeConvPrivate *priv;

	priv = g_new0(LoquiCodeConvPrivate, 1);

	codeconv->priv = priv;
	codeconv->mode = LOQUI_CODECONV_MODE_AUTOMATIC;
	LOQUI_CODECONV_G_ICONV_INVALIDATE(&codeconv->cd_to_server);
	LOQUI_CODECONV_G_ICONV_INVALIDATE(&codeconv->cd_to_local);
}
static void 
loqui_codeconv_finalize(GObject *object)
{
	LoquiCodeConv *codeconv;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_CODECONV(object));

        codeconv = LOQUI_CODECONV(object);

	LOQUI_G_FREE_UNLESS_NULL(codeconv->name);
	LOQUI_G_FREE_UNLESS_NULL(codeconv->codeset);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize) (object);

	g_free(codeconv->priv);
}
GQuark
loqui_codeconv_error_quark(void)
{
        static GQuark quark = 0;
        if (!quark)
                quark = g_quark_from_static_string("loqui-codeconv-error-quark");

        return quark;
}
LoquiCodeConv*
loqui_codeconv_new(void)
{
        LoquiCodeConv *codeconv;

	codeconv = g_object_new(loqui_codeconv_get_type(), NULL);
	
	return codeconv;
}

/* NULL terminated and static item array */
void
loqui_codeconv_set_table(LoquiCodeConv *codeconv, LoquiCodeConvTableItem *table)
{
        g_return_if_fail(codeconv != NULL);
        g_return_if_fail(LOQUI_IS_CODECONV(codeconv));

	codeconv->table = table;
}
LoquiCodeConvTableItem *
loqui_codeconv_get_table(LoquiCodeConv *codeconv)
{
        g_return_val_if_fail(codeconv != NULL, NULL);
        g_return_val_if_fail(LOQUI_IS_CODECONV(codeconv), NULL);

	return codeconv->table;
}

void
loqui_codeconv_set_mode(LoquiCodeConv *codeconv, LoquiCodeConvMode mode)
{
        g_return_if_fail(codeconv != NULL);
        g_return_if_fail(LOQUI_IS_CODECONV(codeconv));
	
	codeconv->mode = mode;
}
LoquiCodeConvMode
loqui_codeconv_get_mode(LoquiCodeConv *codeconv)
{
        g_return_val_if_fail(codeconv != NULL, 0);
        g_return_val_if_fail(LOQUI_IS_CODECONV(codeconv), 0);
	
	return codeconv->mode;
}
void
loqui_codeconv_set_table_item_name(LoquiCodeConv *codeconv, const gchar *name)
{
        g_return_if_fail(codeconv != NULL);
        g_return_if_fail(LOQUI_IS_CODECONV(codeconv));
	
	LOQUI_G_FREE_UNLESS_NULL(codeconv->name);
	codeconv->name = g_strdup(name);
}
G_CONST_RETURN gchar *
loqui_codeconv_get_table_item_name(LoquiCodeConv *codeconv)
{
        g_return_val_if_fail(codeconv != NULL, NULL);
        g_return_val_if_fail(LOQUI_IS_CODECONV(codeconv), NULL);
	
	return codeconv->name;
}
void
loqui_codeconv_set_codeset(LoquiCodeConv *codeconv, const gchar *codeset)
{
        g_return_if_fail(codeconv != NULL);
        g_return_if_fail(LOQUI_IS_CODECONV(codeconv));
	
	LOQUI_G_FREE_UNLESS_NULL(codeconv->codeset);
	codeconv->codeset = g_strdup(codeset);
}
G_CONST_RETURN gchar *
loqui_codeconv_get_codeset(LoquiCodeConv *codeconv)
{
        g_return_val_if_fail(codeconv != NULL, NULL);
        g_return_val_if_fail(LOQUI_IS_CODECONV(codeconv), NULL);
	
	return codeconv->codeset;
}
gboolean
loqui_codeconv_update(LoquiCodeConv *codeconv, GError **error)
{
	LoquiCodeConvPrivate *priv;
	LoquiCodeConvTableItem *item = NULL;

        g_return_val_if_fail(codeconv != NULL, FALSE);
        g_return_val_if_fail(LOQUI_IS_CODECONV(codeconv), FALSE);

	priv = codeconv->priv;

	if (codeconv->cd_to_local >= 0)
		g_iconv_close(codeconv->cd_to_local);
	LOQUI_CODECONV_G_ICONV_INVALIDATE(&codeconv->cd_to_local);

	if (codeconv->cd_to_server >= 0)
		g_iconv_close(codeconv->cd_to_server);
	LOQUI_CODECONV_G_ICONV_INVALIDATE(&codeconv->cd_to_server);

	priv->func = NULL;

#define CHECK_TABLE_IS_SET() { \
	if (codeconv->table == NULL) { \
		g_set_error(error, \
			    LOQUI_CODECONV_ERROR, \
			    LOQUI_CODECONV_ERROR_TABLE_NOT_SET, \
			    "Table is not set"); \
                return FALSE; \
	} \
}

	switch (codeconv->mode) {
	case LOQUI_CODECONV_MODE_AUTOMATIC:
		CHECK_TABLE_IS_SET();
		item = loqui_codeconv_find_table_item_by_locale(codeconv->table);
		break;
	case LOQUI_CODECONV_MODE_NO_CONV:
		item = NULL;
		break;
	case LOQUI_CODECONV_MODE_BY_TABLE:
		CHECK_TABLE_IS_SET();
		item = NULL;
		if (codeconv->name != NULL)
			item = loqui_codeconv_find_table_item_by_name(codeconv->table, codeconv->name);
		if (item == NULL) {
			g_set_error(error,
				    LOQUI_CODECONV_ERROR,
				    LOQUI_CODECONV_ERROR_FAILED_SELECT_ITEM,
				    "The item named '%s' is not found", codeconv->name ? codeconv->name : "(null)");
			return FALSE;
		}
		break;
	case LOQUI_CODECONV_MODE_CODESET:
		break;
	default:
		g_set_error(error,
			    LOQUI_CODECONV_ERROR,
			    LOQUI_CODECONV_ERROR_INVALID_MODE,
			    "Invalid mode '%d'", codeconv->mode);
		return FALSE;
	}
	if (codeconv->mode == LOQUI_CODECONV_MODE_CODESET) {
		if (codeconv->codeset && strlen(codeconv->codeset) > 0) {
			codeconv->cd_to_local = g_iconv_open(GTK_CODESET, codeconv->codeset);
			codeconv->cd_to_server = g_iconv_open(codeconv->codeset, GTK_CODESET);
		}

		if (codeconv->cd_to_local == NULL ||
		    codeconv->cd_to_server == NULL) {
			g_set_error(error,
				    LOQUI_CODECONV_ERROR,
				    LOQUI_CODECONV_ERROR_FAILED_OPEN_ICONV,
				    "Failed to open iconv for the codeset '%s'",
				    codeconv->codeset ? codeconv->codeset : "(null)");
			return FALSE;
		}
	} else {
		if (item) {
			if (item->codeset) {
				codeconv->cd_to_local = g_iconv_open(GTK_CODESET, item->codeset);
				if (codeconv->cd_to_local >= 0) {
					codeconv->cd_to_server = g_iconv_open(item->codeset, GTK_CODESET);
				} else {
					codeconv->cd_to_local = g_iconv_open(GTK_CODESET, item->codeset_secondary);
					codeconv->cd_to_server = g_iconv_open(item->codeset_secondary, GTK_CODESET);
				}

				if (codeconv->cd_to_local < 0) {
					g_set_error(error,
						    LOQUI_CODECONV_ERROR,
						    LOQUI_CODECONV_ERROR_FAILED_OPEN_ICONV,
						    "Failed to open iconv");
					return FALSE;
				}
			}

			priv->func = item->func;
		}
	}

	return TRUE;
}
LoquiCodeConvTableItem *
loqui_codeconv_find_table_item_by_locale(LoquiCodeConvTableItem *table)
{
	gchar *ctype;
	int i;

	ctype = loqui_utils_get_lc_ctype();
	for (i = 0; table[i].name; i++) {
		if (table[i].locale == NULL) { /* when the 'locale' field is NULL, selected automatically. */
			g_free(ctype);
			return &table[i];
		}

		if (ctype == NULL)
			continue;

		if (g_str_has_prefix(ctype, table[i].locale)) {
			g_free(ctype);
			return &table[i];
		}
	}

	g_free(ctype);
	return NULL;
}

LoquiCodeConvTableItem *
loqui_codeconv_find_table_item_by_name(LoquiCodeConvTableItem *table, const gchar *name)
{
	int i;

	g_return_val_if_fail(name != NULL, NULL);
	g_return_val_if_fail(table != NULL, NULL);

	for (i = 0; table[i].name != NULL; i++) {
		if (strcmp(table[i].name, name) == 0) {
			return &table[i];
		}
	}

	return NULL;
}

G_CONST_RETURN gchar *
loqui_codeconv_translate(const gchar *message)
{
	return dgettext(GETTEXT_PACKAGE, message);
}

gchar *
loqui_codeconv_to_server(LoquiCodeConv *codeconv, const gchar *input, GError **error)
{
	LoquiCodeConvPrivate *priv;
	gchar *output;
	gchar *tmp = NULL;

        g_return_val_if_fail(codeconv != NULL, NULL);
        g_return_val_if_fail(LOQUI_IS_CODECONV(codeconv), NULL);
	g_return_val_if_fail(input != NULL, NULL);

	priv = codeconv->priv;

	if (!priv->func && !LOQUI_CODECONV_G_ICONV_IS_VALID(codeconv->cd_to_server))
		return g_strdup(input);

#ifdef G_OS_WIN32
	tmp = loqui_codeconv_tools_utf8_from_ms_table(input);
	input = tmp;
#endif

	if (priv->func) {
		output = priv->func(codeconv, TRUE, input, error);
	} else {
		output = g_convert_with_iconv(input, strlen(input)+1, codeconv->cd_to_server,
					      NULL, NULL, error);
		// if error, initialize iconv_t
		if (error != NULL && *error != NULL) {
			g_iconv(codeconv->cd_to_server, NULL, NULL, NULL, NULL);
		}
	}
	
	g_free(tmp);

	return output;
}

static gchar *
loqui_codeconv_convert(LoquiCodeConv *codeconv, const gchar *input, GIConv cd, GError **error)
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
				g_set_error(error,
					    LOQUI_CODECONV_ERROR,
					    LOQUI_CODECONV_ERROR_CONVERT,
					    "Unknown error occured in code convertion(errno: %d).", errno);
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
loqui_codeconv_to_local(LoquiCodeConv *codeconv, const gchar *input, GError **error)
{
	LoquiCodeConvPrivate *priv;
	gchar *buf;

#ifdef G_OS_WIN32
	gchar *tmp = NULL;
#endif

        g_return_val_if_fail(codeconv != NULL, NULL);
        g_return_val_if_fail(LOQUI_IS_CODECONV(codeconv), NULL);
	g_return_val_if_fail(input != NULL, NULL);

	priv = codeconv->priv;

	if(!priv->func && !LOQUI_CODECONV_G_ICONV_IS_VALID(codeconv->cd_to_local))
		return g_strdup(input);

	if (priv->func) {
		buf = priv->func(codeconv, FALSE, input, error);
	} else {
		buf = loqui_codeconv_convert(codeconv, input, codeconv->cd_to_local, error);
	}
	
#ifdef G_OS_WIN32
	tmp = loqui_codeconv_tools_utf8_to_ms_table(buf);
	g_free(buf);
	buf = tmp;
#endif

	return buf;

}
