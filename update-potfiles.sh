#!/bin/sh
rm po/POTFILES.in
for I in libloqui src; do
  svn ls $I | egrep '\.(c|h|gob)$' | sed -e "s/^/$I\//" >> po/POTFILES.in
done
