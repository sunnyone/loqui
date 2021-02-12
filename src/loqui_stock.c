/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui -- IRC client for Gtk
 * Copyright (C) 2004 Yoichi Imai <sunnyone41@gmail.com>
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
#include "loqui_stock.h"

#include <gtk/gtk.h>
#include "gtkutils.h"

#include "icons/pixbufs.h"

struct icon_pair {
	const gchar *id;
	const guint8 *inline_pixbuf;
};
static struct icon_pair icon_list[] = { { LOQUI_STOCK_CONSOLE, console_pixbuf },
					{ LOQUI_STOCK_ONLINE,  online_pixbuf },
					{ LOQUI_STOCK_OFFLINE, offline_pixbuf },
					{ LOQUI_STOCK_AWAY,    away_pixbuf },
					{ LOQUI_STOCK_BUSY,    busy_pixbuf },
					{ LOQUI_STOCK_COMMAND, command_pixbuf },
					{ LOQUI_STOCK_OPERATOR,      naruto_pixbuf },
					{ LOQUI_STOCK_SPEAK_ABILITY, speaker_pixbuf },
					{ LOQUI_STOCK_WHETHER_SCROLL, whether_scroll_pixbuf },
					{ LOQUI_STOCK_NOTICE,  notice_pixbuf },
					{ LOQUI_STOCK_LOQUI, loqui_pixbuf },
					{ LOQUI_STOCK_LOQUI_HILIGHTED, loqui_hilighted_pixbuf },
					{ NULL, NULL } };

int LOQUI_ICON_SIZE_FONT = GTK_ICON_SIZE_INVALID;

static void make_icons(void);

static void
make_icons(void)
{
	GdkPixbuf *pixbuf;
	GtkIconSet *icon_set;
	GtkIconFactory *icon_factory;
	int i, height;
	GtkWidget *widget;
	
	/* dummy widget */
	widget = gtk_label_new("");
	gtkutils_get_current_font_pixel_size(widget, NULL, &height);
	gtk_widget_destroy(widget);

	LOQUI_ICON_SIZE_FONT = gtk_icon_size_register("loqui-font", height, height);

	icon_factory = gtk_icon_factory_new();
	gtk_icon_factory_add_default(icon_factory);

	for(i = 0; icon_list[i].id != NULL; i++) {
		pixbuf = gdk_pixbuf_new_from_inline(-1, icon_list[i].inline_pixbuf, FALSE, NULL);
		if(pixbuf == NULL)
			continue;
		icon_set = gtk_icon_set_new_from_pixbuf(pixbuf);
		g_object_unref(pixbuf);
		
		gtk_icon_factory_add(icon_factory, icon_list[i].id, icon_set);
	}
	g_object_unref(icon_factory);
}

void
loqui_stock_init(void)
{
	make_icons();
}

G_CONST_RETURN gchar*
loqui_stock_get_id_from_basic_away_type(LoquiBasicAwayType basic_away)
{
	switch (basic_away) {
	case LOQUI_BASIC_AWAY_TYPE_AWAY:
		return LOQUI_STOCK_AWAY;
	case LOQUI_BASIC_AWAY_TYPE_BUSY:
		return LOQUI_STOCK_BUSY;
	case LOQUI_BASIC_AWAY_TYPE_ONLINE:
		return LOQUI_STOCK_ONLINE;
	case LOQUI_BASIC_AWAY_TYPE_OFFLINE:
		return LOQUI_STOCK_OFFLINE;
	default:
		break;
	}
	return NULL;
}
