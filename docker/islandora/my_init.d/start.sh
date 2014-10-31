#!/bin/sh
/etc/init.d/postgresql start
/etc/init.d/apache2 start
/var/www/fedora/tomcat/bin/tomcat-fedora.sh start