if MAINTAINER_MODE
Makefile.am: %: %.m4
	m4 -I$(top_srcdir) $< > $@
endif MAINTAINER_MODE

define(`gob_to_built_sources',`patsubst(`$@', `\.gob\>', `.c') \
		    patsubst(`$@', `\.gob\>', `.h') \
		    patsubst(`$@', `\.gob\>', `-private.h')')dnl
