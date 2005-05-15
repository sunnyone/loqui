/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * libloqui -- Chat/IM client library for GLib <http://loqui.good-day.net/>
 * Copyright (C) 2002-2004 Yoichi Imai <yoichi@silver-forest.com>
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
#ifndef __LOQUI_CODECONV_H__
#define __LOQUI_CODECONV_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define LOQUI_TYPE_CODECONV                 (loqui_codeconv_get_type ())
#define LOQUI_CODECONV(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), LOQUI_TYPE_CODECONV, LoquiCodeConv))
#define LOQUI_CODECONV_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), LOQUI_TYPE_CODECONV, LoquiCodeConvClass))
#define LOQUI_IS_CODECONV(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LOQUI_TYPE_CODECONV))
#define LOQUI_IS_CODECONV_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), LOQUI_TYPE_CODECONV))
#define LOQUI_CODECONV_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), LOQUI_TYPE_CODECONV, LoquiCodeConvClass))

typedef struct _LoquiCodeConv            LoquiCodeConv;
typedef struct _LoquiCodeConvClass       LoquiCodeConvClass;

typedef struct _LoquiCodeConvPrivate     LoquiCodeConvPrivate;
typedef struct _LoquiCodeConvTableItem   LoquiCodeConvTableItem;

typedef gchar * (*LoquiCodeConvFunc) (LoquiCodeConv *codeconv, gboolean is_to_server, const gchar *input, GError **error);

typedef enum {
	LOQUI_CODECONV_MODE_AUTOMATIC,
	LOQUI_CODECONV_MODE_NO_CONV,
	LOQUI_CODECONV_MODE_BY_TABLE,
	LOQUI_CODECONV_MODE_CODESET
} LoquiCodeConvMode;

#define LOQUI_CODECONV_ERROR loqui_codeconv_error_quark()

typedef enum {
	LOQUI_CODECONV_ERROR_CONVERT,
	LOQUI_CODECONV_ERROR_TABLE_NOT_SET,
	LOQUI_CODECONV_ERROR_FAILED_SELECT_ITEM,
	LOQUI_CODECONV_ERROR_INVALID_MODE,
	LOQUI_CODECONV_ERROR_FAILED_OPEN_ICONV,
} LoquiCodeConvError;
	
struct _LoquiCodeConv
{
        GObject parent;
        
	LoquiCodeConvTableItem *table;

	LoquiCodeConvMode mode;
	gchar *name;
	gchar *codeset;


	GIConv cd_to_server;
	GIConv cd_to_local;
	
        LoquiCodeConvPrivate *priv;
};

struct _LoquiCodeConvClass
{
        GObjectClass parent_class;
};

struct _LoquiCodeConvTableItem {
	gchar *name;
	gchar *title;
	gchar *description;
	gchar *locale;
	LoquiCodeConvFunc func; /* if func is NULL, codeset is used */
	gchar *codeset;
	gchar *codeset_secondary;
};

GType loqui_codeconv_get_type(void) G_GNUC_CONST;

GQuark loqui_codeconv_error_quark(void);

LoquiCodeConv* loqui_codeconv_new(void);

gboolean loqui_codeconv_update(LoquiCodeConv *codeconv, GError **error);

/* NULL terminated and static item array */
void loqui_codeconv_set_table(LoquiCodeConv *codeconv, LoquiCodeConvTableItem *table);
LoquiCodeConvTableItem *loqui_codeconv_get_table(LoquiCodeConv *codeconv);

/* return value must be freed. */
gchar *loqui_codeconv_to_server(LoquiCodeConv *codeconv, const gchar *input, GError **error);
gchar *loqui_codeconv_to_local(LoquiCodeConv *codeconv, const gchar *input, GError **error);


void loqui_codeconv_set_mode(LoquiCodeConv *codeconv, LoquiCodeConvMode mode);
LoquiCodeConvMode loqui_codeconv_get_mode(LoquiCodeConv *codeconv);

void loqui_codeconv_set_codeset(LoquiCodeConv *codeconv, const gchar *codeset);
G_CONST_RETURN gchar *loqui_codeconv_get_codeset(LoquiCodeConv *codeconv);

void loqui_codeconv_set_table_item_name(LoquiCodeConv *codeconv, const gchar *name);
G_CONST_RETURN gchar *loqui_codeconv_get_table_item_name(LoquiCodeConv *codeconv);

/* utilities */
LoquiCodeConvTableItem *loqui_codeconv_find_table_item_by_locale(LoquiCodeConvTableItem *table);
LoquiCodeConvTableItem *loqui_codeconv_find_table_item_by_name(LoquiCodeConvTableItem *table, const gchar *name);
G_CONST_RETURN gchar *loqui_codeconv_translate(const gchar *message);

G_END_DECLS

#endif /* __LOQUI_CODECONV_H__ */
