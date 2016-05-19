#!/bin/sh

CONTENT_TYPE=application/zip
FILENAME=$1
TARGET=https://act.easy.dans.knaw.nl/sword2/collection/1
SLUG=`date '+%Y%m%d%H%M%S'`
MD5=`md5 -q $FILENAME | awk '{print $1;}'`
IN_PROGRESS=false
USERNAME=menzowi
PASSWORD=:dansmaw7.

# NOTE: uses -k as we use a self signed certifcate ... NOT for production!
#curl -k -v -H "Content-Type: $CONTENT_TYPE" -H "Slug: $SLUG" -H "Content-Disposition: attachment; filename=$FILENAME" -H "Packaging: http://purl.org/net/sword/package/BagIt" -H "Content-MD5: $MD5"  -H "In-Progress: $IN_PROGRESS" -i -u $USERNAME:$PASSWORD  --data-binary @"$FILENAME"  $TARGET

wget \
  -o $FILENAME-sword.log \
  -O $FILENAME-sword.xml \
  --post-file=$FILENAME \
  --header "Content-Type: $CONTENT_TYPE" \
  --header "Slug: $SLUG" \
  --header "Content-Disposition: attachment; filename=$FILENAME" \
  --header "Packaging: http://purl.org/net/sword/package/BagIt" \
  --header "Content-MD5: $MD5" \
  --header "In-Progress: $IN_PROGRESS" \
  --user "$USERNAME" \
  --password "$PASSWORD" \
  $TARGET &&\
xmllint --format $FILENAME-sword.xml
