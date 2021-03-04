#!/bin/bash

service supervisor start &&\
/wait-postgres.sh &&\
supervisorctl start tomcat &&\
/wait-tomcat.sh

#http://patorjk.com/software/taag/#p=display&f=Slant%20Relief&t=FLAT
cat /flat.txt &&\
tail -f /dev/null