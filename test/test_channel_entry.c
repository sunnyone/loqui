#include "test_helper.h"
#include "loqui_channel_entry.h"

static int
test_channel_entry_add_and_remove(void)
{
	LoquiChannelEntry *chent;
	LoquiMember *member;
	LoquiUser *user;

	chent = loqui_channel_entry_new();
	user = loqui_user_new();
	member = loqui_member_new(user);

	loqui_channel_entry_add_member(chent, member);
	loqui_channel_entry_remove_member_by_user(chent, user);
	g_object_unref(user);

	g_return_val_if_fail(G_OBJECT(member)->ref_count == 1, FALSE);
	g_object_unref(member);
	g_object_unref(chent);

	return TRUE;
}

int
main()
{
	int all=0, failed=0;

	g_type_init();

	DO_TEST(all, failed, test_channel_entry_add_and_remove);

	SHOW_RESULT_AND_EXIT(all, failed, utils);
}

