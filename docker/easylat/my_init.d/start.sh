#!/bin/sh
/etc/init.d/postgresql start
sleep 10
/etc/init.d/apache2 start
sleep 10
/var/www/fedora/tomcat/bin/tomcat-fedora.sh start
sleep 10