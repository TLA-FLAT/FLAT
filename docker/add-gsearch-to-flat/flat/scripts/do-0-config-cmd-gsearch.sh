#!/bin/bash

CMD_GSEARCH_HOME=/app/flat/lib/cmd-gsearch
GSEARCH_HOME=$FEDORA_HOME/tomcat/webapps/fedoragsearch/WEB-INF/classes/fgsconfigFinal/index/FgsIndex

_PWD=$PWD

mkdir -p /app/flat/tmp
cd /app/flat/tmp

flat-config-cmd-gsearch.sh \
    -b $CMD_GSEARCH_HOME \
    -d $CMD_GSEARCH_HOME/cmd2dc-template.xsl \
    -g /app/flat/deposit/policies/gsearch-mapping.xml \
    -i /app/flat/cmd \
    -s $GSEARCH_HOME/conf/flat-solr-schema.xml \
    -t $GSEARCH_HOME/foxmlToSolr.xslt \
    -x $CMD_EXTENSION \
    -v
cp lat-gsearch-transformer.xsl \
    $GSEARCH_HOME/foxmlToSolr.xslt
cp lat-gsearch-schema.xml \
    $FEDORA_HOME/solr/collection1/conf/schema.xml
if [ ! -f /app/flat/deposit/policies/cmd2fox.xsl ] && [ -f lat-gsearch-cmd2dc.xsl ]; then
    cp lat-gsearch-cmd2dc.xsl \
        /app/flat/deposit/policies/cmd2fox.xsl
fi
    
wget -O do-solr-reload.html  "http://localhost:8080/solr/admin/cores?action=RELOAD&core=collection1"

cd $_PWD