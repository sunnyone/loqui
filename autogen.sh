#!/bin/sh

set -e

# pushd test
# ./update-makefile.sh
# popd

aclocal -I m4

libtoolize --force --copy

autoheader

for i in libloqui/Makefile.am src/Makefile.am; do
  m4 -I. "$i.m4" > "$i"
done

automake --add-missing --foreign --copy

autoconf

./configure "$@"
