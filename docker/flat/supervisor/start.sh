#!/bin/bash

service supervisor start &&\
/wait-postgres.sh &&\
supervisorctl start tomcat &&\
/wait-tomcat.sh
ERR="${?}"
if [ ! ${ERR} = 0 ]; then
    exit ${ERR}
fi

#http://patorjk.com/software/taag/#p=display&f=Slant%20Relief&t=FLAT
cat /flat.txt 

#docker: /bin/bash -l $*
#compose: tail -f /dev/null

if [ ${FLAT_START_MODE} = 'bg' ]; then
    echo "flat start mode[bg]"
    tail -f /dev/null
else
    echo "flat start mode[fg]"
    /bin/bash -l $*
fi
