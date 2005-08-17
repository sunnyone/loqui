/* -*- mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * libloqui -- Chat/IM client library for GLib <http://loqui.good-day.net/>
 * Copyright (C) 2002-2004 Yoichi Imai <yoichi@silver-forest.com>
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
#ifndef __LOQUI_ACCOUNT_MANAGER_H__
#define __LOQUI_ACCOUNT_MANAGER_H__

#include "loqui_account.h"
#include "loqui_channel.h"
#include "loqui_protocol_manager.h"

G_BEGIN_DECLS

#define LOQUI_TYPE_ACCOUNT_MANAGER                 (loqui_account_manager_get_type ())
#define LOQUI_ACCOUNT_MANAGER(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), LOQUI_TYPE_ACCOUNT_MANAGER, LoquiAccountManager))
#define LOQUI_ACCOUNT_MANAGER_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), LOQUI_TYPE_ACCOUNT_MANAGER, LoquiAccountManagerClass))
#define LOQUI_IS_ACCOUNT_MANAGER(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LOQUI_TYPE_ACCOUNT_MANAGER))
#define LOQUI_IS_ACCOUNT_MANAGER_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), LOQUI_TYPE_ACCOUNT_MANAGER))
#define LOQUI_ACCOUNT_MANAGER_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), LOQUI_TYPE_ACCOUNT_MANAGER, LoquiAccountManagerClass))

typedef struct _LoquiAccountManager            LoquiAccountManager;
typedef struct _LoquiAccountManagerClass       LoquiAccountManagerClass;

typedef struct _LoquiAccountManagerPrivate     LoquiAccountManagerPrivate;

struct _LoquiAccountManager
{
        GObject parent;

	LoquiProtocolManager *protocol_manager;

        gint max_channel_entry_id;

        LoquiAccountManagerPrivate *priv;
};

struct _LoquiAccountManagerClass
{
        GObjectClass parent_class;

	void (* add_account) (LoquiAccountManager *manager, LoquiAccount *account);
	void (* remove_account) (LoquiAccountManager *manager, LoquiAccount *account);
};

GType loqui_account_manager_get_type(void) G_GNUC_CONST;

LoquiAccountManager* loqui_account_manager_new(LoquiProtocolManager *manager);

void loqui_account_manager_load_accounts(LoquiAccountManager *account_manager);
void loqui_account_manager_save_accounts(LoquiAccountManager *account_manager);

void loqui_account_manager_add_account(LoquiAccountManager *manager, LoquiAccount *account);
void loqui_account_manager_remove_account(LoquiAccountManager *manager, LoquiAccount *account);
void loqui_account_manager_remove_all_account(LoquiAccountManager *manager);

void loqui_account_manager_connect_all_default(LoquiAccountManager *manager);
void loqui_account_manager_disconnect_all(LoquiAccountManager *manager);

GList *loqui_account_manager_get_account_list(LoquiAccountManager *manager);

void loqui_account_manager_update_positions(LoquiAccountManager *manager);

gint loqui_account_manager_new_channel_entry_id(LoquiAccountManager *manager);

LoquiChannelEntry * loqui_account_manager_get_next_channel_entry(LoquiAccountManager *manager,
							   LoquiChannelEntry *chent,
							   gboolean require_updated);

LoquiChannelEntry * loqui_account_manager_get_previous_channel_entry(LoquiAccountManager *manager,
							       LoquiChannelEntry *chent,
							       gboolean require_updated);

G_END_DECLS

#endif /* __LOQUI_ACCOUNT_MANAGER_H__ */
