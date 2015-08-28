#
# Based on the EasyLat islandora base image
# See:  https://github.com/TheLanguageArchive/EasyLAT/tree/master/docker/easylat

# Building the image:
#   docker build -t easylat-with-solution-packs .
#
# Running the image:
#   docker run -i -p 80:80 -p 8443:8443 -p 8080:8080 -h fedora.test.lan -t easylat-with-solution-packs /sbin/my_init -- bash -l
#
#
FROM        easylat
MAINTAINER	Menzo Windhouwer <menzo.windhouwer@meertens.knaw.nl>

ENV FEDORA_HOME /var/www/fedora/
ENV CATALINA_HOME /var/www/fedora/tomcat/

RUN apt-get clean &&\
    rm -rf /var/lib/apt/lists/*

RUN mkdir -p /tmp/islandora
RUN mkdir -p /var/www/html/drupal/sites/all/modules/contrib

#
# Audio solution pack
#

RUN apt-get update &&\
    apt-get -y dist-upgrade &&\
    apt-get -y install lame libmp3lame0
    
RUN cd /tmp/islandora &&\
	wget "https://github.com/Islandora/islandora_solution_pack_audio/archive/7.x-1.5.zip" &&\
	mv 7.x-1.5.zip islandora_solution_pack_audio-7.x-1.5.zip &&\
	unzip islandora_solution_pack_audio-7.x-1.5.zip -d /var/www/html/drupal/sites/all/modules/contrib/

RUN cd /var/www/html/drupal/ &&\
    export PGPASSWORD=fedora &&\
    /etc/init.d/postgresql start &&\
    export FEDORA_HOME=/var/www/fedora/ &&\
    export CATALINA_HOME=/var/www/fedora/tomcat/ &&\
    /var/www/fedora/tomcat/bin/startup.sh &&\
    sleep 60 &&\
#    cat /var/www/fedora/server/logs/fedora.log &&\
    /var/www/composer/vendor/drush/drush/drush en islandora_audio -y &&\
    /var/www/fedora/tomcat/bin/shutdown.sh &&\
    sleep 10 &&\
    /etc/init.d/postgresql stop
    
#
# Islandora Batch Derivative Trigger and its dependencies
#

RUN apt-get update &&\
    apt-get -y dist-upgrade &&\
    apt-get -y install gridsite-clients
    
RUN cd /tmp/islandora &&\
	wget "https://github.com/Islandora/php_lib/archive/7.x-1.5.zip" &&\
	mv 7.x-1.5.zip php_lib-7.x-1.5.zip &&\
	unzip php_lib-7.x-1.5.zip -d /var/www/html/drupal/sites/all/modules/contrib/

RUN cd /tmp/islandora &&\
	wget "https://github.com/Islandora/objective_forms/archive/7.x-1.5.zip" &&\
	mv 7.x-1.5.zip objective_forms-7.x-1.5.zip &&\
	unzip objective_forms-7.x-1.5.zip -d /var/www/html/drupal/sites/all/modules/contrib/

RUN cd /tmp/islandora &&\
	wget "https://github.com/Islandora/islandora_xml_forms/archive/7.x-1.5.zip" &&\
	mv 7.x-1.5.zip islandora_xml_forms-7.x-1.5.zip &&\
	unzip islandora_xml_forms-7.x-1.5.zip -d /var/www/html/drupal/sites/all/modules/contrib/

RUN cd /tmp/islandora &&\
	wget "https://github.com/qadan/islandora_batch_derivative_trigger/archive/7.x.zip" &&\
	mv 7.x.zip islandora_batch_derivative_trigger-7.x.zip &&\
	unzip islandora_batch_derivative_trigger-7.x.zip -d /var/www/html/drupal/sites/all/modules/contrib/

RUN cd /var/www/html/drupal/ &&\
    export PGPASSWORD=fedora &&\
    /etc/init.d/postgresql start &&\
    export FEDORA_HOME=/var/www/fedora/ &&\
    export CATALINA_HOME=/var/www/fedora/tomcat/ &&\
    /var/www/fedora/tomcat/bin/startup.sh &&\
    sleep 60 &&\
#    cat /var/www/fedora/server/logs/fedora.log &&\
    /var/www/composer/vendor/drush/drush/drush en php_lib -y &&\
    /var/www/composer/vendor/drush/drush/drush en objective_forms -y &&\
    /var/www/composer/vendor/drush/drush/drush en xml_form_builder -y &&\
    /var/www/composer/vendor/drush/drush/drush en islandora_batch_derivative_trigger -y &&\
    /var/www/fedora/tomcat/bin/shutdown.sh &&\
    sleep 10 &&\
    /etc/init.d/postgresql stop

#Clean up APT when done.
RUN apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

EXPOSE 80
EXPOSE 443
EXPOSE 8443
EXPOSE 8080 
