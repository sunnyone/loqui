SUBDIRS = icons embedtxt

bin_PROGRAMS = loqui

INCLUDES = -I$(includedir) -I../libloqui
CPPFLAGS =  $(GTK_CFLAGS) $(EXTRA_WARNING_CFLAGS) -g -DDATADIR=\""$(datadir)"\" $(GNET_CFLAGS) -Wall -Wredundant-decls -Wmissing-declarations -Wmissing-prototypes

loqui_SOURCES = \
	eggtrayicon.c eggtrayicon.h \
	gtkutils.c gtkutils.h \
	main.c main.h \
	loqui_gtk.c loqui_gtk.h \
	loqui_app_actions.c loqui_app_actions.h \
	loqui_stock.c loqui_stock.h \
	loqui_tray_icon.c loqui_tray_icon.h \
	loqui_app.c loqui_app.h \
	loqui_app_info.c loqui_app_info.h \
	loqui_channel_entry_store.c loqui_channel_entry_store.h \
	loqui_channel_entry_action.c loqui_channel_entry_action.h \
	loqui_channel_entry_ui.c loqui_channel_entry_ui.h \
	loqui_channel_buffer_gtk.c loqui_channel_buffer_gtk.h \
	loqui_dropdown_box.c loqui_dropdown_box.h \
	loqui_account_manager_store.c loqui_account_manager_store.h \
	loqui_transfer_window.c loqui_transfer_window.h \
	channel_tree.c channel_tree.h \
	nick_list.c nick_list.h \
	about.c about.h \
	account_list_dialog.c account_list_dialog.h \
	account_dialog.h account_dialog.c \
	loqui_statusbar.c loqui_statusbar.h \
	prefs_dialog.c prefs_dialog.h \
	loqui_channel_text_view.c loqui_channel_text_view.h \
	command_dialog.c command_dialog.h \
	remark_entry.c remark_entry.h \
	loqui_channelbar.c loqui_channelbar.h \
	loqui_select_dialog.c loqui_select_dialog.h \
	loqui_style_entry.c loqui_style_entry.h

loqui_LDADD = \
        $(GTK_LIBS) $(GNET_LIBS) ../libloqui/libloqui.la
