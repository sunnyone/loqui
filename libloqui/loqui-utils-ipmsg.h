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

#ifndef __LOQUI_UTILS_IPMSG_H__
#define __LOQUI_UTILS_IPMSG_H__

#include <glib.h>

gchar *loqui_utils_ipmsg_create_identifier(const gchar *ip_addr, gint port);

#endif /* __LOQUI_UTILS_IPMSG_H__ */
