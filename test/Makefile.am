ORIG_SRC = \
	../src/intl.h \
	 ../src/main.h \
	../src/gobject_utils.c ../src/gobject_utils.h \
	../src/prefs_general.c ../src/prefs_general.h \
	../src/prefs_account.c ../src/prefs_account.h \
	../src/loqui_menu.c ../src/loqui_menu.h \
	../src/account_manager.c ../src/account_manager.h \
	../src/loqui_app.c ../src/loqui_app.h \
	../src/account.c ../src/account.h \
	../src/codeconv.c ../src/codeconv.h \
	../src/channel_tree.c ../src/channel_tree.h \
	../src/gtkutils.c ../src/gtkutils.h \
	../src/utils.c ../src/utils.h \
	../src/channel_buffer.c ../src/channel_buffer.h \
	../src/channel.c ../src/channel.h \
	../src/nick_list.c ../src/nick_list.h \
	../src/irc_message.c ../src/irc_message.h \
	../src/ctcp_message.c ../src/ctcp_message.h \
	../src/irc_handle.c ../src/irc_handle.h \
	../src/ctcp_handle.c ../src/ctcp_handle.h \
	../src/irc_constants.h \
	../src/command_table.c ../src/command_table.h \
	../src/about.c ../src/about.h \
	../src/account_list_dialog.c ../src/account_list_dialog.h \
	../src/account_dialog.h ../src/account_dialog.c \
	../src/loqui_toolbar.c ../src/loqui_toolbar.h \
	../src/prefs_dialog.c ../src/prefs_dialog.h \
	../src/connect_dialog.c ../src/connect_dialog.h \
	../src/channel_input_dialog.c ../src/channel_input_dialog.h \
	../src/command_dialog.c ../src/command_dialog.h \
	../src/remark_entry.c ../src/remark_entry.h \
	../src/message_text.c ../src/message_text.h \
	../src/buffer_menu.c ../src/buffer_menu.h \
	../src/loqui_channelbar.c ../src/loqui_channelbar.h


bin_PROGRAMS = test_loqui

INCLUDES = $(GTK_CFLAGS) $(EXTRA_WARNING_CFLAGS) -g -I$(includedir) \
        -DDATADIR=\""$(datadir)"\" $(GNET_CFLAGS) -Wall -I../src

test_loqui_SOURCES = $(ORIG_SRC) test_main.c \
		      test_utils.c

test_loqui_LDADD = \
        $(GTK_LIBS) $(GNET_LIBS)

EXTRA_DIST = Makefile.am.after update-makefile.sh

