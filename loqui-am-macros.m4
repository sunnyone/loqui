if MAINTAINER_MODE
Makefile.am: %: %.m4 $(top_srcdir)/loqui-am-macros.m4
	m4 -I$(top_srcdir) $< > $@
endif MAINTAINER_MODE

show-needless-svn-ignore: all-am
	@svn propget svn:ignore . | while read line; do \
	  [ ! -z "$$(find . -maxdepth 1 -name "$$line" -print)" ] || \
	    echo "$$line"; \
	done

add-built-sources-to-svn-ignore:
	@set -e;tmpname=$$(mktemp -t svn-ignore-temp.XXXXXX) && ( \
	  ((svn propget --strict svn:ignore .; echo -n "$(BUILT_SOURCES)" | tr " " "\n") | \
	   sort | uniq > "$$tmpname") && \
	  svn propset svn:ignore -F "$$tmpname" . && \
	  rm "$$tmpname" \
	)

define(`gob_to_built_sources',`patsubst(`$@', `\.gob\>', `.c') \
		    patsubst(`$@', `\.gob\>', `.h') \
		    patsubst(`$@', `\.gob\>', `-private.h')')dnl
