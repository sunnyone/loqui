EMBEDTXT_ID = loqui_app_ui loqui_transfer_window_ui
EMBEDTXT = $(EMBEDTXT_ID:%=%.txt)
EMBEDTXT_H = $(EMBEDTXT_ID:%=%.h)

noinst_HEADERS = $(EMBEDTXT_H)

%.h: %.txt
	sed -e 's/"/\\"/g; s/^/  "/g; s/$$/\\n"/g; 1 s/^/static const gchar embedtxt_$(@:%.h=%)[] = /; $$ s/$$/;/;' $< > $@
