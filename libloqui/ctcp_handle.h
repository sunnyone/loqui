/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * libloqui -- Chat/IM client library for GLib <http://launchpad.net/loqui/>
 * Copyright (C) 2002-2004 Yoichi Imai <sunnyone41@gmail.com>
 *
 * This Library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with the Gnome Library; see the file COPYING.LIB.  If not,
 * write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#ifndef __CTCP_HANDLE_H__
#define __CTCP_HANDLE_H__

#include <glib-object.h>
#include "loqui_account.h"
#include "loqui_receiver_irc.h"
#include "ctcp_message.h"

G_BEGIN_DECLS

#define TYPE_CTCP_HANDLE                 (ctcp_handle_get_type ())
#define CTCP_HANDLE(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_CTCP_HANDLE, CTCPHandle))
#define CTCP_HANDLE_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_CTCP_HANDLE, CTCPHandleClass))
#define IS_CTCP_HANDLE(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_CTCP_HANDLE))
#define IS_CTCP_HANDLE_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_CTCP_HANDLE))
#define CTCP_HANDLE_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_CTCP_HANDLE, CTCPHandleClass))

typedef struct _CTCPHandle            CTCPHandle;
typedef struct _CTCPHandleClass       CTCPHandleClass;

typedef struct _CTCPHandlePrivate     CTCPHandlePrivate;

struct _CTCPHandle
{
        GObject parent;
        
        CTCPHandlePrivate *priv;
};

struct _CTCPHandleClass
{
        GObjectClass parent_class;
};


GType ctcp_handle_get_type (void) G_GNUC_CONST;

CTCPHandle* ctcp_handle_new(LoquiReceiverIRC *receiver, LoquiAccount *account);
void ctcp_handle_message(CTCPHandle *ctcp_handle, CTCPMessage *ctcp_msg, gboolean is_request);

G_END_DECLS

#endif /* __CTCP_HANDLE_H__ */
