/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui -- IRC client for Gtk2 <http://loqui.good-day.net/>
 * Copyright (C) 2003 Yoichi Imai <yoichi@silver-forest.com>
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
#ifndef __GOBJECT_UTILS_H__
#define __GOBJECT_UTILS_H__

#include "utils.h"
#include <glib.h>

#define ATTR_READER_GENERIC(return_type, failed_value, class_name_capitalized, class_name_lowercase, attr_name) \
return_type class_name_lowercase ## _get_ ## attr_name(class_name_capitalized *obj) \
{ \
 	g_return_val_if_fail(obj != NULL, failed_value); \
        g_return_val_if_fail(G_TYPE_CHECK_INSTANCE_TYPE(obj, class_name_lowercase ## _get_type()), failed_value); \
\
        return obj->attr_name; \
}
#define ATTR_WRITER_GENERIC(in_type, class_name_capitalized, class_name_lowercase, attr_name) \
void class_name_lowercase ## _set_ ## attr_name(class_name_capitalized *obj, in_type foo) \
{ \
 	g_return_if_fail(obj != NULL); \
        g_return_if_fail(G_TYPE_CHECK_INSTANCE_TYPE(obj, class_name_lowercase ## _get_type())); \
\
        obj->attr_name = foo; \
        g_object_notify(G_OBJECT(obj), # attr_name); \
}
#define ATTR_READER_GENERIC_PROTOTYPE(type, class_name_capitalized, class_name_lowercase, attr_name) \
  type class_name_lowercase ## _get_ ## attr_name(class_name_capitalized *obj)
#define ATTR_WRITER_GENERIC_PROTOTYPE(type, class_name_capitalized, class_name_lowercase, attr_name) \
  void class_name_lowercase ## _set_ ## attr_name(class_name_capitalized *obj, type foo)

#define ATTR_ACCESSOR_GENERIC(type, failed_value, class_name_capitalized, class_name_lowercase, attr_name) \
  ATTR_READER_GENERIC(type, failed_value, class_name_capitalized, class_name_lowercase, attr_name); \
  ATTR_WRITER_GENERIC(type, class_name_capitalized, class_name_lowercase, attr_name)
#define ATTR_ACCESSOR_GENERIC_PROTOTYPE(type, class_name_capitalized, class_name_lowercase, attr_name) \
  ATTR_READER_GENERIC_PROTOTYPE(type, class_name_capitalized, class_name_lowercase, attr_name); \
  ATTR_WRITER_GENERIC_PROTOTYPE(type, class_name_capitalized, class_name_lowercase, attr_name)


#define ATTR_WRITER_POINTER(new_func, destroy_func, in_type, class_name_capitalized, class_name_lowercase, attr_name) \
void class_name_lowercase ## _set_ ## attr_name (class_name_capitalized *obj, in_type foo) \
{ \
 	g_return_if_fail(obj != NULL); \
        g_return_if_fail(G_TYPE_CHECK_INSTANCE_TYPE(obj, class_name_lowercase ## _get_type())); \
\
	if (obj->attr_name) { \
		destroy_func(obj->attr_name); \
		obj->attr_name = NULL; \
	} \
	if (foo) \
		obj->attr_name = new_func(foo); \
        g_object_notify(G_OBJECT(obj), # attr_name); \
}

#define ATTR_ACCESSOR_POINTER(new_func, destroy_func, in_type, return_type, class_name_capitalized, class_name_lowercase, attr_name) \
  ATTR_READER_GENERIC(return_type, NULL, class_name_capitalized, class_name_lowercase, attr_name); \
  ATTR_WRITER_POINTER(new_func, destroy_func, const gchar *, class_name_capitalized, class_name_lowercase, attr_name)

#define ATTR_ACCESSOR_POINTER_PROTOTYPE(in_type, return_type, class_name_capitalized, class_name_lowercase, attr_name) \
  ATTR_READER_GENERIC_PROTOTYPE(return_type, class_name_capitalized, class_name_lowercase, attr_name); \
  ATTR_WRITER_GENERIC_PROTOTYPE(in_type, class_name_capitalized, class_name_lowercase, attr_name)
  
#endif /* __GOBJECT_UTILS_H__ */
