SUBDIRS = m4 src po

SCHEMAS_FILE = loqui.schemas
SCHEMAS_DIR = $(sysconfdir)/gconf/schemas

schemasdir = $(SCHEMAS_DIR)
schemas_DATA = $(SCHEMAS_FILE)

install-schemas: loqui.schemas
        GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) gconftool-2 --makefile
-install-rule $(SCHEMAS_FILE)

if INSTALL_SCHEMAS
install-data-local: install-schemas
endif

EXTRA_DIST= autogen.sh \
        intltool-merge.in \
        intltool-update.in \
        intltool-extract.in \
	$(schema_DATA) \
	README \
	NEWS \
	ChangeLog \
	COPYING \
	ABOUT-NLS \
	INSTALL

ACLOCAL_AMFLAGS = -I m4

.PHONY: deb

deb:
        if [ -d debian ]; then fakeroot $(MAKE) -f ./debian/rules clean; fi
        if [ -d debian ]; then fakeroot $(MAKE) -f ./debian/rules binary; fi

