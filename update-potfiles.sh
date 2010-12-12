#!/bin/sh
rm po/POTFILES.in
for I in libloqui src `find libloqui/protocols -type d | egrep -v "/\\."`; do
  bzr ls -V $I | egrep '\.(c|h|gob)$' >> po/POTFILES.in
done
