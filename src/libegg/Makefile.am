# -*- Mode: Makefile; tab-width: 8; indent-tabs-mode: t; -*-

noinst_LTLIBRARIES = libegg.la

INCLUDES = \
	$(GTK_CFLAGS) \
	-I$(includedir) \
	-I$(top_srcdir)/src/

CPPFLAGS= \
	-DGTK_DISABLE_DEPRECATED=1 \
	-DGDK_DISABLE_DEPRECATED=1 \
	-DG_DISABLE_DEPRECATED=1

libegg_la_SOURCES = \
	egg-action-group.c egg-action-group.h \
	egg-action.c egg-action.h \
        egg-entry-action.c egg-entry-action.h \
        egg-history-action.c egg-history-action.h \
	egg-markup.c egg-markup.h \
	egg-marshalers.c egg-marshalers.h \
	egg-menu-merge.c egg-menu-merge.h \
	egg-menu.h \
	egg-radio-action.c egg-radio-action.h \
	egg-radiotoolbutton.c egg-radiotoolbutton.h \
	egg-separatortoolitem.c egg-separatortoolitem.h \
	egg-toggletoolbutton.c egg-toggletoolbutton.h \
	egg-toggle-action.c egg-toggle-action.h \
	egg-toolbar.c egg-toolbar.h \
	egg-toolbutton.c egg-toolbutton.h \
	egg-toolitem.c egg-toolitem.h

BUILT_SOURCES = \
	egg-marshalers.c egg-marshalers.h

libegg_la_LIBADD = \
	$(GTK_LIBS)

egg-marshalers.h : egg-marshalers.list $(GLIB_GENMARSHAL)
	$(GLIB_GENMARSHAL) $< --header --prefix=_egg_marshal > $@

egg-marshalers.c : egg-marshalers.list $(GLIB_GENMARSHAL)
	$(GLIB_GENMARSHAL) $< --body --prefix=_egg_marshal > $@

EXTRA_DIST= \
	egg-marshalers.list
