#!/bin/sh

M4_MAKEFILE_AM_FILES="libloqui/protocols/jabber/Makefile.am libloqui/Makefile.am src/Makefile.am"

set -e

# pushd test
# ./update-makefile.sh
# popd

rm -f ChangeLog

if type gob2 >/dev/null 2>&1; then
   :
else
   echo gob2 is not installed.
   exit 1
fi

aclocal -I m4

glib-gettextize --force --copy

libtoolize --force --copy

autoheader

for i in $M4_MAKEFILE_AM_FILES; do
  m4 -I. "$i.m4" > "$i"
done

automake --add-missing --foreign --copy

autoconf

./configure "$@"
