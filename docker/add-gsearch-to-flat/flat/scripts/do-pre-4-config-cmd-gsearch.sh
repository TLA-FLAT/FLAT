#!/bin/bash

GSEARCH_HOME=$FEDORA_HOME/tomcat/webapps/fedoragsearch/WEB-INF/classes/fgsconfigFinal/index/FgsIndex

/app/flat/config-cmd-gsearch.sh \
    -t $GSEARCH_HOME/foxmlToSolr.xslt \
    -s $GSEARCH_HOME/conf/schema-4.6.1-for-fgs-2.7.xml \
    -i /app/flat/cmd -x cmdi \
    -v
cp lat-gsearch-transformer.xsl \
    $GSEARCH_HOME/foxmlToSolr.xslt