include(loqui-am-macros.m4)dnl

SUBDIRS = icons embedtxt

%.c %.h %-private.h: %.gob
	gob2 --always-private-header $<

bin_PROGRAMS = loqui

INCLUDES = -I$(includedir) -I$(top_srcdir)/libloqui -I$(top_builddir)/libloqui -I$(top_srcdir) -I$(top_builddir) -DG_LOG_DOMAIN=\"Loqui\"
CPPFLAGS =  $(GTK_CFLAGS) $(EXTRA_WARNING_CFLAGS) -g -DDATADIR=\""$(datadir)"\" $(GNET_CFLAGS) -Wall -Wredundant-decls -Wmissing-declarations -Wmissing-prototypes

define(`M4_SRC_GOB',`loqui-core-gtk.gob loqui-style-entry.gob loqui-account-dialog.gob loqui-protocol-selection-dialog.gob loqui-transfer-window.gob loqui-tray-icon.gob loqui-channel-entry-action-group.gob loqui-channel-entry-ui-data.gob loqui-channel-entry-action-group-ui.gob')dnl

loqui_SOURCES = \
	eggtrayicon.c eggtrayicon.h \
	gtkutils.c gtkutils.h \
	main.c main.h \
	loqui_app_actions.c loqui_app_actions.h \
	loqui_stock.c loqui_stock.h \
	loqui_app.c loqui_app.h \
	loqui_app_info.c loqui_app_info.h \
	loqui_channel_entry_store.c loqui_channel_entry_store.h \
	loqui_channel_entry_action.c loqui_channel_entry_action.h \
	loqui_channel_buffer_gtk.c loqui_channel_buffer_gtk.h \
	loqui_dropdown_box.c loqui_dropdown_box.h \
	loqui_account_manager_store.c loqui_account_manager_store.h \
	channel_tree.c channel_tree.h \
	nick_list.c nick_list.h \
	about.c about.h \
	account_list_dialog.c account_list_dialog.h \
	loqui_statusbar.c loqui_statusbar.h \
	prefs_dialog.c prefs_dialog.h \
	loqui_channel_text_view.c loqui_channel_text_view.h \
	command_dialog.c command_dialog.h \
	remark_entry.c remark_entry.h \
	loqui_channelbar.c loqui_channelbar.h \
	loqui_select_dialog.c loqui_select_dialog.h \
	prefs_general_upgrader.c prefs_general_upgrader.h \
	loqui-general-pref-gtk.h loqui-general-pref-gtk-groups.h loqui-general-pref-gtk-default.h \
	$(M4_SRC_GOB) gob_to_built_sources(M4_SRC_GOB)

BUILT_SOURCES := \
	gob_to_built_sources(M4_SRC_GOB)

loqui_LDADD = \
        $(GTK_LIBS) $(GNET_LIBS) ../libloqui/libloqui.la
