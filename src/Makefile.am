SUBDIRS = icons embedtxt

bin_PROGRAMS = loqui

INCLUDES = $(GTK_CFLAGS) $(EXTRA_WARNING_CFLAGS) -g -I$(includedir) -I../gtk24backports \
	-DDATADIR=\""$(datadir)"\" $(GNET_CFLAGS) -Wall -Wredundant-decls -Wmissing-declarations -Wmissing-prototypes

loqui_SOURCES = \
	intl.h \
	utils.c utils.h \
	gobject_utils.c gobject_utils.h \
	loqui_utils_irc.c loqui_utils_irc.h \
	loqui_marshalers.c loqui_marshalers.h \
	loqui_title_format.c loqui_title_format.h \
	loqui_user.c loqui_user.h \
	loqui_user_irc.c loqui_user_irc.h \
	loqui_member.c loqui_member.h \
	loqui_profile_account.c loqui_profile_account.h \
	loqui_profile_account_irc.c loqui_profile_account_irc.h \
	loqui_channel_entry.c loqui_channel_entry.h \
	loqui_channel.c loqui_channel.h \
	loqui_profile_handle.c loqui_profile_handle.h \
	prefs_general.c prefs_general.h \
	main.c main.h \
	loqui_gtk.c loqui_gtk.h \
	loqui_app_actions.c loqui_app_actions.h \
	account_manager.c account_manager.h \
	loqui_stock.c loqui_stock.h \
	loqui_app.c loqui_app.h \
	loqui_channel_entry_store.c loqui_channel_entry_store.h \
	loqui_channel_entry_action.c loqui_channel_entry_action.h \
	loqui_channel_entry_ui.c loqui_channel_entry_ui.h \
	account.c account.h \
	codeconv.c codeconv.h \
	loqui_dropdown_box.c loqui_dropdown_box.h \
	channel_tree.c channel_tree.h \
	gtkutils.c gtkutils.h \
	channel_buffer.c channel_buffer.h \
	nick_list.c nick_list.h \
	irc_message.c irc_message.h \
	ctcp_message.c ctcp_message.h \
	irc_connection.c irc_connection.h \
	irc_handle.c irc_handle.h \
	ctcp_handle.c ctcp_handle.h \
	irc_constants.h \
	command_table.c command_table.h \
	about.c about.h \
	account_list_dialog.c account_list_dialog.h \
	account_dialog.h account_dialog.c \
	loqui_statusbar.c loqui_statusbar.h \
	prefs_dialog.c prefs_dialog.h \
	channel_input_dialog.c channel_input_dialog.h \
	command_dialog.c command_dialog.h \
	remark_entry.c remark_entry.h \
	message_text.c message_text.h \
	loqui_channelbar.c loqui_channelbar.h

loqui_LDADD = \
        $(GTK_LIBS) $(GNET_LIBS) ../gtk24backports/libgtk24backports.la

BUILT_SOURCES = \
	loqui_marshalers.c loqui_marshalers.h

loqui_marshalers.h : loqui_marshalers.list $(GLIB_GENMARSHAL)
	$(GLIB_GENMARSHAL) $< --header --prefix=_loqui_marshal > $@

loqui_marshalers.c : loqui_marshalers.list $(GLIB_GENMARSHAL)
	$(GLIB_GENMARSHAL) $< --body --prefix=_loqui_marshal > $@

EXTRA_DIST= \
	loqui_marshalers.list
