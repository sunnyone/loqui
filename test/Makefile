ORIG_OBJS := $(wildcard orig_src/*.o)
LDFLAGS = `pkg-config --libs gtk+-2.0 gnet-2.0`
CFLAGS = -I../ -I../src/ `pkg-config --cflags gtk+-2.0 gnet-2.0` -g -Wall
FLAGS = $(LDFLAGS) $(CFLAGS) $(ORIG_OBJS)

TARGET := test_utils test_profile_account test_profile_handle

%: %.c
	gcc -o $@ $(FLAGS) $(OBJS) $@.c

all: $(TARGET)

.PHONY: report
report: $(TARGET)
	./$<

clean:
	rm -f $(TARGET)

distclean: clean
	rm -rf orig_src

