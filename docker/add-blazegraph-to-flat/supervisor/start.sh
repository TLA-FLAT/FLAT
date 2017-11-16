#!/bin/bash

service supervisor start &&\
/wait-postgres.sh &&\
supervisorctl start tomcat &&\
/wait-tomcat.sh
supervisorctl start tomcat-blzg &&\
/wait-tomcat-blzg.sh

#http://patorjk.com/software/taag/#p=display&f=Slant%20Relief&t=FLAT
cat /flat.txt &&\
/bin/bash -l $*
