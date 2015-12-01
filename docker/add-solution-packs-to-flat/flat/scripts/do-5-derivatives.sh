#!/bin/sh

_PWD=`pwd`

cd /var/www/html/drupal

FEDORA="https://localhost:8443/fedora"
QUERY='select $s from <#ri> where $s <info:fedora/fedora-system:def/model#hasModel><info:fedora/islandora:sp-audioCModel>'
QUERY=`urlencode "$QUERY"`

wget -qO- --user=fedoraAdmin --password=fedora --no-check-certificate "$FEDORA/risearch?type=tuples&lang=itql&format=csv&distinct=on&query=$QUERY" | sed -n '1!p' | while read PID;
do
	PID=`echo $PID | sed -s 's|info:fedora/||g'`;
	echo $PID;
	/var/www/composer/vendor/drush/drush/drush -v idr --pids=$PID --dsids=TN,PROXY_MP3;
done

cd $_PWD
