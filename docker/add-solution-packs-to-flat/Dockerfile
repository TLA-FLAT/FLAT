FROM		flat
MAINTAINER	Menzo Windhouwer <menzo.windhouwer@meertens.knaw.nl>

RUN mkdir -p /tmp/islandora &&\
	mkdir -p /var/www/html/drupal/sites/all/modules/contrib

#
# Audio solution pack
#

RUN apt-get update &&\
    apt-get -y dist-upgrade &&\
    apt-get -y install lame libmp3lame0
    
RUN cd /tmp/islandora &&\
	wget "https://github.com/Islandora/islandora_solution_pack_audio/archive/7.x-1.5.zip" &&\
	mv 7.x-1.5.zip islandora_solution_pack_audio-7.x-1.5.zip &&\
	unzip islandora_solution_pack_audio-7.x-1.5.zip -d /var/www/html/drupal/sites/all/modules/contrib/ &&\
	cd /var/www/html/drupal/ &&\
	export PGPASSWORD=fedora &&\
	/etc/init.d/postgresql start &&\
	/var/www/fedora/tomcat/bin/startup.sh &&\
	bash -c "timeout 60 grep -q '(AuthFilterJAAS) initialised servlet filter: org.fcrepo.server.security.jaas.AuthFilterJAAS' <(tail -f $FEDORA_HOME/server/logs/fedora.log)" &&\
sleep 10 &&\
	/var/www/composer/vendor/drush/drush/drush en islandora_audio -y &&\
	/var/www/fedora/tomcat/bin/shutdown.sh &&\
	bash -c "timeout 60 grep -q 'Server shutdown complete' <(tail -f $FEDORA_HOME/server/logs/fedora.log)" &&\
	/etc/init.d/postgresql stop
    
#
# Islandora Batch Derivative Trigger and its dependencies
#

RUN cd /tmp/islandora &&\
	wget "https://github.com/Islandora/php_lib/archive/7.x-1.5.zip" &&\
	mv 7.x-1.5.zip php_lib-7.x-1.5.zip &&\
	unzip php_lib-7.x-1.5.zip -d /var/www/html/drupal/sites/all/modules/contrib/ &&\
	cd /tmp/islandora &&\
	wget "https://github.com/Islandora/objective_forms/archive/7.x-1.5.zip" &&\
	mv 7.x-1.5.zip objective_forms-7.x-1.5.zip &&\
	unzip objective_forms-7.x-1.5.zip -d /var/www/html/drupal/sites/all/modules/contrib/ &&\
	cd /tmp/islandora &&\
	wget "https://github.com/Islandora/islandora_xml_forms/archive/7.x-1.5.zip" &&\
	mv 7.x-1.5.zip islandora_xml_forms-7.x-1.5.zip &&\
	unzip islandora_xml_forms-7.x-1.5.zip -d /var/www/html/drupal/sites/all/modules/contrib/ &&\
	cd /tmp/islandora &&\
	wget "https://github.com/qadan/islandora_batch_derivative_trigger/archive/7.x.zip" &&\
	mv 7.x.zip islandora_batch_derivative_trigger-7.x.zip &&\
	unzip islandora_batch_derivative_trigger-7.x.zip -d /var/www/html/drupal/sites/all/modules/contrib/ &&\
	cd /var/www/html/drupal/ &&\
	export PGPASSWORD=fedora &&\
	/etc/init.d/postgresql start &&\
	/var/www/fedora/tomcat/bin/startup.sh &&\
	bash -c "timeout 60 grep -q '(AuthFilterJAAS) initialised servlet filter: org.fcrepo.server.security.jaas.AuthFilterJAAS' <(tail -f $FEDORA_HOME/server/logs/fedora.log)" &&\
	/var/www/composer/vendor/drush/drush/drush en php_lib -y &&\
	/var/www/composer/vendor/drush/drush/drush en objective_forms -y &&\
	/var/www/composer/vendor/drush/drush/drush en xml_form_builder -y &&\
	/var/www/composer/vendor/drush/drush/drush en islandora_batch_derivative_trigger -y &&\
sleep 10 &&\
	/var/www/fedora/tomcat/bin/shutdown.sh &&\
	bash -c "timeout 60 grep -q 'Server shutdown complete' <(tail -f $FEDORA_HOME/server/logs/fedora.log)" &&\
	/etc/init.d/postgresql stop

#
# Add FLAT's own scripts and tools 
#

RUN mkdir -p /app/flat
ADD flat/scripts/* /app/flat/
RUN chmod +x /app/flat/*.sh

#Clean up APT when done.
RUN apt-get clean &&\
	rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*
