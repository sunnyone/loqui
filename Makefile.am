SUBDIRS = m4  src po

schemadir = $(sysconfdir)/gconf/schemas
schema_DATA = loqui.schemas

install-schemas: $(schema_DATA)
        GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) gconftool-2 --makefile-install-rule $(schema_DATA)

if INSTALL_SCHEMAS
install-data-local: install-schemas
endif

EXTRA_DIST= autogen.sh \
        intltool-merge.in \
        intltool-update.in \
        intltool-extract.in \
	$(schema_DATA)

ACLOCAL_AMFLAGS = -I m4

