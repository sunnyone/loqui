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

#include "loqui_common_text.h"

struct _LoquiCommonTextPrivate
{
};

static GtkTextViewClass *parent_class = NULL;
#define PARENT_TYPE GTK_TYPE_TEXT_VIEW

static void loqui_common_text_class_init (LoquiCommonTextClass *klass);
static void loqui_common_text_init (LoquiCommonText *loqui_common_text);
static void loqui_common_text_finalize (GObject *object);
static void loqui_common_text_destroy (GtkObject *object);

GType
loqui_common_text_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(LoquiCommonTextClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) loqui_common_text_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(LoquiCommonText),
				0,              /* n_preallocs */
				(GInstanceInitFunc) loqui_common_text_init
			};
		
		type = g_type_register_static(PARENT_TYPE,
					      "LoquiCommonText",
					      &our_info,
					      0);
	}
	
	return type;
}
static void
loqui_common_text_class_init (LoquiCommonTextClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);
        GtkObjectClass *gtk_object_class = GTK_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = loqui_common_text_finalize;
        gtk_object_class->destroy = loqui_common_text_destroy;
}
static void 
loqui_common_text_init (LoquiCommonText *common_text)
{
	LoquiCommonTextPrivate *priv;

	priv = g_new0(LoquiCommonTextPrivate, 1);

	common_text->priv = priv;
}
static void 
loqui_common_text_destroy (GtkObject *object)
{
	LoquiCommonText *common_text;

        g_return_if_fail (object != NULL);
        g_return_if_fail (LOQUI_IS_COMMON_TEXT (object));

        common_text = LOQUI_COMMON_TEXT(object);

        if (GTK_OBJECT_CLASS(parent_class)->destroy)
                (* GTK_OBJECT_CLASS(parent_class)->destroy) (object);
}
static void 
loqui_common_text_finalize (GObject *object)
{
	LoquiCommonText *common_text;

        g_return_if_fail (object != NULL);
        g_return_if_fail (LOQUI_IS_COMMON_TEXT (object));

        common_text = LOQUI_COMMON_TEXT(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize) (object);

	g_free(common_text->priv);
}
GtkWidget*
loqui_common_text_new (void)
{
        LoquiCommonText *common_text;
	LoquiCommonTextPrivate *priv;

	common_text = g_object_new(loqui_common_text_get_type(), NULL);
	
	gtk_text_view_set_editable(GTK_TEXT_VIEW(common_text), FALSE);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(common_text), GTK_WRAP_CHAR);

	return GTK_WIDGET(common_text);
}
