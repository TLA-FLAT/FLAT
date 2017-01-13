#/bin/sh

_PWD=$PWD

mkdir -p /app/flat/src
cd /app/flat/src
ln -s /lat/Hocank .
cd /app/flat
ln -s /lat/imdi-to-skip.txt .

/app/flat/do-0-convert.sh &&\
 /app/flat/do-0-config-cmd-gsearch.sh &&\
 /app/flat/do-1-fox.sh &&\
# cp /app/flat/deposit/test/*.xml /app/flat/fox/1/ &&\
 /app/flat/do-2-import.sh &&\
 /app/flat/do-3-index.sh

cd $_PWD
