/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * libloqui -- Chat/IM client library for GLib
 * Copyright (C) 2004 Yoichi Imai <sunnyone41@gmail.com>
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
#ifndef __LOQUI_ACCOUNT_MANAGER_ITER_H__
#define __LOQUI_ACCOUNT_MANAGER_ITER_H__

#include <glib.h>
#include "loqui-account-manager.h"

typedef struct {
	GList *cur_ac;
	GList *account_list;

	GList *cur_ch;
	GList *channel_list;
} LoquiAccountManagerIter;

void loqui_account_manager_iter_init(LoquiAccountManager *manager, LoquiAccountManagerIter *iter);

gboolean loqui_account_manager_iter_equal(LoquiAccountManagerIter *a, LoquiAccountManagerIter *b);

LoquiAccountManagerIter* loqui_account_manager_iter_new(LoquiAccountManager *manager);
void loqui_account_manager_iter_free(LoquiAccountManagerIter *iter);

void loqui_account_manager_iter_set_first_channel_entry(LoquiAccountManagerIter *iter);
void loqui_account_manager_iter_set_last_channel_entry(LoquiAccountManagerIter *iter);

gboolean loqui_account_manager_iter_set_channel_entry(LoquiAccountManagerIter *iter, LoquiChannelEntry *chent);

LoquiChannelEntry * loqui_account_manager_iter_get_current_channel_entry(LoquiAccountManagerIter *iter);

/* this method doesn't not return next in reality. return current and go next/prev */
LoquiChannelEntry *loqui_account_manager_iter_channel_entry_next(LoquiAccountManagerIter *iter);
LoquiChannelEntry *loqui_account_manager_iter_channel_entry_previous(LoquiAccountManagerIter *iter);

#endif /* __LOQUI_ACCOUNT_MANAGER_ITER_H__ */
