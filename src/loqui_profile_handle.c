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
#include "config.h"

#include "loqui_profile_handle.h"
#include "intl.h"

#include <string.h>

enum {
        LAST_SIGNAL
};

enum {
        LAST_PROP
};

typedef enum {
	ELEMENT_NONE,
	ELEMENT_ACCOUNTS,
	ELEMENT_ACCOUNT,
	ELEMENT_PARAM,
	ELEMENT_LIST,
	ELEMENT_ITEM,
	ELEMENT_CHILD
} ElementType;

struct _LoquiProfileHandlePrivate
{
	GMarkupParseContext *context;
	
	GQueue *element_queue;
	GQueue *object_queue;
	GQueue *pspec_queue;
	GQueue *value_array_queue;
	
	GList *profile_list;
	
	GHashTable *type_table;
	GHashTable *children_table;

	GHashTable *type_table_reverse;
	GHashTable *children_table_reverse;
};

#define GET_CURRENT_ELEMENT(handle) GPOINTER_TO_INT(g_queue_peek_tail(handle->priv->element_queue))

static GObjectClass *parent_class = NULL;

/* static guint loqui_profile_handle_signals[LAST_SIGNAL] = { 0 }; */

static void loqui_profile_handle_class_init(LoquiProfileHandleClass *klass);
static void loqui_profile_handle_init(LoquiProfileHandle *handle);
static void loqui_profile_handle_finalize(GObject *object);
static void loqui_profile_handle_dispose(GObject *object);

static void loqui_profile_handle_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec);
static void loqui_profile_handle_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec);

static G_CONST_RETURN gchar * loqui_profile_handle_parse_attribute(const gchar *key, const gchar **attribute_names, const gchar **attribute_values);
static void loqui_profile_handle_clear(LoquiProfileHandle *handle);
static gchar *loqui_profile_handle_object_to_xml(LoquiProfileHandle *handle, gboolean is_child, GObject *object);
static gchar *loqui_profile_handle_gvalue_to_xml(LoquiProfileHandle *handle, GParamSpec *pspec, GValue *value);

static void start_element_handler(GMarkupParseContext *context,
                	          const gchar         *element_name,
                   	          const gchar        **attribute_names,
                    	     	  const gchar        **attribute_values,
                        	  gpointer             user_data,
                        	  GError             **error);
static void end_element_handler(GMarkupParseContext *context,
                        	const gchar         *element_name,
                        	gpointer             user_data,
                        	GError             **error);
static void text_handler(GMarkupParseContext *context,
                         const gchar         *text,
                         gsize                text_len,
                         gpointer             user_data,
                         GError             **error);
static void error_handler(GMarkupParseContext *context,
                          GError              *error,
                          gpointer             user_data);
static gboolean parse_text(GParamSpec *pspec, const gchar *text, GValue *value);

static GMarkupParser parser = {
	start_element_handler,
	end_element_handler,
	text_handler,
	NULL,
	error_handler
};

GType
loqui_profile_handle_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(LoquiProfileHandleClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) loqui_profile_handle_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(LoquiProfileHandle),
				0,              /* n_preallocs */
				(GInstanceInitFunc) loqui_profile_handle_init
			};
		
		type = g_type_register_static(G_TYPE_OBJECT,
					      "LoquiProfileHandle",
					      &our_info,
					      0);
	}
	
	return type;
}
static void 
loqui_profile_handle_finalize(GObject *object)
{
	LoquiProfileHandle *handle;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_PROFILE_HANDLE(object));

        handle = LOQUI_PROFILE_HANDLE(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize)(object);

	g_free(handle->priv);
}
static void 
loqui_profile_handle_dispose(GObject *object)
{
	LoquiProfileHandle *handle;
	LoquiProfileHandlePrivate *priv;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_PROFILE_HANDLE(object));

        handle = LOQUI_PROFILE_HANDLE(object);
	priv = handle->priv;

	loqui_profile_handle_clear(handle);
	if (priv->type_table) {
		g_hash_table_destroy(priv->type_table);
		priv->type_table = NULL;
	}
	if (priv->children_table) {
		g_hash_table_destroy(priv->children_table);
		priv->children_table = NULL;
	}
	if (priv->type_table_reverse) {
		g_hash_table_destroy(priv->type_table_reverse);
		priv->type_table_reverse = NULL;
	}
	if (priv->children_table_reverse) {
		g_hash_table_destroy(priv->children_table_reverse);
		priv->children_table_reverse = NULL;
	}

        if (G_OBJECT_CLASS(parent_class)->dispose)
                (* G_OBJECT_CLASS(parent_class)->dispose)(object);
}
static void
loqui_profile_handle_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec)
{
        LoquiProfileHandle *handle;        

        handle = LOQUI_PROFILE_HANDLE(object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}
static void
loqui_profile_handle_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec)
{
        LoquiProfileHandle *handle;        

        handle = LOQUI_PROFILE_HANDLE(object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}

static void
loqui_profile_handle_class_init(LoquiProfileHandleClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = loqui_profile_handle_finalize;
        object_class->dispose = loqui_profile_handle_dispose;
        object_class->get_property = loqui_profile_handle_get_property;
        object_class->set_property = loqui_profile_handle_set_property;

}
static void 
loqui_profile_handle_init(LoquiProfileHandle *handle)
{
	LoquiProfileHandlePrivate *priv;

	priv = g_new0(LoquiProfileHandlePrivate, 1);

	handle->priv = priv;
}

static void
start_element_handler(GMarkupParseContext *context,
                      const gchar         *element_name,
                      const gchar        **attribute_names,
                      const gchar        **attribute_values,
                      gpointer             user_data,
                      GError             **error)
{
	LoquiProfileHandle *handle;
	LoquiProfileHandlePrivate *priv;
	GObject *obj;
	GParamSpec *pspec;
	GType child_type;
	ElementType elem;
	
        g_return_if_fail(user_data != NULL);
        g_return_if_fail(LOQUI_IS_PROFILE_HANDLE(user_data));

        handle = LOQUI_PROFILE_HANDLE(user_data);

	priv = handle->priv;
	{
	    	GList *cur;
	    	debug_print("Profile element: ");
	    	for(cur = priv->element_queue->head; cur != NULL; cur = cur->next) {
	    		debug_print("%d,", GPOINTER_TO_INT(cur->data));
	    	}
	    	debug_puts("%s", element_name);
	}
	
	elem = GET_CURRENT_ELEMENT(handle);
	if (elem == ELEMENT_NONE) {
		if(strcmp(element_name, "accounts") == 0) {
			g_queue_push_tail(priv->element_queue, GINT_TO_POINTER(ELEMENT_ACCOUNTS));
		} else {
      			g_set_error(error,
                   		    G_MARKUP_ERROR,
                   		    G_MARKUP_ERROR_UNKNOWN_ELEMENT,
                   		    _("Invalid element '%s': should be 'accounts'"), element_name);
		}
		return;
	}
	
	if (elem == ELEMENT_ACCOUNTS) {
		const gchar *type_id;
		GType type;
		
		if (strcmp(element_name, "account") == 0) {
			type_id = loqui_profile_handle_parse_attribute("type", attribute_names, attribute_values);
			if (!type_id) {
      				g_set_error(error,
                   			    G_MARKUP_ERROR,
                   			    G_MARKUP_ERROR_INVALID_CONTENT,
                   			    _("Invalid content: account has no type'"));
                   		return;
			}
			type = (GType) g_hash_table_lookup(priv->type_table, type_id);
			if (!type) {
      				g_set_error(error,
                   			    G_MARKUP_ERROR,
                   			    G_MARKUP_ERROR_INVALID_CONTENT,
                   			    _("Invalid content: type '%s' not found'"), type_id);
                   		return;
			}
			g_queue_push_tail(priv->element_queue, GINT_TO_POINTER(ELEMENT_ACCOUNT));
			g_queue_push_tail(priv->object_queue, g_object_new(type, NULL));
		} else {
      			g_set_error(error,
                   		    G_MARKUP_ERROR,
                   		    G_MARKUP_ERROR_UNKNOWN_ELEMENT,
                   		    _("Invalid element '%s': should be 'account'"), element_name);
                   	return;
		}
		return;
	}
	
	if ((obj = g_queue_peek_tail(priv->object_queue)) == NULL) {
      		g_set_error(error,
                	    G_MARKUP_ERROR,
                   	    G_MARKUP_ERROR_UNKNOWN_ELEMENT,
                   	    _("Invalid element: why is not in account?"));
                return;
	}
	
	if (elem == ELEMENT_ACCOUNT && strcmp(element_name, "param") == 0) {
		const gchar *key;
		key = loqui_profile_handle_parse_attribute("key", attribute_names, attribute_values);
		if (!key) {
			g_set_error(error,
           			    G_MARKUP_ERROR,
           			    G_MARKUP_ERROR_INVALID_CONTENT,
           			    _("Invalid param: doesn't have a key'"));
           		return;
		}
		if ((pspec = g_object_class_find_property(G_OBJECT_GET_CLASS(obj), key)) == NULL) {
			g_set_error(error,
           			    G_MARKUP_ERROR,
           			    G_MARKUP_ERROR_INVALID_CONTENT,
           			    _("Invalid param key"));
           		return;;
		}
		g_queue_push_tail(priv->pspec_queue, pspec);
		g_queue_push_tail(priv->element_queue, GINT_TO_POINTER(ELEMENT_PARAM));
		return;
	}
	if (elem == ELEMENT_PARAM && strcmp(element_name, "list") == 0) {
		g_queue_push_tail(priv->value_array_queue, g_value_array_new(0));
		g_queue_push_tail(priv->element_queue, GINT_TO_POINTER(ELEMENT_LIST));
		return;
	}
	if (elem == ELEMENT_LIST && strcmp(element_name, "item") == 0) {
		g_queue_push_tail(priv->element_queue, GINT_TO_POINTER(ELEMENT_ITEM));
		return;
	}
	
	if ((elem != ELEMENT_PARAM && elem != ELEMENT_ITEM) ||
	    (child_type = (GType) g_hash_table_lookup(priv->children_table, element_name)) == 0) {

		g_set_error(error,
			    G_MARKUP_ERROR,
			    G_MARKUP_ERROR_UNKNOWN_ELEMENT,
			    _("Invalid element '%s': child type not found"), element_name);
		return;
	}
	g_queue_push_tail(priv->element_queue, GINT_TO_POINTER(ELEMENT_CHILD));
	g_queue_push_tail(priv->object_queue, g_object_new(child_type, NULL));
}

static void
end_element_handler    (GMarkupParseContext *context,
                        const gchar         *element_name,
                        gpointer             user_data,
                        GError             **error)
{
	LoquiProfileHandle *handle;
	LoquiProfileHandlePrivate *priv;
	ElementType elem;
	GValueArray *value_array;
	GValue value = {0, };
	GParamSpec *pspec;
	GObject *obj, *tmp_obj;
	
        g_return_if_fail(user_data != NULL);
        g_return_if_fail(LOQUI_IS_PROFILE_HANDLE(user_data));

        handle = LOQUI_PROFILE_HANDLE(user_data);

	priv = handle->priv;
	
	elem = GET_CURRENT_ELEMENT(handle);
	g_queue_pop_tail(priv->element_queue);
	switch (elem) {
	case ELEMENT_LIST:
		value_array = g_queue_pop_tail(priv->value_array_queue);
		if (!value_array) {
	      		g_set_error(error,
	                	    G_MARKUP_ERROR,
	                   	    G_MARKUP_ERROR_INVALID_CONTENT,
	                   	    _("null value array"));
	                return;
		}
		pspec = g_queue_peek_tail(priv->pspec_queue);
		obj = g_queue_peek_tail(priv->object_queue);
		g_value_init(&value, G_TYPE_VALUE_ARRAY);
		g_value_set_boxed(&value, value_array);
		g_object_set_property(obj, g_param_spec_get_name(pspec), &value);
		break;
	case ELEMENT_PARAM:
		pspec = g_queue_pop_tail(priv->pspec_queue);
		if (G_IS_PARAM_SPEC_OBJECT(pspec)) {
			tmp_obj = g_queue_pop_tail(priv->object_queue);
			obj = g_queue_pop_tail(priv->object_queue);
			g_value_init(&value, G_TYPE_OBJECT);
			g_value_set_object(&value, tmp_obj);
			g_object_set_property(obj, g_param_spec_get_name(pspec), &value);
			g_value_unset(&value);
		}
		break;
	case ELEMENT_ITEM:
		pspec = g_queue_peek_tail(priv->pspec_queue);
		if (!G_IS_PARAM_SPEC_VALUE_ARRAY(pspec)) {
	      		g_set_error(error,
	                	    G_MARKUP_ERROR,
	                   	    G_MARKUP_ERROR_INVALID_CONTENT,
	                   	    _("object item not in value array of object"));
	                return;
		}
		if (G_IS_PARAM_SPEC_OBJECT(G_PARAM_SPEC_VALUE_ARRAY(pspec)->element_spec)) { 
			obj = g_queue_pop_tail(priv->object_queue);
			g_value_init(&value, G_TYPE_OBJECT);
			g_value_set_object(&value, obj);
			value_array = g_queue_peek_tail(priv->value_array_queue);
			g_value_array_append(value_array, &value);
			g_value_unset(&value);
		}
		break;
	case ELEMENT_ACCOUNT:
		obj = g_queue_pop_tail(priv->object_queue);
		if (!obj) {
	      		g_set_error(error,
	                	    G_MARKUP_ERROR,
	                   	    G_MARKUP_ERROR_INVALID_CONTENT,
	                   	    _("null object"));
	                return;
		}
		priv->profile_list = g_list_append(priv->profile_list, obj);
		break;
	default:
		break;
	}
}

static gboolean
parse_text(GParamSpec *pspec, const gchar *text, GValue *value)
{
	guint64 i, j;
	gdouble d;
	GValue tmp = {0, };
	
	if (G_IS_PARAM_SPEC_STRING(pspec)) {
		g_value_init(value, G_TYPE_STRING);
		g_value_set_string(value, text);
		return TRUE;
	}
	if (G_IS_PARAM_SPEC_BOOLEAN(pspec)) {
		g_value_init(value, G_TYPE_BOOLEAN);
		if (g_ascii_strcasecmp(text, "false") == 0 || strcmp(text, "0") == 0)
			g_value_set_boolean(value, FALSE);
		else
			g_value_set_boolean(value, TRUE);
		return TRUE;
	}
	
	if (G_IS_PARAM_SPEC_UCHAR(pspec) ||
	    G_IS_PARAM_SPEC_UINT(pspec) ||
	    G_IS_PARAM_SPEC_ULONG(pspec) ||
	    G_IS_PARAM_SPEC_ENUM(pspec)) {
		g_value_init(&tmp, G_TYPE_UINT);
	    	i = g_ascii_strtoull(text, NULL, 10);
		g_value_set_uint(&tmp, (guint) MIN(i, G_MAXUINT));
		g_value_init(value, G_PARAM_SPEC_VALUE_TYPE(pspec));
		return g_param_value_convert(pspec, &tmp, value, FALSE);
	    }
	if (G_IS_PARAM_SPEC_CHAR(pspec) ||
	    G_IS_PARAM_SPEC_INT(pspec) ||
	    G_IS_PARAM_SPEC_LONG(pspec)) {
		g_value_init(&tmp, G_TYPE_INT);
		if(*text == '+' || *text == '-') 
			i = g_ascii_strtoull(text+1, NULL, 10);
		else
			i = g_ascii_strtoull(text, NULL, 10);
		j = (int) MIN(i, G_MAXINT);
		if(*text == '-')
			j *= -1;
		g_value_set_int(&tmp, j);
		g_value_init(value, G_PARAM_SPEC_VALUE_TYPE(pspec));
		return g_param_value_convert(pspec, &tmp, value, FALSE);
	    }
	if (G_IS_PARAM_SPEC_FLOAT(pspec)) {
		d = g_ascii_strtod(text, NULL);
		g_value_init(value, G_TYPE_FLOAT);
		g_value_set_float(value, d);
		return TRUE;
	} else if (G_IS_PARAM_SPEC_DOUBLE(pspec)) {
		d = g_ascii_strtod(text, NULL);
		g_value_init(value, G_TYPE_DOUBLE);;
		g_value_set_double(value, d);
		return TRUE;
	}
	return FALSE;
	
}

static void
text_handler           (GMarkupParseContext *context,
                        const gchar         *text,
                        gsize                text_len,
                        gpointer             user_data,
                        GError             **error)
{
	LoquiProfileHandle *handle;
	LoquiProfileHandlePrivate *priv;
	GObject *obj;
	GParamSpec *pspec;
	GParamSpec *element_spec;
	GValueArray *value_array;
	GValue value = {0, };
	
	ElementType elem;
	
        g_return_if_fail(user_data != NULL);
        g_return_if_fail(LOQUI_IS_PROFILE_HANDLE(user_data));

        handle = LOQUI_PROFILE_HANDLE(user_data);

	priv = handle->priv;
	elem = GET_CURRENT_ELEMENT(handle);	

	switch(elem) {
   	case ELEMENT_ITEM:
		if ((pspec = g_queue_peek_tail(priv->pspec_queue)) == NULL) {
	      		g_set_error(error,
	                	    G_MARKUP_ERROR,
	                   	    G_MARKUP_ERROR_INVALID_CONTENT,
	                   	    _("Invalid item: doesn't have pspec"));
	                return;
		}
		if (!G_IS_PARAM_SPEC_VALUE_ARRAY(pspec)) {
	      		g_set_error(error,
	                	    G_MARKUP_ERROR,
	                   	    G_MARKUP_ERROR_INVALID_CONTENT,
	                   	    _("Invalid item: not list param"));
	                return;
		}
		if ((value_array = g_queue_peek_tail(priv->value_array_queue)) == NULL) {
	      		g_set_error(error,
	                	    G_MARKUP_ERROR,
	                   	    G_MARKUP_ERROR_INVALID_CONTENT,
	                   	    _("Invalid item: doesn't have value_array"));
	                return;
		}
		element_spec = G_PARAM_SPEC_VALUE_ARRAY(pspec)->element_spec;
		if (element_spec == NULL) {
	      		g_set_error(error,
	                	    G_MARKUP_ERROR,
	                   	    G_MARKUP_ERROR_INVALID_CONTENT,
	                   	    _("Invalid item: can't determine element type"));
	                return;
		}
		if(!parse_text(element_spec, text, &value)) {
	      		g_set_error(error,
	                	    G_MARKUP_ERROR,
	                   	    G_MARKUP_ERROR_INVALID_CONTENT,
	                   	    _("Invalid item: invalid data '%s'"), text);
	                return;
	        }
		g_value_array_append(value_array, &value);
		g_value_unset(&value);
		
		break;
   	case ELEMENT_PARAM:
		if ((pspec = g_queue_peek_tail(priv->pspec_queue)) == NULL) {
	      		g_set_error(error,
	                	    G_MARKUP_ERROR,
	                   	    G_MARKUP_ERROR_INVALID_CONTENT,
	                   	    _("Invalid param: doesn't have pspec"));
	                return;
		}
		if (G_IS_PARAM_SPEC_VALUE_ARRAY(pspec) || G_IS_PARAM_SPEC_OBJECT(pspec)) {
	                return;
		}
		if ((obj = g_queue_peek_tail(priv->object_queue)) == NULL) {
	      		g_set_error(error,
	                	    G_MARKUP_ERROR,
	                   	    G_MARKUP_ERROR_INVALID_CONTENT,
	                   	    _("Invalid param: doesn't have obj"));
	                return;
		}
		if(!parse_text(pspec, text, &value)) {
	      		g_set_error(error,
	                	    G_MARKUP_ERROR,
	                   	    G_MARKUP_ERROR_INVALID_CONTENT,
	                   	    _("Invalid param: invalid data '%s'"), text);
	                return;
	        }
		g_object_set_property(obj, g_param_spec_get_name(pspec), &value);
		g_value_unset(&value);
		
		break;
	default:
		break;
	}

}
static void
error_handler          (GMarkupParseContext *context,
                        GError              *error,
                        gpointer             user_data)
{
	g_warning(_("LoquiProfileHandle parse error: %s"), error->message);
}

static G_CONST_RETURN gchar *
loqui_profile_handle_parse_attribute(const gchar *key,
				     const gchar **attribute_names,
                        	     const gchar **attribute_values)
{
	gint i;
	
	for (i = 0; attribute_names[i] != NULL; i++) {
		if (strcmp(attribute_names[i], key) == 0)
			return attribute_values[i];
	}
	return NULL;
}
static void
loqui_profile_handle_clear(LoquiProfileHandle *handle)
{
	LoquiProfileHandlePrivate *priv;

        g_return_if_fail(handle != NULL);
        g_return_if_fail(LOQUI_IS_PROFILE_HANDLE(handle));

	priv = handle->priv;
	
	if (priv->context) {
		g_markup_parse_context_free(priv->context);
		priv->context = NULL;
	}
	if (priv->element_queue) {
		g_queue_free(priv->element_queue);
		priv->element_queue = NULL;
	}
	if (priv->object_queue) {
		g_list_foreach(priv->object_queue->head, (GFunc) g_object_unref, NULL);
		g_queue_free(priv->object_queue);
		priv->profile_list = NULL;
	}
	if (priv->pspec_queue) {
		g_queue_free(priv->pspec_queue);
		priv->pspec_queue = NULL;
	}
	if (priv->value_array_queue) {
		g_list_foreach(priv->value_array_queue->head, (GFunc) g_value_array_free, NULL);
		g_queue_free(priv->value_array_queue);
		priv->profile_list = NULL;
	}
	if (priv->profile_list) {
		g_list_foreach(priv->profile_list, (GFunc) g_object_unref, NULL);
		g_list_free(priv->profile_list);
		priv->profile_list = NULL;
	}
}

LoquiProfileHandle*
loqui_profile_handle_new(void)
{
        LoquiProfileHandle *handle;
	LoquiProfileHandlePrivate *priv;

	handle = g_object_new(loqui_profile_handle_get_type(), NULL);
	
        priv = handle->priv;

	priv->type_table = g_hash_table_new(g_str_hash, g_str_equal);
	priv->children_table = g_hash_table_new(g_str_hash, g_str_equal);
	
	priv->type_table_reverse = g_hash_table_new(g_int_hash, g_int_equal);
	priv->children_table_reverse = g_hash_table_new(g_int_hash, g_int_equal);

        return handle;
}
void
loqui_profile_handle_register_type(LoquiProfileHandle *handle, const gchar* name, GType type)
{
	LoquiProfileHandlePrivate *priv;
	
        g_return_if_fail(handle != NULL);
        g_return_if_fail(LOQUI_IS_PROFILE_HANDLE(handle));

        priv = handle->priv;
        
	g_hash_table_insert(priv->type_table, (gpointer) name, GINT_TO_POINTER((int) type));
	g_hash_table_insert(priv->type_table_reverse, GINT_TO_POINTER((int) type), (gpointer) name);
}

void
loqui_profile_handle_register_child_type(LoquiProfileHandle *handle, const gchar* name, GType type)
{
	LoquiProfileHandlePrivate *priv;
	
        g_return_if_fail(handle != NULL);
        g_return_if_fail(LOQUI_IS_PROFILE_HANDLE(handle));

        priv = handle->priv;
        
	g_hash_table_insert(priv->children_table, (gpointer) name, GINT_TO_POINTER((int) type));
	g_hash_table_insert(priv->children_table_reverse, GINT_TO_POINTER((int) type), (gpointer) name);
}

gboolean
loqui_profile_handle_read_from_buffer(LoquiProfileHandle *handle, GList **profiles, const gchar *buf)
{
	GError *error = NULL;
	LoquiProfileHandlePrivate *priv;
	
        g_return_val_if_fail(handle != NULL, FALSE);
        g_return_val_if_fail(LOQUI_IS_PROFILE_HANDLE(handle), FALSE);
	g_return_val_if_fail(profiles != NULL, FALSE);

        priv = handle->priv;

	loqui_profile_handle_clear(handle);

	priv->context = g_markup_parse_context_new(&parser, 0, handle, NULL);
	priv->element_queue = g_queue_new();
	priv->object_queue = g_queue_new();
	priv->pspec_queue = g_queue_new();
	priv->value_array_queue = g_queue_new();
	priv->profile_list = NULL;

	if (!g_markup_parse_context_parse(priv->context, buf, -1, &error)) {
		g_warning(_("LoquiProfile parse error: %s"), error->message);
		g_error_free(error);
		return FALSE;
	}
	*profiles = g_list_copy(priv->profile_list);

	return TRUE;
}

gboolean
loqui_profile_handle_read_from_file(LoquiProfileHandle *handle, GList **profiles, const gchar *path)
{
	gchar *contents;
	GError *error = NULL;
	gboolean result;
	LoquiProfileHandlePrivate *priv;
	
        g_return_val_if_fail(handle != NULL, FALSE);
        g_return_val_if_fail(LOQUI_IS_PROFILE_HANDLE(handle), FALSE);
	g_return_val_if_fail(profiles != NULL, FALSE);

        priv = handle->priv;
	
	if (!g_file_get_contents(path, &contents, NULL, &error)) {
		if(error->code != G_FILE_ERROR_NOENT)
			g_warning("LoquiProfile read error %s", error->message);

		g_error_free(error);
		return FALSE;
	}
	result = loqui_profile_handle_read_from_buffer(handle, profiles, contents);
	g_free(contents);
	
	return result;
}

static gchar *
loqui_profile_handle_gvalue_to_xml(LoquiProfileHandle *handle, GParamSpec *pspec, GValue *value)
{
	GString *string;
	const gchar *tmp;
	gchar *buf;
	GParamSpec *element_spec;
	GValueArray *value_array;
	GValue *tmp_val;
	GValue val = {0, };
	guint i;
	GObject *obj;

	string = g_string_new("");
	if (G_IS_PARAM_SPEC_VALUE_ARRAY(pspec)) {
		element_spec = G_PARAM_SPEC_VALUE_ARRAY(pspec)->element_spec;
		if (!element_spec) {
			g_warning(_("ProfileHandle: Set element spec."));
		} else {
			g_string_append_printf(string, "\n<list>\n");
			value_array = g_value_get_boxed(value);
			for (i = 0; i < value_array->n_values; i++) {
				g_string_append(string, "<item>");
				tmp_val = g_value_array_get_nth(value_array, i);
				buf = loqui_profile_handle_gvalue_to_xml(handle, element_spec, tmp_val);
				g_string_append(string, buf);
				g_free(buf);
				g_string_append_printf(string, "</item>\n");
			}
			g_string_append_printf(string, "</list>\n");
		}
	} else if (G_IS_PARAM_SPEC_OBJECT(pspec)) {
		obj = g_value_get_object(value);
		buf = loqui_profile_handle_object_to_xml(handle, TRUE, obj);
		if (buf) {
			g_string_append(string, buf);
			g_free(buf);
		}
	} else if (G_IS_PARAM_SPEC_STRING(pspec)) {
		tmp = g_value_get_string(value);
		if (tmp) {
			buf = g_markup_escape_text(tmp, -1);
			g_string_append(string, buf);
			g_free(buf);
		}
	} else if (G_IS_PARAM_SPEC_BOOLEAN(pspec)) {
		if (g_value_get_boolean(value) == FALSE)
			g_string_append(string, "FALSE");
		else
			g_string_append(string, "TRUE");
	} else if (G_VALUE_HOLDS_LONG(value)) {
		g_string_append_printf(string, "%ld", g_value_get_long(value));
	} else if (G_VALUE_HOLDS_INT(value)) {
		g_string_append_printf(string, "%d", g_value_get_int(value));
	} else if (G_VALUE_HOLDS_UINT(value)) {
		g_string_append_printf(string, "%u", g_value_get_int(value));
#ifdef G_GINT_64_FORMAT
	} else if (G_VALUE_HOLDS_INT64(value)) {
		g_string_append_printf(string, "%" G_GINT64_FORMAT, g_value_get_int64(value));
#endif
#ifdef G_GUINT64_FORMAT
	} else if (G_VALUE_HOLDS_UINT64(value)) {
		g_string_append_printf(string, "%" G_GUINT64_FORMAT, g_value_get_uint64(value));
#endif
	} else if (G_VALUE_HOLDS_FLOAT(value)) {
		g_string_append_printf(string, "%f", g_value_get_float(value));
	} else if (G_VALUE_HOLDS_DOUBLE(value)) {
		g_string_append_printf(string, "%f", g_value_get_double(value));
	} else if (g_value_type_transformable(G_VALUE_TYPE(value), G_TYPE_LONG)) {
		g_value_init(&val, G_TYPE_LONG);
		g_value_transform(value, &val);
		g_string_append_printf(string, "%ld", g_value_get_long(value));
		g_value_unset(&val);
	} else {
		g_warning(_("ProfileHandle: Unsupported type: %s."), G_PARAM_SPEC_TYPE_NAME(pspec));
	}

	return g_string_free(string, FALSE);
}
	
static gchar *
loqui_profile_handle_object_to_xml(LoquiProfileHandle *handle, gboolean is_child, GObject *object)
{
	GString *string;
	GParamSpec **param_specs;
	GParamSpec *pspec;
	gchar *tmp;
	guint i, n_params;
	GValue value = {0, };
	const gchar *name;
	const gchar *child_name = NULL;
	LoquiProfileHandlePrivate *priv;

        g_return_val_if_fail(handle != NULL, NULL);
        g_return_val_if_fail(LOQUI_IS_PROFILE_HANDLE(handle), NULL);

	priv = handle->priv;

	string = g_string_new("");

	if (is_child) {
		child_name = g_hash_table_lookup(priv->children_table_reverse, GINT_TO_POINTER((int) G_OBJECT_TYPE(object)));
		if (!child_name) {
			g_warning("ProfileHandle: child type not registered");
			return NULL;
		}
		g_string_printf(string, "<%s>", child_name);
	}

	param_specs = g_object_class_list_properties(G_OBJECT_GET_CLASS(object), &n_params);
	for (i = 0; i < n_params; i++) {
		pspec = param_specs[i];
		name = g_param_spec_get_name(pspec);

		g_string_append_printf(string, "<param key=\"%s\">", name);
		g_value_init(&value, G_PARAM_SPEC_VALUE_TYPE(pspec));
		g_object_get_property(object, name, &value);

		tmp = loqui_profile_handle_gvalue_to_xml(handle, pspec, &value);
		if (tmp == NULL) {
			g_warning("Failed to convert value.");
			continue;
		}
		g_string_append(string, tmp);
		g_free(tmp);
		g_string_append_printf(string, "</param>\n");

		g_value_unset(&value);
	}

	if (is_child) {
		g_string_printf(string, "</%s>", child_name);
	}

	return g_string_free(string, FALSE);
}
gboolean
loqui_profile_handle_write_to_buffer(LoquiProfileHandle *handle, GList *profile_list, gchar **buf)
{
	LoquiProfileHandlePrivate *priv;
	LoquiProfileAccount *profile;
	GList *cur;
	GString *string;
	gchar *tmp;

        g_return_val_if_fail(handle != NULL, FALSE);
        g_return_val_if_fail(LOQUI_IS_PROFILE_HANDLE(handle), FALSE);
	g_return_val_if_fail(profile_list != NULL, FALSE);

        priv = handle->priv;

	string = g_string_new(NULL);
	g_string_printf(string, "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n");
	g_string_append_printf(string, "<accounts>\n");

	for (cur = profile_list; cur != NULL; cur = cur->next) {
		profile = LOQUI_PROFILE_ACCOUNT(cur->data);

		tmp = g_hash_table_lookup(priv->type_table_reverse, GINT_TO_POINTER((int) G_OBJECT_TYPE(profile)));
		if (!tmp) {
			g_warning("ProfileHandle: type not registered: %s", G_OBJECT_TYPE_NAME(profile));
			continue;
		}
		g_string_append_printf(string, "<account type=\"%s\">\n", tmp);

		tmp = loqui_profile_handle_object_to_xml(handle, FALSE, G_OBJECT(profile));
		g_string_append(string, tmp);
		g_free(tmp);

		g_string_append_printf(string, "</account>\n");
	}
	g_string_append_printf(string, "</accounts>\n");

	*buf = g_string_free(string, FALSE);
	return TRUE;
}
gboolean
loqui_profile_handle_write_to_file(LoquiProfileHandle *handle, GList *profile_list, const gchar *path)
{
	LoquiProfileHandlePrivate *priv;
	gchar *buf;
	GIOChannel *io;

        g_return_val_if_fail(handle != NULL, FALSE);
        g_return_val_if_fail(LOQUI_IS_PROFILE_HANDLE(handle), FALSE);
	g_return_val_if_fail(profile_list != NULL, FALSE);

        priv = handle->priv;

	if(loqui_profile_handle_write_to_buffer(handle, profile_list, &buf) == FALSE)
		return FALSE;
	
	if((io = g_io_channel_new_file(path, "w", NULL)) == NULL) {
		g_warning("Failed to open profile(w)");
		return FALSE;
	}
	if(g_io_channel_write_chars(io, buf, -1, NULL, NULL) != G_IO_STATUS_NORMAL) {
		g_warning("Failed to write profile");
		return FALSE;
	}
	g_io_channel_unref(io);

	return TRUE;
}
