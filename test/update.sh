#!/usr/bin/env bash
if [ ! -d orig_src ]; then
  mkdir orig_src
fi
rsync -au --exclude=main.c $(perl -e 'undef($/); $_=<>; s/\\\n//mgs; @src = grep { /^(SRC_BASE_UTILITIES|SRC_PROTOCOL_BASE_BASE|SRC_PROTOCOL_BASE|SRC_LIBRARY_CORE) = / } split("\n"); @csrc = grep { /\.c$/ } split(/\s+/, $src[0]); print join("\n", @csrc)' ../src/Makefile.am | sed -e 's|^|../src/|') orig_src 

cd orig_src
cat > Makefile <<EOF
SOURCES := \$(wildcard *.c)
OBJS := \$(SOURCES:%.c=%.o)
%.o: %.c
	gcc -g -c \$< \`pkg-config --cflags gtk+-2.0 gnet-2.0\` -I../../ -I../../src/ -I../../gtk24backports/ -DUSE_GTK_2_2

all: \$(OBJS)
EOF
make
cd ..
make clean

