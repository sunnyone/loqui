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

#include "loqui_style_entry.h"
#include "utils.h"

#include <string.h>

enum {
        LAST_SIGNAL
};

enum {
        LAST_PROP
};

struct _LoquiStyleEntryPrivate
{
};

static GObjectClass *parent_class = NULL;

/* static guint loqui_style_entry_signals[LAST_SIGNAL] = { 0 }; */

static GObject* loqui_style_entry_constructor(GType type, guint n_props, GObjectConstructParam *props);

static void loqui_style_entry_class_init(LoquiStyleEntryClass *klass);
static void loqui_style_entry_init(LoquiStyleEntry *entry);
static void loqui_style_entry_finalize(GObject *object);
static void loqui_style_entry_dispose(GObject *object);

static void loqui_style_entry_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec);
static void loqui_style_entry_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec);

GType
loqui_style_entry_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(LoquiStyleEntryClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) loqui_style_entry_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(LoquiStyleEntry),
				0,              /* n_preallocs */
				(GInstanceInitFunc) loqui_style_entry_init
			};
		
		type = g_type_register_static(G_TYPE_OBJECT,
					      "LoquiStyleEntry",
					      &our_info,
					      0);
	}
	
	return type;
}
static GObject*
loqui_style_entry_constructor(GType type, guint n_props, GObjectConstructParam *props)
{
        GObject *object;
        GObjectClass *object_class = G_OBJECT_CLASS(parent_class);

	object = object_class->constructor(type, n_props, props);

	return object;
}
static void 
loqui_style_entry_finalize(GObject *object)
{
	LoquiStyleEntry *entry;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_STYLE_ENTRY(object));

        entry = LOQUI_STYLE_ENTRY(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize)(object);

	g_free(entry->priv);
}
static void 
loqui_style_entry_dispose(GObject *object)
{
	LoquiStyleEntry *entry;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_STYLE_ENTRY(object));

        entry = LOQUI_STYLE_ENTRY(object);

        if (G_OBJECT_CLASS(parent_class)->dispose)
                (* G_OBJECT_CLASS(parent_class)->dispose)(object);
}
static void
loqui_style_entry_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec)
{
        LoquiStyleEntry *entry;        

        entry = LOQUI_STYLE_ENTRY(object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}
static void
loqui_style_entry_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec)
{
        LoquiStyleEntry *entry;        

        entry = LOQUI_STYLE_ENTRY(object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}

static void
loqui_style_entry_class_init(LoquiStyleEntryClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);

	object_class->constructor = loqui_style_entry_constructor; 
        object_class->finalize = loqui_style_entry_finalize;
        object_class->dispose = loqui_style_entry_dispose;
        object_class->get_property = loqui_style_entry_get_property;
        object_class->set_property = loqui_style_entry_set_property;
}
static void 
loqui_style_entry_init(LoquiStyleEntry *entry)
{
	LoquiStyleEntryPrivate *priv;

	priv = g_new0(LoquiStyleEntryPrivate, 1);

	entry->priv = priv;
}
LoquiStyleEntry*
loqui_style_entry_new(void)
{
        LoquiStyleEntry *entry;
	LoquiStyleEntryPrivate *priv;

	entry = g_object_new(loqui_style_entry_get_type(), NULL);
	
        priv = entry->priv;

        return entry;
}

gboolean
loqui_style_entry_set_color_string(LoquiStyleEntry *style_entry, const gchar *color_string)
{
        g_return_val_if_fail(style_entry != NULL, FALSE);
        g_return_val_if_fail(LOQUI_IS_STYLE_ENTRY(style_entry), FALSE);

	G_FREE_UNLESS_NULL(style_entry->color_string);
	
	if (!color_string)
		return FALSE;

	if (!gdk_color_parse(color_string, &style_entry->color))
		return FALSE;;

	style_entry->color_string = g_strdup(color_string);
	return TRUE;
}
G_CONST_RETURN gchar *
loqui_style_entry_get_color_string(LoquiStyleEntry *style_entry)
{
        g_return_val_if_fail(style_entry != NULL, NULL);
        g_return_val_if_fail(LOQUI_IS_STYLE_ENTRY(style_entry), NULL);
	
	return style_entry->color_string;
}

void
loqui_style_entry_set_color(LoquiStyleEntry *style_entry, GdkColor color)
{
        g_return_if_fail(style_entry != NULL);
        g_return_if_fail(LOQUI_IS_STYLE_ENTRY(style_entry));
	
	memcpy(&style_entry->color, &color, sizeof(style_entry->color));
	G_FREE_UNLESS_NULL(style_entry->color_string);
	style_entry->color_string = loqui_style_entry_get_string_from_gdk_color(&color);
}
GdkColor
loqui_style_entry_get_color(LoquiStyleEntry *style_entry)
{
	return style_entry->color;
}

gboolean
loqui_style_entry_set_font_string(LoquiStyleEntry *style_entry, const gchar *font_string)
{
        g_return_val_if_fail(style_entry != NULL, FALSE);
        g_return_val_if_fail(LOQUI_IS_STYLE_ENTRY(style_entry), FALSE);

	if (!(style_entry->font_desc = pango_font_description_from_string(font_string)))
		return FALSE;
	return TRUE;
}

gchar *
loqui_style_entry_get_font_string(LoquiStyleEntry *style_entry)
{
	if (style_entry->font_desc)
		return pango_font_description_to_string(style_entry->font_desc);
	return NULL;
}

void
loqui_style_entry_set_font(LoquiStyleEntry *style_entry, PangoFontDescription *font_desc)
{
	style_entry->font_desc = font_desc;
}

PangoFontDescription *
loqui_style_entry_get_font(LoquiStyleEntry *style_entry)
{
	return style_entry->font_desc;
}

void
loqui_style_entry_set_string(LoquiStyleEntry *style_entry, const gchar *string)
{
	G_FREE_UNLESS_NULL(style_entry->string);
	style_entry->string = g_strdup(string);
}

G_CONST_RETURN gchar *
loqui_style_entry_get_string(LoquiStyleEntry *style_entry)
{
	return style_entry->string;
}


void
loqui_style_entry_set_integer(LoquiStyleEntry *style_entry, int i)
{
	style_entry->i = i;
}

int
loqui_style_entry_get_integer(LoquiStyleEntry *style_entry)
{
	return style_entry->i;
}


void
loqui_style_entry_set_double(LoquiStyleEntry *style_entry, double d)
{
	style_entry->d = d;
}

double
loqui_style_entry_get_double(LoquiStyleEntry *style_entry)
{
	return style_entry->d;
}

void
loqui_style_entry_set_is_color_default(LoquiStyleEntry *style_entry, gboolean is_color_default)
{
        g_return_if_fail(style_entry != NULL);
        g_return_if_fail(LOQUI_IS_STYLE_ENTRY(style_entry));

	style_entry->is_color_default = is_color_default;
}

gboolean
loqui_style_entry_get_is_color_default(LoquiStyleEntry *style_entry)
{
        g_return_val_if_fail(style_entry != NULL, FALSE);
        g_return_val_if_fail(LOQUI_IS_STYLE_ENTRY(style_entry), FALSE);

	return style_entry->is_color_default;
}

void
loqui_style_entry_set_is_font_default(LoquiStyleEntry *style_entry, gboolean is_font_default)
{
        g_return_if_fail(style_entry != NULL);
        g_return_if_fail(LOQUI_IS_STYLE_ENTRY(style_entry));

	style_entry->is_font_default = is_font_default;
}

gboolean
loqui_style_entry_get_is_font_default(LoquiStyleEntry *style_entry)
{
        g_return_val_if_fail(style_entry != NULL, FALSE);
        g_return_val_if_fail(LOQUI_IS_STYLE_ENTRY(style_entry), FALSE);

	return style_entry->is_font_default;
}



gchar *
loqui_style_entry_get_string_from_gdk_color(GdkColor *color)
{
	return g_strdup_printf("#%.2x%.2x%.2x",
			       color->red,
			       color->green,
			       color->blue);
}
