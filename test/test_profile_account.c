#include "test_helper.h"
#include "loqui_profile_account.h"
#include <libloqui/loqui_protocol_irc.h>

static int
test_profile_account_make(void)
{
	LoquiProfileAccount *prof;
	GList *list = NULL;
	GList *tmp_list;

	prof = loqui_profile_account_new(loqui_protocol_irc_get());
	loqui_profile_account_set_nick(prof, "hoge");
	loqui_profile_account_set_servername(prof, "example.com");
	loqui_profile_account_set_username(prof, "user");
	loqui_profile_account_set_port(prof, 3323);

	list = g_list_append(list, g_strdup("hoge_away"));
	list = g_list_append(list, g_strdup("hoge_zzz"));
	loqui_profile_account_set_nick_list(prof, list);

	loqui_profile_account_print(prof);

	g_return_val_if_fail(strcmp(loqui_profile_account_get_nick(prof), "hoge") == 0, FALSE);
	g_return_val_if_fail(strcmp(loqui_profile_account_get_username(prof), "user") == 0, FALSE);
	g_return_val_if_fail(strcmp(loqui_profile_account_get_servername(prof), "example.com") == 0, FALSE);
	g_return_val_if_fail(loqui_profile_account_get_port(prof) == 3323, FALSE);

	tmp_list = loqui_profile_account_get_nick_list(prof);
	g_return_val_if_fail(g_list_length(tmp_list) == 2, FALSE);
	g_return_val_if_fail(strcmp(g_list_nth_data(tmp_list, 0), "hoge_away") == 0, FALSE);
	g_return_val_if_fail(strcmp(g_list_nth_data(tmp_list, 1), "hoge_zzz") == 0, FALSE);
	
	g_object_unref(prof);

	return TRUE;
}
static int
test_profile_account_object(void)
{
	LoquiProfileAccount *prof;
	gchar *tmp;
	gint i;

	prof = loqui_profile_account_new(loqui_protocol_irc_get());

	i = 9999;
	g_object_set(G_OBJECT(prof), "port", i, NULL);

	tmp = "server";
	g_object_set(G_OBJECT(prof), "servername", tmp, NULL);

	tmp = "user";
	g_object_set(G_OBJECT(prof), "username", tmp, NULL);
	
	loqui_profile_account_print(prof);

	g_object_get(G_OBJECT(prof), "servername", &tmp, NULL);
	g_return_val_if_fail(strcmp(tmp, "server") == 0, FALSE);
	g_free(tmp);

	g_object_get(G_OBJECT(prof), "username", &tmp, NULL);
	g_return_val_if_fail(strcmp(tmp, "user") == 0, FALSE);
	g_free(tmp);

	g_object_get(G_OBJECT(prof), "port", &i, NULL);
	g_return_val_if_fail(i == 9999, FALSE);
	
	g_object_unref(prof);

	return TRUE;
}

int
main()
{
	int all=0, failed=0;

	g_type_init();

	DO_TEST(all, failed, test_profile_account_make);
	DO_TEST(all, failed, test_profile_account_object);

	SHOW_RESULT_AND_EXIT(all, failed, utils);
}

