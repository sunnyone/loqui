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
#ifndef __LOQUI_CHANNEL_ENTRY_ACTION_H__
#define __LOQUI_CHANNEL_ENTRY_ACTION_H__

#include <gtk/gtk.h>
#include <gtk24backports.h>

G_BEGIN_DECLS

#define LOQUI_TYPE_CHANNEL_ENTRY_ACTION                 (loqui_channel_entry_action_get_type ())
#define LOQUI_CHANNEL_ENTRY_ACTION(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), LOQUI_TYPE_CHANNEL_ENTRY_ACTION, LoquiChannelEntryAction))
#define LOQUI_CHANNEL_ENTRY_ACTION_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), LOQUI_TYPE_CHANNEL_ENTRY_ACTION, LoquiChannelEntryActionClass))
#define LOQUI_IS_CHANNEL_ENTRY_ACTION(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LOQUI_TYPE_CHANNEL_ENTRY_ACTION))
#define LOQUI_IS_CHANNEL_ENTRY_ACTION_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), LOQUI_TYPE_CHANNEL_ENTRY_ACTION))
#define LOQUI_CHANNEL_ENTRY_ACTION_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), LOQUI_TYPE_CHANNEL_ENTRY_ACTION, LoquiChannelEntryActionClass))

typedef struct _LoquiChannelEntryAction            LoquiChannelEntryAction;
typedef struct _LoquiChannelEntryActionClass       LoquiChannelEntryActionClass;

typedef struct _LoquiChannelEntryActionPrivate     LoquiChannelEntryActionPrivate;

struct _LoquiChannelEntryAction
{
        GtkAction parent;
        
	GObject *channel_entry; /* FIXME */

        LoquiChannelEntryActionPrivate *priv;
};

struct _LoquiChannelEntryActionClass
{
        GtkActionClass parent_class;
};


GType loqui_channel_entry_action_get_type(void) G_GNUC_CONST;

LoquiChannelEntryAction* loqui_channel_entry_action_new(void);

void loqui_channel_entry_action_set_channel_entry(LoquiChannelEntryAction *action, GObject *channel_entry);
GObject *loqui_channel_entry_action_get_channel_entry(LoquiChannelEntryAction *action);  /* FIXME */ 

G_END_DECLS

#endif /* __LOQUI_CHANNEL_ENTRY_ACTION_H__ */
