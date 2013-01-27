include(loqui-am-macros.m4)dnl

INCLUDES = $(GLIB_CFLAGS) $(GNET_CFLAGS) $(EXTRA_WARNING_CFLAGS) -I$(includedir) -I$(top_srcdir) -DG_LOG_DOMAIN=\"libloqui\"

AM_CFLAGS = -g -Wall -Wredundant-decls -Wmissing-declarations -Wmissing-prototypes

%.c %.h %-private.h: %.gob
	gob2 --always-private-header $<

noinst_LTLIBRARIES = libloqui_jabber.la

define(`M4_SOURCES_GOB', `loqui-account-jabber.gob')

BUILT_SOURCES := \
	gob_to_built_sources(M4_SOURCES_GOB)

libloqui_jabber_la_SOURCES = \
	M4_SOURCES_GOB \
	gob_to_built_sources(M4_SOURCES_GOB)

libloqui_jabber_la_LIBADD = \
	$(top_builddir)/libloqui/libloqui.la

