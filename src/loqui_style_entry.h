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
#ifndef __LOQUI_STYLE_ENTRY_H__
#define __LOQUI_STYLE_ENTRY_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define LOQUI_TYPE_STYLE_ENTRY                 (loqui_style_entry_get_type ())
#define LOQUI_STYLE_ENTRY(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), LOQUI_TYPE_STYLE_ENTRY, LoquiStyleEntry))
#define LOQUI_STYLE_ENTRY_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), LOQUI_TYPE_STYLE_ENTRY, LoquiStyleEntryClass))
#define LOQUI_IS_STYLE_ENTRY(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LOQUI_TYPE_STYLE_ENTRY))
#define LOQUI_IS_STYLE_ENTRY_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), LOQUI_TYPE_STYLE_ENTRY))
#define LOQUI_STYLE_ENTRY_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), LOQUI_TYPE_STYLE_ENTRY, LoquiStyleEntryClass))

typedef struct _LoquiStyleEntry            LoquiStyleEntry;
typedef struct _LoquiStyleEntryClass       LoquiStyleEntryClass;

typedef struct _LoquiStyleEntryPrivate     LoquiStyleEntryPrivate;

struct _LoquiStyleEntry
{
        GObject parent;

	gboolean is_color_default;
	/* a value is set when the other value is set. */
	gchar *color_string;
        GdkColor color;

	gboolean is_font_default;
	PangoFontDescription *font_desc;
	
	/* various purpose */
	gchar *string;
	int i;
	double d;

        LoquiStyleEntryPrivate *priv;
};

struct _LoquiStyleEntryClass
{
        GObjectClass parent_class;
};


GType loqui_style_entry_get_type(void) G_GNUC_CONST;

LoquiStyleEntry* loqui_style_entry_new(void);

gboolean loqui_style_entry_set_color_string(LoquiStyleEntry *style_entry, const gchar *color_string);
G_CONST_RETURN gchar *loqui_style_entry_get_color_string(LoquiStyleEntry *style_entry);

void loqui_style_entry_set_color(LoquiStyleEntry *style_entry, GdkColor color);
GdkColor loqui_style_entry_get_color(LoquiStyleEntry *style_entry);

gboolean loqui_style_entry_set_font_string(LoquiStyleEntry *style_entry, const gchar *font_string);
/* return value must be freed! */
gchar *loqui_style_entry_get_font_string(LoquiStyleEntry *style_entry);

void loqui_style_entry_set_font(LoquiStyleEntry *style_entry, PangoFontDescription *font_desc);
PangoFontDescription *loqui_style_entry_get_font(LoquiStyleEntry *style_entry);

void loqui_style_entry_set_string(LoquiStyleEntry *style_entry, const gchar *string);
G_CONST_RETURN gchar *loqui_style_entry_get_string(LoquiStyleEntry *style_entry);

void loqui_style_entry_set_integer(LoquiStyleEntry *style_entry, int i);
int loqui_style_entry_get_integer(LoquiStyleEntry *style_entry);

void loqui_style_entry_set_double(LoquiStyleEntry *style_entry, double d);
double loqui_style_entry_get_double(LoquiStyleEntry *style_entry);

void loqui_style_entry_set_is_color_default(LoquiStyleEntry *style_entry, gboolean is_color_default);
gboolean loqui_style_entry_get_is_color_default(LoquiStyleEntry *style_entry);

void loqui_style_entry_set_is_font_default(LoquiStyleEntry *style_entry, gboolean is_font_default);
gboolean loqui_style_entry_get_is_font_default(LoquiStyleEntry *style_entry);

gchar *loqui_style_entry_get_string_from_gdk_color(GdkColor *color);

G_END_DECLS

#endif /* __LOQUI_STYLE_ENTRY_H__ */
