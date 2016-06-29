#!/bin/sh

export CLASSPATH="`find /app/flat/lib/saxon -type f -name '*.jar' -exec echo -n "{}:" \;`$CLASSPATH"

java -Xmx512m net.sf.saxon.Transform $*

exit $?
