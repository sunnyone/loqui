#!/bin/sh
git ls-tree -r --name-only HEAD | egrep '^(src|libloqui)/.*\.(c|h|gob)$' > po/POTFILES.in
