INCLUDES = $(GLIB_CFLAGS) $(GNET_CFLAGS) $(EXTRA_WARNING_CFLAGS) -I$(includedir)
CPPFLAGS = -g -Wall -Wredundant-decls -Wmissing-declarations -Wmissing-prototypes

%.c %.h %-private.h: %.gob
	gob2 --always-private-header $<

SRC_BASE_UTILITIES = \
	intl.h \
	utils.c utils.h \
	loqui_webutils.c loqui_webutils.h \
	gobject_utils.c gobject_utils.h \
	loqui_string_tokenizer.c loqui_string_tokenizer.h \
	loqui_title_format.c loqui_title_format.h \
	loqui_marshalers.c loqui_marshalers.h \
	lqgkeyfile.c lqgkeyfile.h

SRC_PROTOCOL_BASE_BASE = \
	loqui_channel_entry.c loqui_channel_entry.h \
	loqui_codeconv_tools.c loqui_codeconv_tools.h \
	loqui_codeconv.c loqui_codeconv.h \
	loqui_message_text.c loqui_message_text.h \
	loqui_channel_buffer.c loqui_channel_buffer.h \
	loqui_channel_entry_utils.c loqui_channel_entry_utils.h

SRC_PROTOCOL_BASE = \
	loqui_protocol.c loqui_protocol.h \
	loqui_user.c loqui_user.h \
	loqui_profile_account.c loqui_profile_account.h \
	loqui_channel.c loqui_channel.h \
	loqui_sender.c loqui_sender.h \
	loqui_receiver.c loqui_receiver.h \
	loqui_account.c loqui_account.h \
	loqui_message.c loqui_message.h \
	loqui_member.c loqui_member.h \
	loqui_transfer_item.c loqui_transfer_item.h

SRC_PROTOCOL_IRC = \
	loqui_protocol_irc.c loqui_protocol_irc.h \
	loqui_user_irc.c loqui_user_irc.h \
	loqui_utils_irc.c loqui_utils_irc.h \
	loqui_channel_irc.c loqui_channel_irc.h \
	loqui_profile_account_irc.c loqui_profile_account_irc.h \
	loqui_sender_irc.c loqui_sender_irc.h \
	loqui_receiver_irc.c loqui_receiver_irc.h \
	loqui_account_irc.c loqui_account_irc.h \
	irc_constants.h \
	irc_message.c irc_message.h \
	ctcp_message.c ctcp_message.h \
	ctcp_handle.c ctcp_handle.h

SRC_PROTOCOL_IPMSG = \
	loqui_protocol_ipmsg.c loqui_protocol_ipmsg.h \
	loqui_user_ipmsg.c loqui_user_ipmsg.h \
	loqui_profile_account_ipmsg.c loqui_profile_account_ipmsg.h \
	loqui_sender_ipmsg.c loqui_sender_ipmsg.h \
	loqui_receiver_ipmsg.c loqui_receiver_ipmsg.h \
	loqui_account_ipmsg.c loqui_account_ipmsg.h \
	ipmsg.h \
	ipmsg_socket.c ipmsg_socket.h \
	ipmsg_packet.c ipmsg_packet.h

SRC_PROTOCOL_MSN = \
	msn_login.c msn_login.h \
	msn_message.c msn_message.h \
	loqui_protocol_msn.c loqui_protocol_msn.h \
	loqui_account_msn.c loqui_account_msn.h \
	msn_constants.h

SRC_LIBRARY_CORE = \
	loqui_protocol_manager.c loqui_protocol_manager.h \
	loqui_member_sort_funcs.c loqui_member_sort_funcs.h \
	loqui_profile_handle.c loqui_profile_handle.h \
	prefs_general.c prefs_general.h \
	loqui-general-pref-groups.h loqui-general-pref-default.h \
	loqui_account_manager.c loqui_account_manager.h \
	loqui_account_manager_iter.c loqui_account_manager_iter.h \
	loqui-core.gob loqui-core.c loqui-core.h loqui-core-private.h \
	loqui-pref.gob loqui-pref.c loqui-pref.h loqui-pref-private.h \
	loqui-static-core.c loqui-static-core.h \
	loqui.h

BUILT_SOURCES := loqui_marshalers.c loqui_marshalers.h \
	loqui-core.c loqui-core.h loqui-core-private.h \
	loqui-pref.c loqui-pref.h loqui-pref-private.h

lib_LTLIBRARIES = libloqui.la

libloqui_la_SOURCES =  \
	$(SRC_BASE_UTILITIES) \
	$(SRC_PROTOCOL_BASE_BASE) \
	$(SRC_PROTOCOL_BASE) \
	$(SRC_PROTOCOL_IRC) \
	$(SRC_PROTOCOL_IPMSG) \
	$(SRC_PROTOCOL_MSN) \
	$(SRC_LIBRARY_CORE)

libloqui_la_LIBADD = $(GLIB_LIBS) $(GNET_LIBS)

loqui_marshalers.h : loqui_marshalers.list $(GLIB_GENMARSHAL)
	$(GLIB_GENMARSHAL) $< --header --prefix=_loqui_marshal > $@

loqui_marshalers.c : loqui_marshalers.list $(GLIB_GENMARSHAL)
	$(GLIB_GENMARSHAL) $< --body --prefix=_loqui_marshal > $@

EXTRA_DIST = \
	loqui_marshalers.list
