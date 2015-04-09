#!/bin/bash

export FEDORA_HOME="/var/www/fedora"

echo "START: `date --rfc-3339=seconds`"
if [ -f /import/log ]; then
	rm /import/log
fi
for DIR in `find /import/fox/ -type d | sed '1d'`; do
  if [ -f STOP ]; then
	echo "FORCED STOP: `date --rfc-3339=seconds`"
	rm STOP
	exit 1
  fi
  if [ -f SLEEP ]; then
    echo "SLEEP: `date --rfc-3339=seconds` (remove the SLEEP file to wake me up (after 1 minute))"
    while [ -f SLEEP ]; do
	echo -n "zZ"
	sleep 1m
    done
    echo "WAKEUP: `date --rfc-3339=seconds`"
  fi
  BEGIN="`date --rfc-3339=seconds`"
  echo "BEGIN DIR: $DIR,`date --rfc-3339=seconds`"
  B="`date '+%s'`"
  $FEDORA_HOME/client/bin/fedora-batch-ingest.sh $DIR /import/log xml info:fedora/fedora-system:FOXML-1.1 localhost:8443 fedoraAdmin fedora https fedora
  E="`date '+%s'`"
  RES="FAILED"
  CNT="-1"
  if [ -f /import/log ]; then
  	cat /import/log
	RES="SUCCESS"
        CNT="`grep path2object /import/log | wc -l`"
	rm /import/log
  fi
  echo "END DIR: $DIR,$RES,$CNT,$BEGIN,`date --rfc-3339=seconds`,`expr $E - $B`"
done
echo "STOP: `date --rfc-3339=seconds`"
