/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui TitleFormat Utility <http://loqui.good-day.net/>
 * Copyright (C) 2004 Yoichi Imai <yoichi@silver-forest.com>
 *
 * You can use TitleFormat on these conditions.
 * (in GPL/LGPL program, TitleFormat follows GPL/LGPL.)
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *  notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *  notice, this list of conditions and the following disclaimer in the
 *  documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *  may be used to endorse or promote products derived from this software
 *  without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/* TODO:
 *  - test function
 *  - standard functions
 */

#include "config.h"
#include "loqui_title_format.h"
#include "loqui_string_tokenizer.h"
#include <string.h>

typedef union _TFItem TFItem;
typedef struct _TFItemText TFItemText;
typedef struct _TFItemVariable TFItemVariable;
typedef struct _TFItemVariableArea TFItemVariableArea;
typedef struct _TFItemFunction TFItemFunction;

typedef enum {
	LOQUI_TITLE_FORMAT_NODE_INVALID = 0,
	LOQUI_TITLE_FORMAT_NODE_ROOT,
	LOQUI_TITLE_FORMAT_NODE_VARIABLE_AREA,
	LOQUI_TITLE_FORMAT_NODE_ARGUMENT_HOLDER,
	LOQUI_TITLE_FORMAT_NODE_TEXT,
	LOQUI_TITLE_FORMAT_NODE_VARIABLE,
	LOQUI_TITLE_FORMAT_NODE_FUNCTION,
} TFItemType;

struct _TFItemText {
	TFItemType type;
	gchar *text;
};

struct _TFItemVariable {
	TFItemType type;
	gchar *name;
};

struct _TFItemVariableArea {
	TFItemType type;
};

struct _TFItemFunction {
	TFItemType type;
	gchar *name;
};

union _TFItem {
	TFItemType type;
	TFItemText v_text;
	TFItemVariable v_variable;
	TFItemFunction v_function;
};


static gboolean loqui_title_format_validate_symbol_name(const gchar *name);
static gboolean loqui_title_format_parse_internal(LoquiTitleFormat *ltf, LoquiStringTokenizer *st, gchar *end_chars, gchar *delim_ptr, GNode *parent, GNode *sibling, GError **error);
static gboolean loqui_title_format_parse_function(LoquiTitleFormat *ltf, LoquiStringTokenizer *st, GNode *parent, GNode *sibling, GError **error);
static GNode *tf_item_node_new(TFItemType type, const gchar *name, const gchar *text);

static void loqui_title_format_fetch_variable_area(LoquiTitleFormat *ltf, GNode *node, GString *string);
static void loqui_title_format_fetch_internal(LoquiTitleFormat *ltf, GNode *node, GString *string, gboolean *var_set);

static gboolean
loqui_title_format_validate_symbol_name(const gchar *name)
{
	const gchar* s;
	
	s = name;
	while(*s != '\0') {
		if(!strchr(G_CSET_A_2_Z G_CSET_a_2_z G_CSET_DIGITS "-_", *s))
			return FALSE;
		s++;
	}
	return TRUE;
}
/* called at v   (after $)
            $func()
*/
static gboolean
loqui_title_format_parse_function(LoquiTitleFormat *ltf, LoquiStringTokenizer *st, GNode *parent, GNode *sibling, GError **error)
{
	gchar delim;
	const gchar *func_name;
	GNode *node;
	TFItem *tfitem;

	loqui_string_tokenizer_set_delimiters(st, "(");
	func_name = loqui_string_tokenizer_next_token(st, &delim);
	if (!func_name || delim != '(') {
		g_set_error(error,
                            LOQUI_TITLE_FORMAT_ERROR,
                            LOQUI_TITLE_FORMAT_ERROR_UNTERMINATED_FUNCTION_NAME,
                            "Unterminated function name");
		return FALSE;
	}
	if (!loqui_title_format_validate_symbol_name(func_name)) {
                g_set_error(error,
                            LOQUI_TITLE_FORMAT_ERROR,
                            LOQUI_TITLE_FORMAT_ERROR_INVALID_FUNCTION_NAME,
                            "Invalid function name: %s", func_name);
		return FALSE;
	}

	
	node = tf_item_node_new(LOQUI_TITLE_FORMAT_NODE_FUNCTION, func_name, NULL);
	g_node_insert_after(parent, sibling, node);

	parent = node;
	sibling = NULL;

	node = tf_item_node_new(LOQUI_TITLE_FORMAT_NODE_ARGUMENT_HOLDER, NULL, NULL);
	g_node_insert_after(parent, sibling, node);

	parent = node;
	sibling = NULL;

	while (TRUE) {
		if (!loqui_title_format_parse_internal(ltf, st, ",)", &delim, parent, sibling, error))
			return FALSE;

		if (delim == '\0') {
			g_set_error(error,
				    LOQUI_TITLE_FORMAT_ERROR,
				    LOQUI_TITLE_FORMAT_ERROR_UNTERMINATED_FUNCTION_ARGUMENT,
				    "Unterminated function argument");
			return FALSE;
		}

		if (delim == ',') {
			tfitem = parent->data;
			if (tfitem->type != LOQUI_TITLE_FORMAT_NODE_ARGUMENT_HOLDER) {
				g_set_error(error,
					    LOQUI_TITLE_FORMAT_ERROR,
					    LOQUI_TITLE_FORMAT_ERROR_INVALID_INTERNAL_STRUCTURE,
					    "Parent type is not argument holder");
				return FALSE;
			}

			node = tf_item_node_new(LOQUI_TITLE_FORMAT_NODE_ARGUMENT_HOLDER, NULL, NULL);
			g_node_insert_after(parent->parent, parent, node);

			parent = node;
			sibling = NULL;
			continue;
		}

		if (delim == ')') {
			tfitem = parent->data;
			if (tfitem->type != LOQUI_TITLE_FORMAT_NODE_ARGUMENT_HOLDER) {
				g_set_error(error,
					    LOQUI_TITLE_FORMAT_ERROR,
					    LOQUI_TITLE_FORMAT_ERROR_INVALID_INTERNAL_STRUCTURE,
					    "Parent type is not argument holder");
				return FALSE;
			}
			g_assert(parent->parent != NULL);

			tfitem = parent->parent->data;
			if (tfitem->type != LOQUI_TITLE_FORMAT_NODE_FUNCTION) {
				g_set_error(error,
					    LOQUI_TITLE_FORMAT_ERROR,
					    LOQUI_TITLE_FORMAT_ERROR_INVALID_INTERNAL_STRUCTURE,
					    "Grandparent type is not function");
				return FALSE;
			}
			sibling = parent->parent; /* arg->func */
			parent = parent->parent->parent; /* arg->func->(parent) */
		}
		break;
	}

	return TRUE;
}
/* called at v   (after [)
            [hogehoge
*/
static gboolean
loqui_title_format_parse_variable_area(LoquiTitleFormat *ltf, LoquiStringTokenizer *st, GNode *parent, GNode *sibling, GError **error)
{
	gchar delim;
	GNode *node;

	node = tf_item_node_new(LOQUI_TITLE_FORMAT_NODE_VARIABLE_AREA, NULL, NULL);
	g_node_insert_after(parent, sibling, node);

	parent = node;
	sibling = NULL;

	if (!loqui_title_format_parse_internal(ltf, st, "]", &delim, parent, sibling, error))
		return FALSE;

	if (delim == '\0') {
		g_set_error(error,
			    LOQUI_TITLE_FORMAT_ERROR,
			    LOQUI_TITLE_FORMAT_ERROR_UNTERMINATED_FUNCTION_ARGUMENT,
			    "Unterminated variable area");
		return FALSE;
	}
	return TRUE;
}

/* this function supports the text has one line */
static gboolean
loqui_title_format_remove_comment(const gchar *text, gchar **result)
{
	gchar *tmp;
	gboolean is_include_comment = FALSE;
	gchar *buf;

	buf = g_strdup(text);
	
	if ((tmp = strstr(buf, "//")) != NULL) {
		is_include_comment = TRUE;
		*tmp = '\0';
	}

	if (result)
		*result = buf;
	else
		g_free(buf);

	return is_include_comment;
}
static gboolean
loqui_title_format_parse_internal(LoquiTitleFormat *ltf, LoquiStringTokenizer *st, gchar *end_chars, gchar *delim_ptr, GNode *parent, GNode *sibling, GError **error)
{
	GNode *node;
	gchar *delimiters;
	const gchar *token, *text, *var_name;
	gchar delim, d;
	gchar *tmp;
	gboolean is_include_comment;

	/* end_chars is NULL or not */
	delimiters = g_strconcat("$[%'\n", end_chars, NULL);
	loqui_string_tokenizer_set_delimiters(st, delimiters);

	while ((token = loqui_string_tokenizer_next_token(st, &delim)) != NULL) {
		is_include_comment = loqui_title_format_remove_comment(token, &tmp);
		node = tf_item_node_new(LOQUI_TITLE_FORMAT_NODE_TEXT, NULL, tmp);
		g_free(tmp);
		
		g_node_insert_after(parent, sibling, node);
		sibling = node;

		if (delim == '\0')
			break;

		if (is_include_comment && delim != '\n') {
			loqui_string_tokenizer_set_delimiters(st, "\n");
			loqui_string_tokenizer_next_token(st, NULL);
			loqui_string_tokenizer_set_delimiters(st, delimiters);
			continue;
		}

		// when ended with end_chars
		if (end_chars != NULL &&
		    strchr(end_chars, delim) != NULL) {
			if (delim_ptr)
				*delim_ptr = delim;
			g_free(delimiters);
			return TRUE;
		}

		switch (delim) {
		case '\n':
			break;
		case '$':
			if (!loqui_title_format_parse_function(ltf, st, parent, sibling, error))
				return FALSE;

			loqui_string_tokenizer_set_delimiters(st, delimiters);
			break;
		case '[':
			if (!loqui_title_format_parse_variable_area(ltf, st, parent, sibling, error))
				return FALSE;

			loqui_string_tokenizer_set_delimiters(st, delimiters);
			break;
		case '%':
			loqui_string_tokenizer_set_delimiters(st, "%");
			var_name = loqui_string_tokenizer_next_token(st, &d);
			if (!var_name || d != '%') {
				loqui_string_tokenizer_set_delimiters(st, delimiters);
				g_set_error(error,
                                            LOQUI_TITLE_FORMAT_ERROR,
                                            LOQUI_TITLE_FORMAT_ERROR_INVALID_VARIABLE_NAME,
                                            "Invalid variable name");
				return FALSE;
			}
			if (!loqui_title_format_validate_symbol_name(var_name)) {
				loqui_string_tokenizer_set_delimiters(st, delimiters);
				g_set_error(error,
                                            LOQUI_TITLE_FORMAT_ERROR,
                                            LOQUI_TITLE_FORMAT_ERROR_INVALID_VARIABLE_NAME,
                                            "Invalid variable name: %s", var_name);
				return FALSE;
			}

			node = tf_item_node_new(LOQUI_TITLE_FORMAT_NODE_VARIABLE, var_name, NULL);
			g_node_insert_after(parent, sibling, node);

			sibling = node;
			loqui_string_tokenizer_set_delimiters(st, delimiters);
			break;
		case '\'':
			loqui_string_tokenizer_set_delimiters(st, "'");
			text = loqui_string_tokenizer_next_token(st, &d);
			if (!text || d != '\'') {
				loqui_string_tokenizer_set_delimiters(st, delimiters);
				g_set_error(error,
                                            LOQUI_TITLE_FORMAT_ERROR,
                                            LOQUI_TITLE_FORMAT_ERROR_UNTERMINATED_QUOTATION,
                                            "Unterminated quotation");
				return FALSE;
			}
			
			
			node = tf_item_node_new(LOQUI_TITLE_FORMAT_NODE_TEXT, NULL, text);
			g_node_insert_after(parent, sibling, node);
			sibling = node;

			loqui_string_tokenizer_set_delimiters(st, delimiters);
			break;
		default:
			g_assert_not_reached();
		}
	}

	if (delim_ptr)
		*delim_ptr = delim;
	g_free(delimiters);
	return TRUE;
}
static void
tf_free_item(TFItem *item)
{
	if (item == NULL)
		return;

	switch (item->type) {
	case LOQUI_TITLE_FORMAT_NODE_ROOT:
	case LOQUI_TITLE_FORMAT_NODE_VARIABLE_AREA:
	case LOQUI_TITLE_FORMAT_NODE_ARGUMENT_HOLDER:
		break;
	case LOQUI_TITLE_FORMAT_NODE_TEXT:
		g_free(((TFItemText *) item)->text);
		break;
	case LOQUI_TITLE_FORMAT_NODE_VARIABLE:
		g_free(((TFItemVariable *) item)->name);
		break;
	case LOQUI_TITLE_FORMAT_NODE_FUNCTION:
		g_free(((TFItemFunction *) item)->name);
		break;
	default:
		g_assert_not_reached();
	}
	g_free(item);
}
static gboolean
tf_free_node_func(GNode *node, gpointer data)
{
	tf_free_item(node->data);
	return FALSE;
}
static void
tf_free_tree(GNode *node)
{
	g_node_traverse(node, G_TRAVERSE_ALL, G_IN_ORDER, -1, tf_free_node_func, NULL);
	g_node_destroy(node);
}
static GNode *
tf_item_node_new(TFItemType type, const gchar *name, const gchar *text)
{
	TFItem *tfitem = NULL;

	switch (type) {
	case LOQUI_TITLE_FORMAT_NODE_ROOT:
	case LOQUI_TITLE_FORMAT_NODE_VARIABLE_AREA:
	case LOQUI_TITLE_FORMAT_NODE_ARGUMENT_HOLDER:
		tfitem = (TFItem *) g_new0(TFItemType, 1);
		break;
	case LOQUI_TITLE_FORMAT_NODE_TEXT:
		tfitem = (TFItem *) g_new0(TFItemText, 1);
		((TFItemText *) tfitem)->text = g_strdup(text);
		break;
	case LOQUI_TITLE_FORMAT_NODE_VARIABLE:
		tfitem = (TFItem *) g_new0(TFItemVariable, 1);
		((TFItemVariable *) tfitem)->name = g_strdup(name);
		break;
	case LOQUI_TITLE_FORMAT_NODE_FUNCTION:
		tfitem = (TFItem *) g_new0(TFItemFunction, 1);
		((TFItemFunction *) tfitem)->name = g_strdup(name);
		break;
	default:
		g_assert_not_reached();
	}

	tfitem->type = type;
	return g_node_new(tfitem);
}
GQuark
loqui_title_format_error_quark(void)
{
        static GQuark quark = 0;
        if (!quark)
                quark = g_quark_from_static_string("loqui-title-format-error-quark");

        return quark;
}
LoquiTitleFormat *
loqui_title_format_new(void)
{
	LoquiTitleFormat *ltf;

	ltf = g_new0(LoquiTitleFormat, 1);
	ltf->function_table = g_hash_table_new_full(g_str_hash, g_str_equal, (GDestroyNotify) g_free, NULL);
	ltf->variable_table = g_hash_table_new_full(g_str_hash, g_str_equal, (GDestroyNotify) g_free, (GDestroyNotify) g_free);
	return ltf;
}
gboolean
loqui_title_format_parse(LoquiTitleFormat *ltf, const gchar *str, GError **error)
{
	GNode *parent, *sibling;
	LoquiStringTokenizer *st;
	gboolean result;

	g_return_val_if_fail(ltf != NULL, FALSE);

	if (ltf->root)
		tf_free_tree(ltf->root);

	parent = tf_item_node_new(LOQUI_TITLE_FORMAT_NODE_ROOT, NULL, NULL);
	sibling = NULL;

	ltf->root = parent;

	st = loqui_string_tokenizer_new(str, "");
	result = loqui_title_format_parse_internal(ltf, st, NULL, NULL, parent, sibling, error);
	loqui_string_tokenizer_free(st);
	if (!result && ltf->root)
		tf_free_tree(ltf->root);

	return result;
}
static void
loqui_title_format_fetch_variable_area(LoquiTitleFormat *ltf, GNode *node, GString *string)
{
	GString *string_child;
	gboolean var_set = FALSE;

	string_child = g_string_new(NULL);
	loqui_title_format_fetch_internal(ltf, node->children, string_child, &var_set);
	if (var_set && string_child->str != NULL)
		g_string_append(string, string_child->str);
	g_string_free(string_child, TRUE);
}
static void
loqui_title_format_fetch_internal(LoquiTitleFormat *ltf, GNode *node, GString *string, gboolean *var_set)
{
	GNode *cur;
	TFItem *tfitem;
	gchar *buf;
	
	for (cur = node; cur != NULL; cur = cur->next) {
		tfitem = cur->data;

		switch (tfitem->type) {
		case LOQUI_TITLE_FORMAT_NODE_ROOT:
			/* g_print("Parse: Node\n"); */
			loqui_title_format_fetch_internal(ltf, cur->children, string, NULL);
			break;
		case LOQUI_TITLE_FORMAT_NODE_VARIABLE_AREA:
			/* g_print("Parse: VariableArea\n"); */
			loqui_title_format_fetch_variable_area(ltf, cur, string);
			break;
		case LOQUI_TITLE_FORMAT_NODE_ARGUMENT_HOLDER:
			/* g_print("Parse: ArgumentHolder\n"); */
			g_assert(FALSE);
			break;
		case LOQUI_TITLE_FORMAT_NODE_TEXT:
			/* g_print("Parse: Text: %s\n", ((TFItemText *) tfitem)->text); */
			g_string_append(string, ((TFItemText *) tfitem)->text);
			break;
		case LOQUI_TITLE_FORMAT_NODE_VARIABLE:
			/* g_print("Parse: Variable: %s\n", ((TFItemVariable *) tfitem)->name); */
			buf = g_hash_table_lookup(ltf->variable_table, ((TFItemVariable *) tfitem)->name);
			if (buf) {
				if (var_set)
					*var_set = TRUE;
				g_string_append(string, buf);
			}
			break;
		case LOQUI_TITLE_FORMAT_NODE_FUNCTION:
			/* g_print("Parse: Function: %s\n", ((TFItemFunction *) tfitem)->name); */
			g_assert(FALSE);
			break;
		default:
			/* g_print("Parse: Unknown (%d)\n", tfitem->type); */
			g_assert_not_reached();
		}
	}
}
gchar *
loqui_title_format_fetch(LoquiTitleFormat *ltf)
{
	GString *string;
	g_return_val_if_fail(ltf != NULL, NULL);

	if (!ltf->root)
		return NULL;

	string = g_string_new(NULL);
	loqui_title_format_fetch_internal(ltf, ltf->root, string, NULL);
	return g_string_free(string, FALSE);
}
void
loqui_title_format_register_function(LoquiTitleFormat *ltf, const gchar *name, LoquiTitleFormatFunction *func)
{
	g_return_if_fail(ltf != NULL);

	if (func)
		g_hash_table_insert(ltf->function_table, g_strdup(name), func);
	else
		g_hash_table_remove(ltf->function_table, name);
}
void
loqui_title_format_register_variable(LoquiTitleFormat *ltf, const gchar *name, const gchar *value)
{
	g_return_if_fail(ltf != NULL);

	if (value)
		g_hash_table_insert(ltf->variable_table, g_strdup(name), g_strdup(value));
	else
		g_hash_table_remove(ltf->variable_table, name);
}
void
loqui_title_format_register_variables(LoquiTitleFormat *ltf, ...)
{
	va_list args;
	const gchar *name;
	const gchar *value;

	va_start(args, ltf);
	while ((name = va_arg(args, gchar *)) != NULL) {
		value = va_arg(args, gchar *);
		loqui_title_format_register_variable(ltf, name, value);
	}
	va_end(args);
}
void
loqui_title_format_free(LoquiTitleFormat *ltf)
{
	g_hash_table_destroy(ltf->variable_table);
	g_hash_table_destroy(ltf->function_table);
	if (ltf->root)
		tf_free_tree(ltf->root);
	g_free(ltf);
}
