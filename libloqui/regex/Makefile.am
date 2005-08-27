SUBDIRS = pcre

INCLUDES = \
  $(GLIB_CFLAGS) \
  -DG_DISABLE_DEPRECATED

noinst_LTLIBRARIES = libeggregex.la

libeggregex_la_LIBADD = \
	pcre/libpcre.la

libeggregex_la_SOURCES = \
  eggregex.c eggregex.h

# nobase_include_HEADERS = \
noinst_HEADERS = \
  eggregex.h
