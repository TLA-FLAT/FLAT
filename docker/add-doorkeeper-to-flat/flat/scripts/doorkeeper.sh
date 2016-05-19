#!/bin/sh

java -Djavax.net.ssl.trustStore=/opt/jssecacerts -Djavax.net.ssl.trustStorePassword=changeit -jar /app/flat/lib/doorkeeper.jar /app/flat/deposit/flat-deposit.xml $*

echo "NOTE: If SOLR is used for retrieving (collection) relationships reindexing might be required!"
