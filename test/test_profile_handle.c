#include "test_helper.h"
#include <libloqui/loqui-profile-handle.h>
#include "loqui_profile_account_irc.h"
#include "loqui_protocol_irc.h"

static int
test_profile_handle_read_simple_one(void)
{
	LoquiProfileHandle *handle;
	LoquiProfileAccount *prof;
	GList *prof_list = NULL, *list;
	LoquiProtocolManager *protocol_manager;
	GList *factory_list;

	const gchar *xml = "<accounts><account type=\"IRC\"><param key=\"use\">TRUE</param><param key=\"name\">name</param><param key=\"nick\">hoge</param><param key=\"username\">user</param><param key=\"servername\">example.com</param><param key=\"port\">3323</param><param key=\"codeset-type\">2</param><param key=\"nick_list\"><list><item>hoge_away</item><item>hoge_zzz</item></list></param></account></accounts>";

	protocol_manager = loqui_protocol_manager_new();
		
	loqui_protocol_manager_register(protocol_manager, loqui_protocol_irc_get());
	factory_list = loqui_protocol_manager_get_protocol_list(protocol_manager);

	handle = loqui_profile_handle_new(factory_list);
	loqui_profile_handle_read_from_buffer(handle, &prof_list, xml);

	g_return_val_if_fail(g_list_length(prof_list) > 0, FALSE);
	prof = g_list_nth_data(prof_list, 0);
	g_list_free(prof_list);

	loqui_profile_account_print(prof);

	g_return_val_if_fail(loqui_profile_account_get_use(prof) == TRUE, FALSE);
	g_return_val_if_fail(strcmp(loqui_profile_account_get_nick(prof), "hoge") == 0, FALSE);
	g_return_val_if_fail(strcmp(loqui_profile_account_get_username(prof), "user") == 0, FALSE);
	g_return_val_if_fail(strcmp(loqui_profile_account_get_servername(prof), "example.com") == 0, FALSE);
	g_return_val_if_fail(loqui_profile_account_get_port(prof) == 3323, FALSE);
	g_return_val_if_fail(loqui_profile_account_irc_get_codeset_type(LOQUI_PROFILE_ACCOUNT_IRC(prof)) == 2, FALSE);

	list = loqui_profile_account_get_nick_list(prof);
	g_return_val_if_fail(g_list_length(list) == 2, FALSE);
	g_return_val_if_fail(strcmp(g_list_nth_data(list, 0), "hoge_away") == 0, FALSE);
	g_return_val_if_fail(strcmp(g_list_nth_data(list, 1), "hoge_zzz") == 0, FALSE);

/*	g_list_foreach(factory_list, (GFunc) g_object_unref, NULL); */
	g_list_free(factory_list);

	g_object_unref(prof);
	g_object_unref(handle);
	g_object_unref(protocol_manager);

	return TRUE;
}
static int
test_profile_handle_read_simple_two(void)
{
	LoquiProfileHandle *handle;
	LoquiProfileAccount *prof1, *prof2;
	GList *prof_list = NULL, *list;

	const gchar *xml = "<accounts><account type=\"IRC\"><param key=\"username\">user1</param><param key=\"nick_list\"><list><item>hoge1_away</item><item>hoge1_zzz</item></list></param></account><account type=\"IRC\"><param key=\"username\">user2</param><param key=\"nick_list\"><list><item>hoge2_away</item><item>hoge2_zzz</item></list></param></account></accounts>";
	
	handle = loqui_profile_handle_new();

	loqui_profile_handle_register_type(handle, "IRC", LOQUI_TYPE_PROFILE_ACCOUNT_IRC);
	loqui_profile_handle_read_from_buffer(handle, &prof_list, xml);

	g_return_val_if_fail(g_list_length(prof_list) > 0, FALSE);
	prof1 = g_list_nth_data(prof_list, 0);
	prof2 = g_list_nth_data(prof_list, 1);
	g_list_free(prof_list);

	g_return_val_if_fail(prof1 != NULL, FALSE);
	g_return_val_if_fail(prof2 != NULL, FALSE);

	loqui_profile_account_print(prof1);
	loqui_profile_account_print(prof2);

	g_return_val_if_fail(strcmp(loqui_profile_account_get_username(prof1), "user1") == 0, FALSE);
	g_return_val_if_fail(strcmp(loqui_profile_account_get_username(prof2), "user2") == 0, FALSE);

	list = loqui_profile_account_get_nick_list(prof1);
	g_return_val_if_fail(g_list_length(list) == 2, FALSE);
	g_return_val_if_fail(strcmp(g_list_nth_data(list, 0), "hoge1_away") == 0, FALSE);
	g_return_val_if_fail(strcmp(g_list_nth_data(list, 1), "hoge1_zzz") == 0, FALSE);

	list = loqui_profile_account_get_nick_list(prof2);
	g_return_val_if_fail(g_list_length(list) == 2, FALSE);
	g_return_val_if_fail(strcmp(g_list_nth_data(list, 0), "hoge2_away") == 0, FALSE);
	g_return_val_if_fail(strcmp(g_list_nth_data(list, 1), "hoge2_zzz") == 0, FALSE);

	g_object_unref(prof1);
	g_object_unref(prof2);
	g_object_unref(handle);

	return TRUE;
}

static int
test_profile_handle_write_simple_one(void)
{
	LoquiProfileHandle *handle;
	LoquiProfileAccount *prof;
	GList *prof_list = NULL, *list = NULL;
	gchar *buf;
	
	handle = loqui_profile_handle_new();
	loqui_profile_handle_register_type(handle, "IRC", LOQUI_TYPE_PROFILE_ACCOUNT_IRC);

	prof = LOQUI_PROFILE_ACCOUNT(loqui_profile_account_irc_new());
	loqui_profile_account_set_name(prof, "hoge");
	loqui_profile_account_set_nick(prof, "fuga");
	loqui_profile_account_set_port(prof, 6667);
	loqui_profile_account_irc_set_codeset_type(LOQUI_PROFILE_ACCOUNT_IRC(prof), 2);

	list = g_list_append(list, g_strdup("hoge_away"));
	list = g_list_append(list, g_strdup("hoge_zzz"));
	loqui_profile_account_set_nick_list(prof, list);

	prof_list = g_list_append(prof_list, prof);
	loqui_profile_handle_write_to_buffer(handle, prof_list, &buf);
	g_list_free(prof_list);

	g_print(buf);

	prof_list = NULL;
	loqui_profile_handle_read_from_buffer(handle, &prof_list, buf);
	g_free(buf);

	g_return_val_if_fail(strcmp(loqui_profile_account_get_name(prof), "hoge") == 0, FALSE);
	g_return_val_if_fail(strcmp(loqui_profile_account_get_nick(prof), "fuga") == 0, FALSE);
	g_return_val_if_fail(loqui_profile_account_get_port(prof) == 6667, FALSE);
	g_return_val_if_fail(loqui_profile_account_irc_get_codeset_type(LOQUI_PROFILE_ACCOUNT_IRC(prof)) == 2, FALSE);

	list = loqui_profile_account_get_nick_list(prof);
	g_return_val_if_fail(g_list_length(list) == 2, FALSE);
	g_return_val_if_fail(strcmp(g_list_nth_data(list, 0), "hoge_away") == 0, FALSE);
	g_return_val_if_fail(strcmp(g_list_nth_data(list, 1), "hoge_zzz") == 0, FALSE);
	g_list_free(list);

	return TRUE;
}
int
main()
{
	int all=0, failed=0;

	g_type_init();

	DO_TEST(all, failed, test_profile_handle_read_simple_one);
	DO_TEST(all, failed, test_profile_handle_read_simple_two);
	DO_TEST(all, failed, test_profile_handle_write_simple_one);

	SHOW_RESULT_AND_EXIT(all, failed, handle);
}

