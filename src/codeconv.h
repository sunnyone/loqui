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
#ifndef __CODECONV_H__
#define __CODECONV_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define TYPE_CODECONV                 (codeconv_get_type ())
#define CODECONV(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_CODECONV, CodeConv))
#define CODECONV_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_CODECONV, CodeConvClass))
#define IS_CODECONV(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_CODECONV))
#define IS_CODECONV_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_CODECONV))
#define CODECONV_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_CODECONV, CodeConvClass))

typedef struct _CodeConv            CodeConv;
typedef struct _CodeConvClass       CodeConvClass;

typedef struct _CodeConvPrivate     CodeConvPrivate;

struct _CodeConv
{
        GObject parent;
        
        CodeConvPrivate *priv;
};

struct _CodeConvClass
{
        GObjectClass parent_class;
};

typedef gchar * (*CodeConvFunc) (const gchar *input);

typedef struct _CodeConvDef {
	gchar *title;
	gchar *locale;
	gchar *codeset;
	CodeConvFunc func;
} CodeConvDef;

typedef enum {
	CODESET_TYPE_AUTO_DETECTION,
	CODESET_TYPE_NO_CONV,
	CODESET_TYPE_CUSTOM,
	CODESET_TYPE_JAPANESE,
	N_CODESET_TYPE,
} CodeSetType;

extern CodeConvDef conv_table[];

GType codeconv_get_type (void) G_GNUC_CONST;

CodeConv* codeconv_new(void);

void codeconv_set_codeset_type(CodeConv *codeconv, CodeSetType type);
CodeSetType codeconv_get_codeset_type(CodeConv *codeconv);

void codeconv_set_codeset(CodeConv *codeconv, const gchar *codeset);
G_CONST_RETURN gchar *codeconv_get_codeset(CodeConv *codeconv);

/* return value must be freed. */
gchar *codeconv_to_server(CodeConv *codeconv, const gchar *input);
gchar *codeconv_to_local(CodeConv *codeconv, const gchar *input);

G_END_DECLS

#endif /* __CODECONV_H__ */
