EMBEDTXT_ID = loqui_app_ui
EMBEDTXT = $(EMBEDTXT_ID:%=%.txt)
EMBEDTXT_H = $(EMBEDTXT_ID:%=%.h)

noinst_DATA = $(EMBEDTXT_H)
CLEANFILES = $(noinst_DATA)

%.h: %.txt
	sed -e 's/"/\\"/g; s/^/  "/g; s/$$/\\n"/g; 1 s/^/static const gchar embedtxt_$(@:%.h=%)[] = /; $$ s/$$/;/;' $< > $@

EXTRA_DIST = $(EMBEDTXT)
