/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui -- IRC client for GNOME2 <http://loqui.good-day.net/>
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
#include "about.h"

void
about_open(gpointer data)
{
	GtkWidget *about;

	const gchar *authors[] = { "Yoichi Imai", NULL };
	const gchar *docs[] = { NULL };

	about = gnome_about_new(_("Loqui"), VERSION,
				_("(C) 2002 Yoichi Imai"),
				_("IRC client for GNOME2\n"
				  "http://loqui.good-day.net/"),
				authors, docs,
				NULL, /* translators ... why it's not "char **"? */
				NULL);

	gtk_window_set_transient_for(GTK_WINDOW(about), GTK_WINDOW(data));
        gtk_window_set_destroy_with_parent(GTK_WINDOW(about), TRUE);
        g_signal_connect (G_OBJECT (about), "destroy",
			  G_CALLBACK (gtk_widget_destroyed), &about);
	
	gtk_widget_show (about);

}
