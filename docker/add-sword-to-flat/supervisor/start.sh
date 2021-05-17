#!/bin/bash

service supervisor start &&\
/wait-postgres.sh &&\
supervisorctl start tomcat &&\
/wait-tomcat.sh &&\
supervisorctl start easy-deposit

#http://patorjk.com/software/taag/#p=display&f=Slant%20Relief&t=FLAT
cat /flat.txt &&\
/bin/bash -l $*