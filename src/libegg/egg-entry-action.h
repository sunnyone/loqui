/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/*
 *  Copyright (C) 2003 Takuro Ashie
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef EGG_ENTRYACTION_H
#define EGG_ENTRYACTION_H

#include <gtk/gtk.h>
#include <egg-action.h>

G_BEGIN_DECLS

#define EGG_TYPE_ENTRY_ACTION            (egg_entry_action_get_type ())
#define EGG_ENTRY_ACTION(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_ENTRY_ACTION, EggEntryAction))
#define EGG_ENTRY_ACTION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), EGG_TYPE_ENTRY_ACTION, EggEntryActionClass))
#define EGG_IS_ENTRY_ACTION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_ENTRY_ACTION))
#define EGG_IS_ENTRY_ACTION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((obj), EGG_TYPE_ENTRY_ACTION))
#define EGG_ENTRY_ACTION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), EGG_TYPE_ENTRY_ACTION, EggEntryActionClass))

typedef struct _EggEntryAction      EggEntryAction;
typedef struct _EggEntryActionClass EggEntryActionClass;

struct _EggEntryAction
{
	EggAction parent;

	gchar *text;
};

struct _EggEntryActionClass
{
	EggActionClass parent_class;

	GtkEntry *(*get_entry_widget) (EggEntryAction *action,
				       GtkWidget *proxy);
};

GType        egg_entry_action_get_type (void);

void         egg_entry_action_set_text (EggEntryAction *action,
					const gchar *text);
const gchar *egg_entry_action_get_text (EggEntryAction *action);

/* protected */
GtkEntry    *egg_entry_action_get_entry_widget (EggEntryAction *action,
						GtkWidget *proxy);

G_END_DECLS

#endif /* EGG_ENTRYACTION */
