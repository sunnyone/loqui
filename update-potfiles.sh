#!/bin/sh
rm po/POTFILES.in
for I in libloqui src `find libloqui/protocols -type d | egrep -v "/\\."`; do
  svn ls $I | egrep '\.(c|h|gob)$' | sed -e "s,^,$I\/," >> po/POTFILES.in
done
