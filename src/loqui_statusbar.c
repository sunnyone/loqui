/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui -- IRC client for Gtk2 <http://loqui.good-day.net/>
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

#include "loqui_statusbar.h"

struct _LoquiStatusbarPrivate
{
	GtkWidget *label_user_number;
	GtkWidget *label_channel;
	GtkWidget *label_channel_mode;
	GtkWidget *label_account;
};

static GtkStatusbarClass *parent_class = NULL;
#define PARENT_TYPE GTK_TYPE_STATUSBAR

static void loqui_statusbar_class_init(LoquiStatusbarClass *klass);
static void loqui_statusbar_init(LoquiStatusbar *statusbar);
static void loqui_statusbar_finalize(GObject *object);
static void loqui_statusbar_destroy(GtkObject *object);

GType
loqui_statusbar_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(LoquiStatusbarClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) loqui_statusbar_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(LoquiStatusbar),
				0,              /* n_preallocs */
				(GInstanceInitFunc) loqui_statusbar_init
			};
		
		type = g_type_register_static(PARENT_TYPE,
					      "LoquiStatusbar",
					      &our_info,
					      0);
	}
	
	return type;
}
static void
loqui_statusbar_class_init (LoquiStatusbarClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);
        GtkObjectClass *gtk_object_class = GTK_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = loqui_statusbar_finalize;
        gtk_object_class->destroy = loqui_statusbar_destroy;
}
static void 
loqui_statusbar_init (LoquiStatusbar *statusbar)
{
	LoquiStatusbarPrivate *priv;

	priv = g_new0(LoquiStatusbarPrivate, 1);

	statusbar->priv = priv;
}
static void 
loqui_statusbar_finalize (GObject *object)
{
	LoquiStatusbar *statusbar;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_STATUSBAR(object));

        statusbar = LOQUI_STATUSBAR(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize) (object);

	g_free(statusbar->priv);
}
static void 
loqui_statusbar_destroy (GtkObject *object)
{
        LoquiStatusbar *statusbar;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_STATUSBAR(object));

        statusbar = LOQUI_STATUSBAR(object);

        if (GTK_OBJECT_CLASS(parent_class)->destroy)
                (* GTK_OBJECT_CLASS(parent_class)->destroy) (object);
}

GtkWidget*
loqui_statusbar_new (void)
{
        LoquiStatusbar *statusbar;
	LoquiStatusbarPrivate *priv;
	GtkWidget *hsep;

	statusbar = g_object_new(loqui_statusbar_get_type(), NULL);
	priv = statusbar->priv;
	
	gtk_statusbar_set_has_resize_grip(GTK_STATUSBAR(statusbar), FALSE);
	gtk_label_set_selectable(GTK_LABEL(GTK_STATUSBAR(statusbar)->label), TRUE);
	
	priv->label_channel = gtk_label_new("");
	gtk_box_pack_start(GTK_BOX(statusbar), priv->label_channel, FALSE, FALSE, 0);

	priv->label_channel_mode = gtk_label_new("");
	gtk_box_pack_start(GTK_BOX(statusbar), priv->label_channel_mode, FALSE, FALSE, 0);

	priv->label_user_number = gtk_label_new("");
	gtk_box_pack_start(GTK_BOX(statusbar), priv->label_user_number, FALSE, FALSE, 0);

	hsep = gtk_vseparator_new();
	gtk_box_pack_start(GTK_BOX(statusbar), hsep, FALSE, FALSE, 2);

	priv->label_account = gtk_label_new("");
	gtk_box_pack_start(GTK_BOX(statusbar), priv->label_account, FALSE, FALSE, 0);
	
	return GTK_WIDGET(statusbar);
}

void
loqui_statusbar_set_current_channel(LoquiStatusbar *statusbar, Channel *channel)
{
	LoquiStatusbarPrivate *priv;
	gchar *buf;
	gchar *channel_mode;
	guint user_number, op_number;
	
        g_return_if_fail(statusbar != NULL);
        g_return_if_fail(LOQUI_IS_STATUSBAR(statusbar));
    
    	priv = statusbar->priv;
        
	gtk_label_set(GTK_LABEL(priv->label_channel), "");
	gtk_label_set(GTK_LABEL(priv->label_channel_mode), "");
	gtk_label_set(GTK_LABEL(priv->label_user_number), "");
	
	if(channel) {
		channel_mode = channel_get_mode(channel);
		channel_get_user_number(channel, &user_number, &op_number);
		
		gtk_label_set(GTK_LABEL(priv->label_channel), channel_get_name(channel));
		
		buf = g_strdup_printf("[%s]", channel_mode);
		gtk_label_set(GTK_LABEL(priv->label_channel_mode), buf);
		g_free(buf);
		
		if(user_number > 0) {
			buf = g_strdup_printf("(%d/%d)", op_number, user_number);
			gtk_label_set(GTK_LABEL(priv->label_user_number), buf);
			g_free(buf);
		}
	}
}
void
loqui_statusbar_set_current_account(LoquiStatusbar *statusbar, Account *account)
{
        g_return_if_fail(statusbar != NULL);
        g_return_if_fail(LOQUI_IS_STATUSBAR(statusbar));
        
        gtk_label_set(GTK_LABEL(statusbar->priv->label_account), account ? account_get_name(account) : "");
}
void loqui_statusbar_set_default(LoquiStatusbar *statusbar, const gchar *str)
{
	guint context_id;

        g_return_if_fail(statusbar != NULL);
        g_return_if_fail(LOQUI_IS_STATUSBAR(statusbar));
        	
	context_id = gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), "Default");
	gtk_statusbar_pop(GTK_STATUSBAR(statusbar), context_id);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), context_id, str);
}
