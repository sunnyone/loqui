SUBDIRS =  m4 gtk24backports src po # test

.PHONY: ChangeLog
ChangeLog:
	svn log -v > $@

EXTRA_DIST= autogen.sh \
	README \
	NEWS \
	ChangeLog \
	COPYING \
	ABOUT-NLS \
	INSTALL \
	loqui.spec \
	loqui.spec.in

ACLOCAL_AMFLAGS = -I m4
