bin_PROGRAMS = loqui

INCLUDES = $(LOQUI_CFLAGS) $(EXTRA_WARNING_CFLAGS) -g -I$(includedir) \
	-DDATADIR=\""$(datadir)"\" $(GNET_CFLAGS) -Wall

loqui_SOURCES = \
	main.c main.h \
	loqui_menu.c loqui_menu.h \
	loqui_app.c loqui_app.h \
	loqui_common_text.c loqui_common_text.h \
	connection.c connection.h \
	eel-gconf-extensions.c eel-gconf-extensions.h \
	loqui_gconf.h \
	account.c account.h \
	codeconv.c codeconv.h \
	channel_tree.c channel_tree.h \
	gtkutils.c gtkutils.h \
	utils.c utils.h \
	channel_book.c channel_book.h \
	channel_text.c channel_text.h \
	channel.c channel.h \
	nick_list.c nick_list.h \
	irc_message.c irc_message.h \
	irc_handle.c irc_handle.h \
	irc_constants.h \
	command_table.c command_table.h \
	about.c about.h

loqui_LDADD = \
        $(LOQUI_LIBS) $(GNET_LIBS)
