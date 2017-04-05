#!/bin/sh

export CLASSPATH="`find /var/www/fedora/tomcat/webapps/flat/WEB-INF/lib -type f -name '*.jar' ! -name 'logback-classic-*.jar' -exec echo -n "{}:" \;`$CLASSPATH"
java $JAVA_OPTS nl.mpi.tla.flat.deposit.DoorKeeper /app/flat/deposit/flat-deposit.xml $*