#!/bin/sh

java $JAVA_OPTS -Xmx4096m -jar /app/flat/lib/lat2fox.jar $*

exit $?
