LDFLAGS = `pkg-config --libs glib-2.0 gnet-2.0` ../libloqui/libloqui.la
# FIXME: -I../libloqui should not needed.
CFLAGS = -I../ -I../src/ -I../libloqui/ `pkg-config --cflags glib-2.0 gnet-2.0` -g -Wall
FLAGS = $(LDFLAGS) $(CFLAGS)

TARGET := test_ipmsg test_utils test_profile_account test_profile_handle test_user test_title_format test_message test_string_tokenizer test_channel_entry

%: %.c
	libtool --mode=link gcc -o $@ $(FLAGS) $(OBJS) $@.c

all: $(TARGET)

.PHONY: report
report: $(TARGET)
	./$<

clean:
	rm -f $(TARGET)
