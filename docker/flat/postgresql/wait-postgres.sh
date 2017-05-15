#!/bin/bash

TO=`expr $FLAT_TIMEOUT / 10`

# give supervisor a little time to start postgres, create the log to monitor and first entries to appear
sleep $TO

PLOG="/var/log/postgresql/postgresql-9.5-main.log"
if [ -f /var/log/postgresql/postgresql.log ]; then
    PLOG="/var/log/postgresql/postgresql.log"
fi

TO=`expr $FLAT_TIMEOUT / 10`
timeout $FLAT_TIMEOUT bash -c "until egrep 'database system (is|was)' $PLOG | tail -n 1 | grep 'database system is ready to accept connections'; do sleep $TO; echo 'Waiting ("${TO}" sec.) for postgres to be completely up and running ... '; done" 2> /dev/null

RES=$?
if [ $RES = 124 ]; then
    echo "Postgres took too long (> ${FLAT_TIMEOUT} sec.) to become completely up and running!";
else
    echo "Postgres is completely up and running.";
fi

echo "Postgres log[$PLOG] tail>";
tail $PLOG
echo "<Postgres log[$PLOG] tail";

exit $RES