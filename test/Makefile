ORIG_OBJS := $(wildcard orig_src/*.o)
LDFLAGS = `pkg-config --libs gtk+-2.0 gnet-2.0`
CPPFLAGS = -I../ -I../src/ `pkg-config --cflags gtk+-2.0 gnet-2.0` -g -Wall
FLAGS = $(LDFLAGS) $(CPPFLAGS)

TARGET := test_utils

test_utils: test_utils.c
	gcc -o $@ $(FLAGS) $(OBJS) $@.c

all: $(TARGET)

.PHONY: report
report: $(TARGET)
	./$<

clean:
	rm -rf orig_src
