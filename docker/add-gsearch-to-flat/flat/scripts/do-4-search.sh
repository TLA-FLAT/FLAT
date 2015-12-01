#!/bin/bash
wget -t 1 --user fgsAdmin --password fgsAdmin -O do-search-result.html "http://localhost:8080/fedoragsearch/rest?operation=updateIndex&action=fromFoxmlFiles&value=" &&\
/var/www/fedora/tomcat/bin/tomcat-fedora.sh stop &&\
bash -c "timeout 60 grep -q 'Server shutdown complete' <(tail -f $FEDORA_HOME/server/logs/fedora.log)" &&\
sleep 10 &&\
/var/www/fedora/tomcat/bin/tomcat-fedora.sh start &&\
bash -c "timeout 60 grep -q 'Server startup complete' <(tail -f $FEDORA_HOME/server/logs/fedora.log)" &&\
sleep 10
