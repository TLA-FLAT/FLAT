#!/bin/sh

date

if [ -f /app/flat/imdi-to-convert.txt ]; then
	/app/flat/isle2clarin.sh /app/flat/imdi-to-convert.txt
else
	/app/flat/isle2clarin.sh /app/flat/src
fi

if [ -d /app/flat/cmd ]; then
	rm -rf /app/flat/cmd
fi

mkdir -p /app/flat/cmd

rsync -avzr /app/flat/src/ /app/flat/cmd/ --include '*.cmdi' --exclude '*.imdi' --exclude 'corpman' --exclude 'sessions' --exclude 'mirrored_corpora' --exclude 'media-archive'

date
