#!/bin/sh

date

/app/flat/lat2fox.sh -n=1000 -r=./relations.xml -d=./imdi2dc.xsl -f=./fox -x=./fox-error -i=/lat /app/flat/cmd
ERR=$?

date

exit $ERR
