#include "test_helper.h"
#include "utils.h"

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
int
main()
{
	int all=0, failed=0;

	DO_TEST(all, failed, test_utils_format);
	DO_TEST(all, failed, test_utils_search_uri);

	SHOW_RESULT_AND_EXIT(all, failed, utils);
}

