#!/bin/sh

echo "waiting for fedora tomcat to start"
while [ ! -d "/var/www/fedora/data" ]
  do
  printf "."
  sleep 1
done
echo "tomcat started"