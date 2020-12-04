#!/bin/bash

FLAT="$1"
    docker tag flat-pre-doorkeeper flat &&\
FLAT="$FLAT + DOORKEEPER" && echo $FLAT &&\
    cd add-doorkeeper-to-flat &&\
    tar -czh . | docker build --squash -t flat - &&\
    cd .. &&\
FLAT="$FLAT + EXAMPLE SETUP" && echo $FLAT &&\
    docker build --squash -t mpi-flat add-example-setup-to-flat &&\
echo "TODO: docker run -p 80:80 -p 8080:8080 -v /Users/menzowi/Documents/Projects/FLAT/test-Hocank/:/lat --name=flat --rm -it mpi-flat" &&\
    tput bel
