#!/bin/sh

CONTENT_TYPE=application/zip
FILENAME=$1
TARGET=https://192.168.99.100:8443/easy-deposit/collection
SLUG=$2
MD5=`md5sum -b $FILENAME | awk '{print $1;}'`
IN_PROGRESS=false
USERNAME=flat
PASSWORD=sword

# NOTE: uses -k as we use a self signed certifcate ... NOT for production!
curl -k -v -H "Content-Type: $CONTENT_TYPE" -H "Slug: $SLUG" -H "Content-Disposition: attachment; filename=$FILENAME" -H "Packaging: http://purl.org/net/sword/package/BagIt" -H "Content-MD5: $MD5"  -H "In-Progress: $IN_PROGRESS" -i -u $USERNAME:$PASSWORD  --data-binary @"$FILENAME"  $TARGET