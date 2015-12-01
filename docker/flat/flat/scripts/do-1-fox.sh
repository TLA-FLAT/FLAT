#!/bin/sh

if [ ! -f /app/flat/cmd2dc.xsl ]; then
	echo "FATAL: please provide a cmd2dc.xsl!"
	exit 1
fi

date

/app/flat/lat2fox.sh -n=1000 -r=/app/flat/relations.xml -d=/app/flat/cmd2dc.xsl -f=/app/flat/fox -x=/app/flat/fox-error -i=/lat /app/flat/cmd
ERR=$?

date

exit $ERR
