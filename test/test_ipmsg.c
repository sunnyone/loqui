#include <libloqui/loqui.h>
#include <libloqui/loqui_account.h>
#include <libloqui/loqui-account-ipmsg.h>
#include <libloqui/loqui-sender-ipmsg.h>
#include <libloqui/loqui_protocol_ipmsg.h>
#include <locale.h>

void connected_cb(LoquiAccount *account, GParamSpec *pspec, gpointer data)
{
	g_print("Connected.\n");
}
void add_cb(LoquiChannelEntry *chent, LoquiMember *member)
{
	g_print("Added!\n");

	loqui_sender_ipmsg_getinfo(LOQUI_SENDER_IPMSG(loqui_account_get_sender(LOQUI_ACCOUNT(chent))));
}

int
main(int argc, char *argv[])
{
	LoquiCore *core;
	LoquiAccount *account;
	LoquiProfileAccount *profile;
	LoquiProtocol *proto;

	g_type_init();

	setlocale(LC_ALL, "");
	core = loqui_core_new();
	loqui_init(core);
	loqui_core_initialize(core);
	loqui_core_set_show_msg_mode(core, TRUE);

	proto = loqui_protocol_ipmsg_get();
	profile = loqui_protocol_create_profile_account(proto);
	g_object_set(profile, "nick", "test", "username", "user", NULL);
	account = LOQUI_ACCOUNT(loqui_account_ipmsg_new(profile));

	g_signal_connect(account, "notify::is-connected", G_CALLBACK(connected_cb), NULL);
	g_signal_connect(account, "add", G_CALLBACK(add_cb), NULL);

	loqui_account_connect(account);
	g_main_loop_run(g_main_loop_new(NULL, TRUE));

	g_object_unref(account);
	g_object_unref(core);

	return 0;
}
