#!/bin/sh

if [ ! -f /app/flat/deposit/policies/cmd2fox.xsl ]; then
	echo "FATAL: please provide a cmd2fox.xsl in /app/flat/deposit/policies/, which overwrites the empty DC templates!"
	exit 1
fi

date

export JAVA_OPTS="-DLAT2FOX.cmd2fox=file:/app/flat/deposit/policies/cmd2fox.xsl"

mkdir -p /app/flat/tmp

lat2fox.sh -e=$CMD_EXTENSION -h -n=1000 -r=/app/flat/tmp/relations.xml -f=/app/flat/fox -x=/app/flat/tmp/fox-error -p=/app/flat/deposit/policies -i=/lat -b=$FLAT_ICON_DIR /app/flat/cmd 2>&1 | tee -a /app/flat/tmp/lat2fox.log
ERR=$?

date

exit $ERR
