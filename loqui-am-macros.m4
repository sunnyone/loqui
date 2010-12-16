if MAINTAINER_MODE
Makefile.am: %: %.m4 $(top_srcdir)/loqui-am-macros.m4
	m4 -I$(top_srcdir) $< > $@
endif MAINTAINER_MODE

show-needless-svn-ignore: all-am
	@svn propget svn:ignore . | while read line; do \
	  [ ! -z "$$(find . -maxdepth 1 -name "$$line" -print)" ] || \
	    echo "$$line"; \
	done

add-built-sources-to-bzr-ignore:
	@set -e;tmpname=$$(mktemp -t bzr-ignore-temp.XXXXXX) && ( \
	  ((cat "$(top_srcdir)/.bzrignore"; \
	    echo -n "$(BUILT_SOURCES)" | tr " " "\n" | sed -e "s|^|$(subdir)/|") | \
	   sort | uniq > "$$tmpname") && \
	  if cmp -s "$(top_srcdir)/.bzrignore" "$$tmpname"; then \
	    rm "$$tmpname"; \
	  else \
	    mv "$$tmpname" "$(top_srcdir)/.bzrignore"; \
	  fi; \
	)

define(`gob_to_built_sources',`patsubst(`$@', `\.gob\>', `.c') \
		    patsubst(`$@', `\.gob\>', `.h') \
		    patsubst(`$@', `\.gob\>', `-private.h')')dnl
