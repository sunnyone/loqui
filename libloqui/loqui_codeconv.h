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

struct _LoquiCodeConv
{
        GObject parent;
        
        LoquiCodeConvPrivate *priv;
};

struct _LoquiCodeConvClass
{
        GObjectClass parent_class;
};

typedef gchar * (*LoquiCodeConvFunc) (const gchar *input);

typedef struct _LoquiCodeConvDef {
	gchar *title;
	gchar *locale;
	gchar *codeset;
	LoquiCodeConvFunc func;
} LoquiCodeConvDef;

typedef enum {
	CODESET_TYPE_AUTO_DETECTION,
	CODESET_TYPE_NO_CONV,
	CODESET_TYPE_CUSTOM,
	CODESET_TYPE_JAPANESE,
	N_CODESET_TYPE,
} CodeSetType;

extern LoquiCodeConvDef conv_table[];

GType loqui_codeconv_get_type (void) G_GNUC_CONST;

LoquiCodeConv* loqui_codeconv_new(void);

void loqui_codeconv_set_codeset_type(LoquiCodeConv *codeconv, CodeSetType type);
CodeSetType loqui_codeconv_get_codeset_type(LoquiCodeConv *codeconv);

void loqui_codeconv_set_codeset(LoquiCodeConv *codeconv, const gchar *codeset);
G_CONST_RETURN gchar *loqui_codeconv_get_codeset(LoquiCodeConv *codeconv);

/* return value must be freed. */
gchar *loqui_codeconv_to_server(LoquiCodeConv *codeconv, const gchar *input);
gchar *loqui_codeconv_to_local(LoquiCodeConv *codeconv, const gchar *input);

G_END_DECLS

#endif /* __LOQUI_CODECONV_H__ */
