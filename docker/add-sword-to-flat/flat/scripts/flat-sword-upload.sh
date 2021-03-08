#!/bin/sh

CONTENT_TYPE=application/zip
FILENAME=$1
TARGET=http://$FLAT_HOST/$FLAT_NAME/easy-deposit/collection/1
SLUG=$2
MD5=`md5sum -b $FILENAME | awk '{print $1;}'`
IN_PROGRESS=false
USERNAME=flat
PASSWORD=sword

curl -s -H "Content-Type: $CONTENT_TYPE" -H "Slug: $SLUG" -H "Content-Disposition: attachment; filename=$FILENAME" -H "Packaging: http://purl.org/net/sword/package/BagIt" -H "Content-MD5: $MD5"  -H "In-Progress: $IN_PROGRESS" -u $USERNAME:$PASSWORD  --data-binary @"$FILENAME"  $TARGET | xmllint --format -