# -*- Mode: Makefile; tab-width: 8; indent-tabs-mode: t; -*-

noinst_LTLIBRARIES = libgtk24backports.la

INCLUDES = \
	$(GTK_CFLAGS) \
	-I$(includedir)

CPPFLAGS= \
	-DGTK_DISABLE_DEPRECATED=1 \
	-DGDK_DISABLE_DEPRECATED=1 \
	-DG_DISABLE_DEPRECATED=1

libgtk24backports_la_SOURCES = \
	gtkaction.c gtkaction.h \
	gtkactiongroup.c gtkactiongroup.h \
	gtkuimanager.c gtkuimanager.h \
	gtkradioaction.c gtkradioaction.h \
        gtk24marshalers.c gtk24marshalers.h \
	gtkradiotoolbutton.c gtkradiotoolbutton.h \
	gtkseparatortoolitem.c gtkseparatortoolitem.h \
	gtktoggletoolbutton.c gtktoggletoolbutton.h gtktoggleactionprivate.h \
	gtktoggleaction.c gtktoggleaction.h \
	gtk24toolbar.c gtk24toolbar.h \
	gtktoolbutton.c gtktoolbutton.h \
	gtktoolitem.c gtktoolitem.h

BUILT_SOURCES = \
	gtk24marshalers.c gtk24marshalers.h

libgtk24backports_la_LIBADD = \
	$(GTK_LIBS)

gtk24marshalers.h : gtk24marshalers.list $(GLIB_GENMARSHAL)
	$(GLIB_GENMARSHAL) $< --header --prefix=_gtk24_marshal > $@

gtk24marshalers.c : gtk24marshalers.list $(GLIB_GENMARSHAL)
	$(GLIB_GENMARSHAL) $< --body --prefix=_gtk24_marshal > $@

EXTRA_DIST= \
	gtk24marshalers.list
