#include <config.h>

#include "test_helper.h"
#include "utils.h"
#include <glib.h>

static int
test_utils_format(void)
{
   return TRUE;
}

static int
test_utils_search_uri(void)
{
	const gchar *uri1 = "hoge http://hoge aaaa";
	const gchar *uri2 = "http://fuga/< ttp://bar";
	gchar *got_uri;
	const gchar *start_uri, *end_uri;

	g_return_val_if_fail(utils_search_uri(uri1, &got_uri, &start_uri, &end_uri), FALSE);
	g_print("got_uri: %s\n", got_uri);
	g_return_val_if_fail(strcmp(got_uri, "http://hoge") == 0, FALSE);
	g_return_val_if_fail(start_uri == uri1 + 5, FALSE);
	g_return_val_if_fail(end_uri == uri1 + 15, FALSE);
	g_free(got_uri);

	g_return_val_if_fail(utils_search_uri(uri2, &got_uri, &start_uri, &end_uri), FALSE);
	g_print("got_uri: %s\n", got_uri);
	g_return_val_if_fail(strcmp(got_uri, "http://fuga/") == 0, FALSE);
	g_free(got_uri);

	g_return_val_if_fail(utils_search_uri(end_uri, &got_uri, &start_uri, &end_uri), FALSE);
	g_print("got_uri: %s\n", got_uri);
	g_return_val_if_fail(strcmp(got_uri, "ttp://bar") == 0, FALSE);
	g_free(got_uri);

	return TRUE;
}

static gint
sort_func(gconstpointer *a, gconstpointer *b)
{
	g_print("compare: %d - %d\n", GPOINTER_TO_INT(*a), GPOINTER_TO_INT(*b));
	return GPOINTER_TO_INT(*a) - GPOINTER_TO_INT(*b);
}
static int
test_utils_g_ptr_array_insert_sort(void)
{
	GPtrArray *ptr_array;
	gint i;

	ptr_array = g_ptr_array_new();
	g_ptr_array_add(ptr_array, GINT_TO_POINTER(2));
	g_ptr_array_add(ptr_array, GINT_TO_POINTER(2));
	g_ptr_array_add(ptr_array, GINT_TO_POINTER(2));
	g_ptr_array_add(ptr_array, GINT_TO_POINTER(2));
	g_ptr_array_add(ptr_array, GINT_TO_POINTER(1));
	utils_g_ptr_array_insert_sort(ptr_array, 4, (GCompareFunc) sort_func);
//	g_ptr_array_sort(ptr_array, (GCompareFunc) sort_func);
	for(i = 0; i < ptr_array->len; i++) {
		g_print("Result[%d] = %d\n", i, GPOINTER_TO_INT(g_ptr_array_index(ptr_array, i)));
	}
	g_ptr_array_free(ptr_array, TRUE);
	
	return TRUE;
}
int
main()
{
	int all=0, failed=0;

	DO_TEST(all, failed, test_utils_format);
	DO_TEST(all, failed, test_utils_search_uri);
	DO_TEST(all, failed, test_utils_g_ptr_array_insert_sort);

	SHOW_RESULT_AND_EXIT(all, failed, utils);
}

