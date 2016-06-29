#!/bin/sh

date

if [ -f /app/flat/imdi-to-convert.txt ]; then
	/app/flat/isle2clarin.sh /app/flat/imdi-to-convert.txt
else
	if [ -f /app/flat/imdi-to-skip.txt ]; then
		/app/flat/isle2clarin.sh -s /app/flat/imdi-to-skip.txt /app/flat/src
	else
		/app/flat/isle2clarin.sh /app/flat/src
	fi
fi

if [ -d /app/flat/cmd ]; then
	rm -rf /app/flat/cmd
fi

mkdir -p /app/flat/cmd

rsync -Lavzr /app/flat/src/ /app/flat/cmd/ --include='*/' --include='*.cmdi' --exclude='*' --exclude='corpman' --exclude='sessions' --exclude='mirrored_corpora' --exclude='media-archive'

date
