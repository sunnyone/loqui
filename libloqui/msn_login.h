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
#ifndef __MSN_LOGIN_H__
#define __MSN_LOGIN_H__

#include <gtk/gtk.h>
#include "loqui_account.h"
#include "msn_message.h"

G_BEGIN_DECLS

#define TYPE_MSN_LOGIN                 (msn_login_get_type ())
#define MSN_LOGIN(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_MSN_LOGIN, MSNLogin))
#define MSN_LOGIN_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_MSN_LOGIN, MSNLoginClass))
#define IS_MSN_LOGIN(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_MSN_LOGIN))
#define IS_MSN_LOGIN_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_MSN_LOGIN))
#define MSN_LOGIN_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_MSN_LOGIN, MSNLoginClass))

typedef struct _MSNLogin            MSNLogin;
typedef struct _MSNLoginClass       MSNLoginClass;

typedef struct _MSNLoginPrivate     MSNLoginPrivate;

struct _MSNLogin
{
        GObject parent;

        LoquiAccount *account; /* actually AccountMSN */

        MSNLoginPrivate *priv;
};

struct _MSNLoginClass
{
        GObjectClass parent_class;
};


GType msn_login_get_type(void) G_GNUC_CONST;

MSNLogin* msn_login_new(LoquiAccount *account);

void msn_login_connect(MSNLogin *login);
void msn_login_send_message(MSNLogin *login, MSNMessage *msg);

G_END_DECLS

#endif /* __MSN_LOGIN_H__ */
