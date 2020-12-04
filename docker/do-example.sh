#!/bin/bash

FLAT="FLAT" && echo $FLAT &&\
    docker build --squash -t flat flat &&\
FLAT="$FLAT + BLAZEGRAPH" && echo $FLAT &&\
    docker build --squash -t flat add-blazegraph-to-flat &&\
FLAT="$FLAT + GSEARCH" && echo $FLAT &&\
    docker build --squash -t flat add-gsearch-to-flat &&\
FLAT="$FLAT + ISLANDORA SOLR" && echo $FLAT &&\
    docker build --squash -t flat add-islandora-solr-to-flat &&\
FLAT="$FLAT + ISLANDORA OAI" && echo $FLAT &&\
    docker build --squash -t flat add-islandora-oai-to-flat &&\
FLAT="$FLAT + SHIBBOLETH" && echo $FLAT &&\
    docker build --squash -t flat add-shibboleth-to-flat &&\
FLAT="$FLAT + SWORD" && echo $FLAT &&\
    docker build --squash -t flat add-sword-to-flat &&\
    docker tag flat flat-example-pre-doorkeeper &&\
FLAT="$FLAT + DOORKEEPER" && echo $FLAT &&\
    cd add-doorkeeper-to-flat &&\
    tar -czh . | docker build --squash -t flat - &&\
    cd .. &&\
FLAT="$FLAT + EXAMPLE SETUP" && echo $FLAT &&\
    docker build --squash -t example-flat add-example-setup-to-flat &&\
echo "TODO: docker run -p 80:80 -p 8080:8080 --name=flat --rm -it example-flat" &&\
    tput bel
