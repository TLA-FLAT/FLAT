#/bin/sh

_PWD=$PWD

if [ -d /lat/Hocank ]; then
    mkdir -p /app/flat/src
    cd /app/flat/src
    ln -s /lat/Hocank .
    ln -s /lat/imdi-to-skip.txt .
fi

/app/flat/do-0-example-setup.sh &&\
 if [ -f /app/flat/do-0-convert.sh ]; then /app/flat/do-0-convert.sh; fi &&\
 /app/flat/do-0-config-cmd-gsearch.sh &&\
 /app/flat/do-1-fox.sh &&\
 /app/flat/do-2-import.sh &&\
 /app/flat/do-3-index.sh &&\
 /app/flat/do-9-example-setup.sh

cd $_PWD

cat << EOF
TODO: flat-create-sip.sh /app/flat/test/test-sip
TODO: flat-sword-upload.sh test-sip.zip test
TODO: wget --method=PUT http://localhost:8080/flat/doorkeeper/test
TODO: tail -f deposit/bags/test/bag-test-sip/data/test-sip/logs/devel.log
EOF