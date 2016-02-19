#!/bin/sh

if [ ! -f /app/flat/cmd2dc.xsl ]; then
	echo "FATAL: please provide a cmd2dc.xsl!"
	exit 1
fi

ARGS=""
if [ -f /app/flat/cmd2other.xsl ]; then
	ARGS="$ARGS -m=/app/flat/cmd2other.xsl"
fi

date

/app/flat/lat2fox.sh -h -n=1000 -r=/app/flat/relations.xml -d=/app/flat/cmd2dc.xsl -f=/app/flat/fox -x=/app/flat/fox-error -i=/lat -b=$FLAT_ICONS $ARGS /app/flat/cmd
ERR=$?

date

exit $ERR
