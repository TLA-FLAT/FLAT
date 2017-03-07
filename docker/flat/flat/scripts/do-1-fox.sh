#!/bin/sh

if [ ! -f /app/flat/cmd2fox.xsl ]; then
	echo "FATAL: please provide a cmd2fox.xsl with overwritten DC templates!"
	exit 1
fi

date

export JAVA_OPTS="-DLAT2FOX.cmd2fox=file:/app/flat/cmd2fox.xsl"

/app/flat/lat2fox.sh -e=$CMD_EXTENSION -h -n=1000 -r=/app/flat/relations.xml -f=/app/flat/fox -x=/app/flat/fox-error -p=/app/flat/policies -i=/lat -b=$FLAT_ICONS /app/flat/cmd
ERR=$?

date

exit $ERR
