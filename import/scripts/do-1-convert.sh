#!/bin/sh

date

/import/isle2clarin /import/imdi-to-convert.txt
ERR=$?

exit $ERR

if [ -d /import/cmd ]; then
	rm -rf /import/cmd
fi

mkdir /import/cmd

rsync -avzr /import/src/ /import/cmd/ --include '*.cmdi' --exclude '*.imdi' --exclude 'corpman' --exclude 'sessions' --exclude 'mirrored_corpora' --exclude 'media-archive'

date
