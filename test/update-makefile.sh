#!/bin/sh
sed -ne '/^loqui_SOURCES/, /[^\\]$/ { s/main.c//; s/loqui_SOURCES/ORIG_SRC/; s/[A-Za-z0-9_]*\.[ch]/..\/src\/&/g; p }' ../src/Makefile.am > Makefile.am
cat Makefile.am.after >> Makefile.am
