#!/bin/bash

# disable OAI polling
su -c "psql proai -c \"UPDATE rcAdmin SET pollingEnabled = 0;\"" postgres

export FEDORA_HOME="/var/www/fedora"

echo "START: `date --rfc-3339=seconds`"
if [ -f /app/flat/log ]; then
	rm /app/flat/log
fi
for DIR in `find /app/flat/fox/ -type d | sed '1d'`; do
  if [ -f STOP ]; then
	echo "FORCED STOP: `date --rfc-3339=seconds`"
	rm STOP
    # enable OAI polling
    su -c "psql proai -c \"UPDATE rcAdmin SET pollingEnabled = 1;\"" postgres
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
  $FEDORA_HOME/client/bin/fedora-batch-ingest.sh $DIR /app/flat/log xml info:fedora/fedora-system:FOXML-1.1 localhost:8443 fedoraAdmin fedora https fedora
  E="`date '+%s'`"
  RES="FAILED"
  CNT="-1"
  if [ -f /app/flat/log ]; then
  	cat /app/flat/log
	RES="SUCCESS"
        CNT="`grep path2object /app/flat/log | wc -l`"
	rm /app/flat/log
  fi
  echo "END DIR: $DIR,$RES,$CNT,$BEGIN,`date --rfc-3339=seconds`,`expr $E - $B`"
done
echo "STOP: `date --rfc-3339=seconds`"

# enable OAI polling
su -c "psql proai -c \"UPDATE rcAdmin SET pollingEnabled = 1;\"" postgres