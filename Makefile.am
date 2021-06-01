SUBDIRS =  m4 libloqui src po # test

AUTOMAKE_OPTIONS = 1.5

ChangeLog:
	git log --name-status > $@

EXTRA_DIST= autogen.sh \
	AUTHORS \
	README \
	NEWS \
	NEWS.ja \
	COPYING \
	COPYING.LIB \
	LICENSE \
	ChangeLog \
	INSTALL \
	loqui.spec \
	loqui.spec.in \
	loqui.desktop \
	loqui.png \
	loqui-am-macros.m4

ACLOCAL_AMFLAGS = -I m4

icon_DATA = loqui.png
icondir = $(datadir)/icons/hicolor/48x48/apps

desktop_DATA = loqui.desktop
desktopdir = $(datadir)/applications/

dist-spec: distdir
	$(AMTAR) chof - $(distdir) | GZIP=$(GZIP_ENV) gzip -c >$(distdir).tar.gz
	sed '/^Source:/ s/\.tar\.gz$$/\.tar\.bz2/' $(distdir)/loqui.spec > $(distdir)/loqui.spec.bzip2
	mv $(distdir)/loqui.spec.bzip2 $(distdir)/loqui.spec
	$(AMTAR) chof - $(distdir) | bzip2 -9 -c >$(distdir).tar.bz2
	$(am__remove_distdir)
