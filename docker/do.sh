#!/bin/bash

FLAT="FLAT" && echo $FLAT &&\
    docker build --squash -t flat flat &&\
FLAT="$FLAT + IMDI CONVERSION" && echo $FLAT &&\
    docker build --squash -t flat add-imdi-conversion-to-flat &&\
FLAT="$FLAT + GSEARCH" && echo $FLAT &&\
    docker build --squash -t flat add-gsearch-to-flat &&\
FLAT="$FLAT + ISLANDORA SOLR" && echo $FLAT &&\
    docker build --squash -t flat add-islandora-solr-to-flat &&\
FLAT="$FLAT + IMDI GSEARCH" && echo $FLAT &&\
    docker build --squash -t flat add-imdi-gsearch-to-flat &&\
FLAT="$FLAT + SWORD" && echo $FLAT &&\
    docker build --squash -t flat add-sword-to-flat &&\
docker tag flat flat-pre-doorkeeper &&\
    ./do-doorkeeper.sh $FLAT
