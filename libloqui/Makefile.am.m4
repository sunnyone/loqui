include(loqui-am-macros.m4)dnl

INCLUDES = $(GLIB_CFLAGS) $(GNET_CFLAGS) $(EXTRA_WARNING_CFLAGS) -I$(includedir) -I$(top_srcdir) -DG_LOG_DOMAIN=\"libloqui\"
CPPFLAGS = -g -Wall -Wredundant-decls -Wmissing-declarations -Wmissing-prototypes

%.c %.h %-private.h: %.gob
	gob2 --always-private-header $<

SRC_BASE_UTILITIES = \
	loqui-utils.c loqui-utils.h \
	loqui_webutils.c loqui_webutils.h \
	loqui-gobject-utils.c loqui-gobject-utils.h \
	loqui_string_tokenizer.c loqui_string_tokenizer.h \
	loqui_title_format.c loqui_title_format.h \
	loqui_marshalers.c loqui_marshalers.h \
	lqgkeyfile.c lqgkeyfile.h

define(`M4_SRC_PROTOCOL_BASE_BASE_GOB',`loqui-generic-profile-factory.gob loqui-property-profile.gob loqui-message-text.gob')
SRC_PROTOCOL_BASE_BASE = \
	loqui_channel_entry.c loqui_channel_entry.h \
	loqui_codeconv_tools.c loqui_codeconv_tools.h \
	loqui_codeconv.c loqui_codeconv.h \
	loqui_channel_buffer.c loqui_channel_buffer.h \
	loqui_channel_entry_utils.c loqui_channel_entry_utils.h \
	loqui-profile.c loqui-profile.h \
	loqui-profile-factory.c loqui-profile-factory.h \
	M4_SRC_PROTOCOL_BASE_BASE_GOB gob_to_built_sources(M4_SRC_PROTOCOL_BASE_BASE_GOB)

define(`M4_SRC_PROTOCOL_BASE_GOB',`loqui-protocol.gob loqui-mode-item.gob loqui-mode-manager.gob loqui-transfer-item.gob')
SRC_PROTOCOL_BASE = \
	loqui_user.c loqui_user.h \
	loqui_profile_account.c loqui_profile_account.h \
	loqui_channel.c loqui_channel.h \
	loqui_sender.c loqui_sender.h \
	loqui_receiver.c loqui_receiver.h \
	loqui_account.c loqui_account.h \
	loqui_message.c loqui_message.h \
	loqui_member.c loqui_member.h \
	M4_SRC_PROTOCOL_BASE_GOB gob_to_built_sources(M4_SRC_PROTOCOL_BASE_GOB)

define(`M4_SRC_PROTOCOL_IRC_GOB',`loqui-transfer-item-irc.gob')
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
	ctcp_handle.c ctcp_handle.h \
	M4_SRC_PROTOCOL_IRC_GOB gob_to_built_sources(M4_SRC_PROTOCOL_IRC_GOB)
	

define(`M4_SRC_PROTOCOL_IPMSG_GOB',`loqui-account-ipmsg.gob loqui-receiver-ipmsg.gob loqui-sender-ipmsg.gob loqui-socket-ipmsg.gob')

SRC_PROTOCOL_IPMSG = \
	loqui_protocol_ipmsg.c loqui_protocol_ipmsg.h \
	loqui_user_ipmsg.c loqui_user_ipmsg.h \
	loqui-utils-ipmsg.c loqui-utils-ipmsg.h \
	loqui_profile_account_ipmsg.c loqui_profile_account_ipmsg.h \
	ipmsg.h \
	ipmsg_packet.c ipmsg_packet.h \
	M4_SRC_PROTOCOL_IPMSG_GOB gob_to_built_sources(M4_SRC_PROTOCOL_IPMSG_GOB)

SRC_PROTOCOL_MSN = \
	msn_login.c msn_login.h \
	msn_message.c msn_message.h \
	loqui_protocol_msn.c loqui_protocol_msn.h \
	loqui_account_msn.c loqui_account_msn.h \
	msn_constants.h

define(`M4_SRC_LIBRARY_CORE_GOB',`loqui-core.gob loqui-pref.gob loqui-pref-partial.gob loqui-pref-sequence.gob loqui-profile-handle.gob loqui-account-manager.gob')dnl

SRC_LIBRARY_CORE = \
	libloqui-intl.h \
	loqui_protocol_manager.c loqui_protocol_manager.h \
	loqui_member_sort_funcs.c loqui_member_sort_funcs.h \
	loqui-general-pref-groups.h loqui-general-pref-default.h \
	loqui_account_manager_iter.c loqui_account_manager_iter.h \
	loqui-static-core.c loqui-static-core.h \
	loqui.h \
	M4_SRC_LIBRARY_CORE_GOB \
	gob_to_built_sources(M4_SRC_LIBRARY_CORE_GOB)

BUILT_SOURCES := loqui_marshalers.c loqui_marshalers.h \
	gob_to_built_sources(M4_SRC_LIBRARY_CORE_GOB) \
	gob_to_built_sources(M4_SRC_PROTOCOL_BASE_BASE_GOB) \
	gob_to_built_sources(M4_SRC_PROTOCOL_BASE_GOB) \
	gob_to_built_sources(M4_SRC_PROTOCOL_IPMSG_GOB) \
	gob_to_built_sources(M4_SRC_PROTOCOL_IRC_GOB)

noinst_LTLIBRARIES = libloqui.la

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
