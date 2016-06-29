#!/bin/bash

case "$1" in
        start)
            export FEDORA_HOME=/var/www/fedora/
            export CATALINA_HOME=/var/www/fedora/tomcat/
            /var/www/fedora/tomcat/bin/startup.sh
            ;;
         
        stop)
            /var/www/fedora/tomcat/bin/shutdown.sh
            ;;
       
        *)
            echo $"Usage: $0 {start|stop}"
            exit 1
esac