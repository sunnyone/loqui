#!/bin/sh

# pushd test
# ./update-makefile.sh
# popd

aclocal -I m4 \
  && libtoolize --force --copy \
  && autoheader \
  && automake --add-missing --foreign --copy \
  && autoconf \
  && ./configure $@
