#!/usr/bin/env bash
if [ ! -d orig_src ]; then
  mkdir orig_src
fi
cp $(perl -e 'undef($/); $_=<>; s/\\\n//mgs; @src = grep { /^loqui_SOURCES = / } split("\n"); @csrc = grep { /\.c$/ } split(/\s+/, $src[0]); print join("\n", @csrc)' ../src/Makefile.am | sed -e 's|^|../src/|') orig_src

cd orig_src
rm main.c
cat > Makefile <<EOF
SOURCES := \$(wildcard *.c)
OBJS := \$(SOURCES:%.c=%.o)
%.o: %.c
	gcc -c \$< \`pkg-config --cflags gtk+-2.0 gnet-2.0\` -I../../ -I../../src/

all: \$(OBJS)
EOF
make
cd ..
