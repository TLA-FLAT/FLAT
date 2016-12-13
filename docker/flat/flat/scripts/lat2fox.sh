#!/bin/sh

java -Xmx4096m -jar $JAVA_OPTS /app/flat/lib/lat2fox.jar $*

exit $?
