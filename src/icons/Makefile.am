IMAGES_ID = console online offline
IMAGES = $(IMAGES_ID:%=%.png)

noinst_DATA = pixbufs.h
CLEANFILES = $(noinst_DATA)

pixbufs.h: $(IMAGES)
	rm -f $(srcdir)/pixbufs.h
	touch $(srcdir)/pixbufs.h
	for I in $(IMAGES_ID); do \
	  gdk-pixbuf-csource --raw --build-list $${I}_pixbuf $(srcdir)/$$I.png >> $(srcdir)/pixbufs.h; \
	done

EXTRA_DIST = $(IMAGES) pixbufs.h
