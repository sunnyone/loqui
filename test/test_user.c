#include <config.h>

#include "test_helper.h"
#include "loqui_user.h"

static int
test_user_create(void)
{
	LoquiUser *user;
	gchar *nick;

	user = loqui_user_new();
	g_object_set(user, "nick", "hoge", NULL);
	g_object_get(user, "nick", &nick, NULL);

	g_return_val_if_fail(strcmp("hoge", nick) == 0, FALSE);

	g_print("nick: %s\n", nick);

	return TRUE;
}

int
main()
{
	int all=0, failed=0;

	g_type_init();

	DO_TEST(all, failed, test_user_create);

	SHOW_RESULT_AND_EXIT(all, failed, utils);
}

