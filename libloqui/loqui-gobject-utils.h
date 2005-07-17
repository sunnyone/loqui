/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
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
#ifndef __GOBJECT_UTILS_H__
#define __GOBJECT_UTILS_H__

#include <libloqui/loqui-utils.h>
#include <glib.h>

#define LOQUI_DEFINE_READER_GENERIC(class_name_capitalized, class_name_lowercase, attr_name, return_type) \
return_type class_name_lowercase ## _get_ ## attr_name(class_name_capitalized *obj) \
{ \
 	g_return_val_if_fail(obj != NULL, (return_type) 0); \
        g_return_val_if_fail(G_TYPE_CHECK_INSTANCE_TYPE(obj, class_name_lowercase ## _get_type()), (return_type) 0); \
\
        return obj->attr_name; \
}
#define LOQUI_DEFINE_WRITER_GENERIC(class_name_capitalized, class_name_lowercase, attr_name, in_type) \
void class_name_lowercase ## _set_ ## attr_name(class_name_capitalized *obj, in_type foo) \
{ \
 	g_return_if_fail(obj != NULL); \
        g_return_if_fail(G_TYPE_CHECK_INSTANCE_TYPE(obj, class_name_lowercase ## _get_type())); \
\
        obj->attr_name = foo; \
        g_object_notify(G_OBJECT(obj), # attr_name); \
}
#define LOQUI_DEFINE_READER_GENERIC_PROTOTYPE(class_name_capitalized, class_name_lowercase, attr_name, type) \
  type class_name_lowercase ## _get_ ## attr_name(class_name_capitalized *obj)
#define LOQUI_DEFINE_WRITER_GENERIC_PROTOTYPE(class_name_capitalized, class_name_lowercase, attr_name, type) \
  void class_name_lowercase ## _set_ ## attr_name(class_name_capitalized *obj, type foo)

#define LOQUI_DEFINE_ACCESSOR_GENERIC(class_name_capitalized, class_name_lowercase, attr_name, type) \
  LOQUI_DEFINE_READER_GENERIC(class_name_capitalized, class_name_lowercase, attr_name, type); \
  LOQUI_DEFINE_WRITER_GENERIC(class_name_capitalized, class_name_lowercase, attr_name, type)
#define LOQUI_DEFINE_ACCESSOR_GENERIC_PROTOTYPE(class_name_capitalized, class_name_lowercase, attr_name, type) \
  LOQUI_DEFINE_READER_GENERIC_PROTOTYPE(class_name_capitalized, class_name_lowercase, attr_name, type); \
  LOQUI_DEFINE_WRITER_GENERIC_PROTOTYPE(class_name_capitalized, class_name_lowercase, attr_name, type)


#define LOQUI_DEFINE_WRITER_POINTER(class_name_capitalized, class_name_lowercase, attr_name, in_type, new_func, destroy_func) \
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

#define LOQUI_DEFINE_ACCESSOR_POINTER(class_name_capitalized, class_name_lowercase, attr_name, in_type, return_type, new_func, destroy_func) \
  LOQUI_DEFINE_READER_GENERIC(class_name_capitalized, class_name_lowercase, attr_name, return_type); \
  LOQUI_DEFINE_WRITER_POINTER(class_name_capitalized, class_name_lowercase, attr_name, const gchar *, new_func, destroy_func)

#define LOQUI_DEFINE_ACCESSOR_POINTER_PROTOTYPE(class_name_capitalized, class_name_lowercase, attr_name, in_type, return_type) \
  LOQUI_DEFINE_READER_GENERIC_PROTOTYPE(class_name_capitalized, class_name_lowercase, attr_name, return_type); \
  LOQUI_DEFINE_WRITER_GENERIC_PROTOTYPE(class_name_capitalized, class_name_lowercase, attr_name, in_type)
  
#define LOQUI_DEFINE_ACCESSOR_CONST_STRING(class_name_capitalized, class_name_lowercase, attr_name) \
  LOQUI_DEFINE_ACCESSOR_POINTER(class_name_capitalized, class_name_lowercase, attr_name, const gchar *, G_CONST_RETURN gchar *, g_strdup, g_free);

#define LOQUI_DEFINE_ACCESSOR_CONST_STRING_PROTOTYPE(class_name_capitalized, class_name_lowercase, attr_name) \
  LOQUI_DEFINE_ACCESSOR_POINTER_PROTOTYPE(class_name_capitalized, class_name_lowercase, attr_name, const gchar *, G_CONST_RETURN gchar *);

#define LOQUI_DEFINE_INTERFACE(TypeName, type_name) \
static void type_name ## _base_init(gpointer object_class); \
GType \
type_name ## _get_type(void) \
{ \
        static GType type = 0;	\
\
        if (type == 0) {										\
                static const GTypeInfo info = {								\
                        sizeof (TypeName ## Iface),						\
                        type_name ## _base_init,   /* base_init */				\
                        NULL,   /* base_finalize */							\
                        NULL,   /* class_init */							\
                        NULL,   /* class_finalize */							\
                        NULL,   /* class_data */							\
                        0,										\
                        0,      /* n_preallocs */							\
                        NULL    /* instance_init */							\
                };											\
                type = g_type_register_static(G_TYPE_INTERFACE, # TypeName, &info, 0);	\
        }												\
													\
        return type;											\
}

/* To use these signal macros, define type_name ## signals static array and add SIGNAL_ ## ENUM_NAME entries to it. */
#define LOQUI_DEFINE_SIGNAL_EMITTER_ARG0(TypeName, type_name, signal_name, ENUM_NAME) \
void \
type_name ## _ ## signal_name ( TypeName *self) \
{ \
        g_signal_emit(self, type_name ## _signals[SIGNAL_ ## ENUM_NAME], 0); \
}

#define LOQUI_DEFINE_SIGNAL_EMITTER_ARG1(TypeName, type_name, signal_name, ENUM_NAME, arg1Type) \
void \
type_name ## _ ## signal_name ( TypeName *self, arg1Type arg1) \
{ \
        g_signal_emit(self, type_name ## _signals[SIGNAL_ ## ENUM_NAME], 0, arg1); \
}


#define LOQUI_DEFINE_INTERFACE_METHOD_CALLER_ARG0(TypeName, type_name, method_name) \
void \
type_name ## _ ## method_name ( TypeName *self) \
{ \
        if (!G_TYPE_INSTANCE_GET_INTERFACE(self, type_name ## _get_type(), TypeName ## Iface)->method_name(self)) { \
                return; \
	} \
	G_TYPE_INSTANCE_GET_INTERFACE(self, type_name ## _get_type(), TypeName ## Iface)->method_name(self); \
}

#define LOQUI_DEFINE_INTERFACE_METHOD_CALLER_ARG1(TypeName, type_name, method_name, arg1Type) \
void \
type_name ## _ ## method_name ( TypeName *self, arg1Type arg1) \
{ \
        if (!G_TYPE_INSTANCE_GET_INTERFACE(self, type_name ## _get_type(), TypeName ## Iface)->method_name) { \
                return; \
	} \
	G_TYPE_INSTANCE_GET_INTERFACE(self, type_name ## _get_type(), TypeName ## Iface)->method_name(self, arg1); \
}

#define LOQUI_DEFINE_INTERFACE_METHOD_CALLER_ARG2(TypeName, type_name, method_name, arg1Type, arg2Type) \
void \
type_name ## _ ## method_name ( TypeName *self, arg1Type arg1, arg2Type arg2) \
{ \
        if (!G_TYPE_INSTANCE_GET_INTERFACE(self, type_name ## _get_type(), TypeName ## Iface)->method_name) { \
                return; \
	} \
	G_TYPE_INSTANCE_GET_INTERFACE(self, type_name ## _get_type(), TypeName ## Iface)->method_name(self, arg1, arg2); \
}


#define LOQUI_DEFINE_INTERFACE_METHOD_CALLER_ARG0_WITH_RETURN(TypeName, type_name, method_name, returnType) \
returnType \
type_name ## _ ## method_name ( TypeName *self) \
{ \
        if (!G_TYPE_INSTANCE_GET_INTERFACE(self, type_name ## _get_type(), TypeName ## Iface)->method_name) { \
                return (returnType) 0; \
	} \
        return G_TYPE_INSTANCE_GET_INTERFACE(self, type_name ## _get_type(), TypeName ## Iface)->method_name(self); \
}

#define LOQUI_DEFINE_INTERFACE_METHOD_CALLER_ARG1_WITH_RETURN(TypeName, type_name, method_name, returnType, arg1Type) \
returnType \
type_name ## _ ## method_name ( TypeName *self, arg1Type arg1) \
{ \
        if (!G_TYPE_INSTANCE_GET_INTERFACE(self, type_name ## _get_type(), TypeName ## Iface)->method_name) { \
                return (returnType) 0; \
	} \
        return G_TYPE_INSTANCE_GET_INTERFACE(self, type_name ## _get_type(), TypeName ## Iface)->method_name(self, arg1); \
}

#define LOQUI_DEFINE_INTERFACE_METHOD_CALLER_ARG2_WITH_RETURN(TypeName, type_name, method_name, returnType, arg1Type, arg2Type) \
returnType \
type_name ## _ ## method_name ( TypeName *self, arg1Type arg1, arg2Type arg2) \
{ \
        if (!G_TYPE_INSTANCE_GET_INTERFACE(self, type_name ## _get_type(), TypeName ## Iface)->method_name) { \
                return (returnType) 0; \
	} \
        return G_TYPE_INSTANCE_GET_INTERFACE(self, type_name ## _get_type(), TypeName ## Iface)->method_name(self, arg1, arg2); \
}

#endif /* __GOBJECT_UTILS_H__ */
