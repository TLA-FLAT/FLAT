#!/bin/sh

date

/import/isle2clarin /import/imdi-to-convert.txt

if [ -d /import/cmd ]; then
	rm -rf /import/cmd
fi

mkdir /import/cmd/media-archive

rsync -avzr /import/src/ /import/cmd/media-archibe --include '*.cmdi' --exclude '*.imdi' --exclude 'corpman' --exclude 'sessions' --exclude 'mirrored_corpora' --exclude 'media-archive'

date
