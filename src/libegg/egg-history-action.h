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

#ifndef __EGG_HISTORY_ACTION_H__
#define __EGG_HISTORY_ACTION_H__

#include <gtk/gtk.h>
#include <egg-entry-action.h>

G_BEGIN_DECLS

#define EGG_TYPE_HISTORY_ACTION            (egg_history_action_get_type ())
#define EGG_HISTORY_ACTION(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_HISTORY_ACTION, EggHistoryAction))
#define EGG_HISTORY_ACTION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), EGG_TYPE_HISTORY_ACTION, EggHistoryActionClass))
#define EGG_IS_HISTORY_ACTION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_HISTORY_ACTION))
#define EGG_IS_HISTORY_ACTION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((obj), EGG_TYPE_HISTORY_ACTION))
#define EGG_HISTORY_ACTION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), EGG_TYPE_HISTORY_ACTION, EggHistoryActionClass))

typedef struct _EggHistoryAction      EggHistoryAction;
typedef struct _EggHistoryActionClass EggHistoryActionClass;

struct _EggHistoryAction
{
	EggEntryAction parent;

	guint max_history;

	GList *history;
};

struct _EggHistoryActionClass
{
	EggEntryActionClass parent_class;

	GtkCombo *(*get_combo_widget) (EggHistoryAction *action,
				       GtkWidget        *proxy);
	void      (*set_history)      (EggHistoryAction *action,
				       GList            *history);

	/* -- signals -- */
	void      (*history_changed)  (EggHistoryAction *action);
};

GType        egg_history_action_get_type (void);

void         egg_history_action_set_history      (EggHistoryAction *action,
						  GList            *history);
const GList *egg_history_action_get_history      (EggHistoryAction *action);
void         egg_history_action_set_max_history  (EggHistoryAction *action,
						  guint             max_history);
guint        egg_history_action_get_max_history  (EggHistoryAction *action);

/* protected */
GtkCombo    *egg_history_action_get_combo_widget (EggHistoryAction *action,
						  GtkWidget        *proxy);

G_END_DECLS

#endif /* __EGG_HISTORY_ACTION__ */
