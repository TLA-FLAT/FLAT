#!/bin/bash

TO=`expr $FLAT_TIMEOUT / 10`

PLOG="/var/log/postgresql/postgresql-9.5-main.log"
if [ -f /var/log/postgresql/postgresql.log ]; then
    PLOG="/var/log/postgresql/postgresql.log"
fi

TO=`expr $FLAT_TIMEOUT / 10`
timeout $FLAT_TIMEOUT bash -c "until egrep 'database system (is|was)' $PLOG | tail -n 1 | grep 'database system is shut down'; do sleep $TO; echo 'Waiting ("${TO}" sec.) for postgres to be completely shutdown ...'; done" 2> /dev/null

RES=$?
if [ $RES = 124 ]; then
    echo "Postgres took too long (> ${FLAT_TIMEOUT} sec.) to become completely shutdown!";
else
    echo "Postgres is completely shutdown.";
fi

echo "Postgres log[$PLOG] tail>";
tail $PLOG
echo "<Postgres log[$PLOG] tail";

exit $RES