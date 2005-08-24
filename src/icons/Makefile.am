IMAGES_ID = console online offline away busy command naruto speaker loqui loqui_hilighted whether_scroll notice
IMAGES = $(IMAGES_ID:%=%.png) loqui.ico
IMAGES_H = $(IMAGES_ID:%=%.h)

noinst_HEADERS = pixbufs.h $(IMAGES_H)
EXTRA_DIST = $(IMAGES)

pixbufs.h: $(IMAGES_H)
	cat $(IMAGES_H) > $@

%.h: %.png
	gdk-pixbuf-csource --raw --build-list $(@:%.h=%)_pixbuf $< > $@

