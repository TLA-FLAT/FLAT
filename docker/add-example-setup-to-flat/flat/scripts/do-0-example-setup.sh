#!/bin/bash

if [ ! -d /app/flat/fox ]; then
    mkdir -p /app/flat/fox
fi

mkdir -p /app/flat/cmd
for REC in `find /app/flat/test/cmd -type f -name '*.cmdi'`; do
	NEW=`basename $REC .cmd`
	cp $REC /app/flat/cmd/$NEW.$CMD_EXTENSION
done

cp -r /app/flat/test/data/* /app/flat/data/
cp /app/flat/test/*.xml /app/flat/fox/