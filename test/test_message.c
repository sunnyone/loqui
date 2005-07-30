#include "test_helper.h"
#include "loqui_message.h"

static int
test_message_join(void)
{
	LoquiMessage *msg;
	gchar *str;

	msg = loqui_message_new("join-channel");
	g_return_val_if_fail(msg != NULL, FALSE);

	loqui_message_set_attribute(msg, "channel-name", "#hoge", NULL);
	loqui_message_get_attribute(msg, "channel-name", &str, NULL);
	g_print("got: %s\n", str);

	g_object_unref(msg);

	return TRUE;
}

int
main()
{
	int all=0, failed=0;

	g_type_init();

	DO_TEST(all, failed, test_message_join);

	SHOW_RESULT_AND_EXIT(all, failed, utils);
}

