SUBDIRS =  m4 libloqui src po # test

AUTOMAKE_OPTIONS = 1.5

.PHONY: ChangeLog
ChangeLog:
	svn log -v > $@

EXTRA_DIST= autogen.sh \
	README \
	NEWS \
	NEWS.ja \
	ChangeLog \
	COPYING \
	ABOUT-NLS \
	INSTALL \
	loqui.spec \
	loqui.spec.in \
	loqui.desktop \
	loqui.png

ACLOCAL_AMFLAGS = -I m4

icon_DATA = loqui.png
icondir = $(datadir)/pixmaps

desktop_DATA = loqui.desktop
desktopdir = $(sysconfdir)/X11/applnk/Internet

