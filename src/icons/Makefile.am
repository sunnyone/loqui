IMAGES_ID = console online offline away busy command
IMAGES = $(IMAGES_ID:%=%.png)
IMAGES_H = $(IMAGES_ID:%=%.h)

noinst_DATA = pixbufs.h $(IMAGES_H)
CLEANFILES = $(noinst_DATA)

pixbufs.h: $(IMAGES_H)
	cat $(IMAGES_H) > $(srcdir)/pixbufs.h

%.h: %.png
	gdk-pixbuf-csource --raw --build-list $(@:%.h=%)_pixbuf $< > $@

EXTRA_DIST = $(IMAGES) pixbufs.h
