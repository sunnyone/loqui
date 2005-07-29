#include <libloqui/loqui.h>
#include <libloqui/loqui_account.h>
#include <libloqui/loqui-account-ipmsg.h>

int
main(int argc, char *argv[])
{
	LoquiCore *core;
	LoquiAccount *account;
	LoquiProfileAccount *profile;

	g_type_init();

	core = loqui_core_new();
	profile = g_object_new(LOQUI_TYPE_PROFILE_ACCOUNT,
			       NULL);
	account = LOQUI_ACCOUNT(loqui_account_ipmsg_new(profile));

	loqui_account_connect(account);

	g_object_unref(account);
	g_object_unref(core);

	return 0;
}
