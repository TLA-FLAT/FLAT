#!/bin/sh

date

/import/lat2fox -n=1000 -r=./relations.xml -f=./fox -x=./fox-error -i=/lat /import/cmd
ERR=$?

date

exit $ERR
