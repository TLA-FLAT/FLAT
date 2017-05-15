#!/bin/bash

# wait till the shutdown port is available
TO=`expr $FLAT_TIMEOUT / 10`
timeout $FLAT_TIMEOUT bash -c "until echo > /dev/tcp/localhost/8005; do sleep $TO; echo 'Waiting ("${TO}" sec.) for tomcat to be completely up and running ... '; done" 2> /dev/null

RES=$?
if [ $RES = 124 ]; then
    echo "Tomcat took too long (> ${FLAT_TIMEOUT} sec.) to become completely up and running!";
else
    echo "Tomcat is completely up and running.";
fi

echo "Tomcat log[/var/www/fedora/tomcat/logs/catalina.out] tail>";
tail /var/www/fedora/tomcat/logs/catalina.out
echo "<Tomcat log[/var/www/fedora/tomcat/logs/catalina.out] tail";
echo "Fedora log[/var/www/fedora/server/logs/fedora.log tail>";
tail /var/www/fedora/server/logs/fedora.log
echo "<Fedora log[/var/www/fedora/server/logs/fedora.log] tail";
echo "Postgres log[/var/log/postgresql/postgresql.log] tail>";
tail /var/log/postgresql/postgresql.log
echo "<Postgres log[/var/log/postgresql/postgresql.log] tail";



exit $RES