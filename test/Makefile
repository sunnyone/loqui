ORIG_OBJS := $(wildcard orig_src/*.o)
LDFLAGS = `pkg-config --libs gtk+-2.0 gnet-2.0` ../gtk24backports/libgtk24backports.la
CFLAGS = -I../ -I../src/ -I../gtk24backports `pkg-config --cflags gtk+-2.0 gnet-2.0` -g -Wall -DUSE_GTK_2_2
FLAGS = $(LDFLAGS) $(CFLAGS) $(ORIG_OBJS)

TARGET := test_utils test_profile_account test_profile_handle test_user test_title_format test_message test_string_tokenizer

%: %.c
	libtool --mode=link gcc -o $@ $(FLAGS) $(OBJS) $@.c

all: $(TARGET)

.PHONY: report
report: $(TARGET)
	./$<

clean:
	rm -f $(TARGET)

distclean: clean
	rm -rf orig_src

