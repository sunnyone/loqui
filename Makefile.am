SUBDIRS =  m4 gtk24backports src po # test

.PHONY: ChangeLog
ChangeLog:
	svn log -v > $@

EXTRA_DIST= autogen.sh \
        intltool-merge.in \
        intltool-update.in \
        intltool-extract.in \
	README \
	NEWS \
	ChangeLog \
	COPYING \
	ABOUT-NLS \
	INSTALL \
	loqui.spec \
	loqui.spec.in

ACLOCAL_AMFLAGS = -I m4
