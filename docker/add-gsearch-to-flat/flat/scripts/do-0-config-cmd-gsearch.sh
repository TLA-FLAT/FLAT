#!/bin/bash

GSEARCH_HOME=$FEDORA_HOME/tomcat/webapps/fedoragsearch/WEB-INF/classes/fgsconfigFinal/index/FgsIndex

/app/flat/config-cmd-gsearch.sh \
    -t $GSEARCH_HOME/foxmlToSolr.xslt \
    -s $GSEARCH_HOME/conf/flat-solr-schema.xml \
    -i /app/flat/cmd \
    -x $CMD_EXTENSION \
    -v
cp lat-gsearch-transformer.xsl \
    $GSEARCH_HOME/foxmlToSolr.xslt
cp lat-gsearch-schema.xml \
    $FEDORA_HOME/solr/collection1/conf/schema.xml
if [ ! -f /app/flat/cmd2fox.xsl ] && [ -f lat-gsearch-cmd2dc.xsl ]; then
    cp lat-gsearch-cmd2dc.xsl \
        /app/flat/cmd2fox.xsl
fi
    
wget -O do-solr-reload.html  "http://localhost:8080/solr/admin/cores?action=RELOAD&core=collection1"
