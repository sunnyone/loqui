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
#ifndef __LOQUI_PROFILE_HANDLE_H__
#define __LOQUI_PROFILE_HANDLE_H__

#include <glib-object.h>
#include "loqui_profile_account.h"

G_BEGIN_DECLS

#define LOQUI_TYPE_PROFILE_HANDLE                 (loqui_profile_handle_get_type ())
#define LOQUI_PROFILE_HANDLE(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), LOQUI_TYPE_PROFILE_HANDLE, LoquiProfileHandle))
#define LOQUI_PROFILE_HANDLE_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), LOQUI_TYPE_PROFILE_HANDLE, LoquiProfileHandleClass))
#define LOQUI_IS_PROFILE_HANDLE(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LOQUI_TYPE_PROFILE_HANDLE))
#define LOQUI_IS_PROFILE_HANDLE_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), LOQUI_TYPE_PROFILE_HANDLE))
#define LOQUI_PROFILE_HANDLE_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), LOQUI_TYPE_PROFILE_HANDLE, LoquiProfileHandleClass))

typedef struct _LoquiProfileHandle            LoquiProfileHandle;
typedef struct _LoquiProfileHandleClass       LoquiProfileHandleClass;

typedef struct _LoquiProfileHandlePrivate     LoquiProfileHandlePrivate;

struct _LoquiProfileHandle
{
        GObject parent;
        
        LoquiProfileHandlePrivate *priv;
};

struct _LoquiProfileHandleClass
{
        GObjectClass parent_class;
};


GType loqui_profile_handle_get_type(void) G_GNUC_CONST;

LoquiProfileHandle* loqui_profile_handle_new(void);

gboolean loqui_profile_handle_read_from_buffer(LoquiProfileHandle *handle, GList **profile_list, const gchar *buf);
gboolean loqui_profile_handle_read_from_file(LoquiProfileHandle *handle, GList **profile_list, const gchar *path);

gboolean loqui_profile_handle_write_to_buffer(LoquiProfileHandle *handle, GList *profile_list, gchar **buf);
gboolean loqui_profile_handle_write_to_file(LoquiProfileHandle *handle, GList *profile_list, const gchar *path);

/* name must be static gchararray */
void loqui_profile_handle_register_type(LoquiProfileHandle *handle, const gchar *name, GType type);
void loqui_profile_handle_register_child_type(LoquiProfileHandle *handle, const gchar* name, GType type);

G_END_DECLS

#endif /* __LOQUI_PROFILE_HANDLE_H__ */
