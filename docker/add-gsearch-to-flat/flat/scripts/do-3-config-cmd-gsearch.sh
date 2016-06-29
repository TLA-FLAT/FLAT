#!/bin/bash

GSEARCH_HOME=$FEDORA_HOME/tomcat/webapps/fedoragsearch/WEB-INF/classes/fgsconfigFinal/index/FgsIndex

/app/flat/config-cmd-gsearch.sh \
    -t $GSEARCH_HOME/foxmlToSolr.xslt \
    -s $GSEARCH_HOME/conf/flat-solr-schema.xml \
    -i /app/flat/cmd -x cmdi \
    -v
cp lat-gsearch-transformer.xsl \
    $GSEARCH_HOME/foxmlToSolr.xslt
cp lat-gsearch-schema.xml \
    $FEDORA_HOME/solr/collection1/conf/schema.xml
    
/var/www/fedora/tomcat/bin/tomcat-fedora.sh stop &&\
bash -c "timeout 60 grep -q 'Server shutdown complete' <(tail -f $FEDORA_HOME/server/logs/fedora.log)" &&\
sleep 10 &&\
/var/www/fedora/tomcat/bin/tomcat-fedora.sh start &&\
bash -c "timeout 60 grep -q 'Server startup complete' <(tail -f $FEDORA_HOME/server/logs/fedora.log)" &&\
sleep 10