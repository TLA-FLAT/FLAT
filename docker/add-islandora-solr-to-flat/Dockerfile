FROM		flat
MAINTAINER	Menzo Windhouwer <menzo.windhouwer@meertens.knaw.nl>

RUN apt-get update &&\
	apt-get -y dist-upgrade &&\
	apt-get -y install ant

RUN mkdir /tmp/islandora &&\
	cd /tmp/islandora &&\
	wget "https://github.com/Islandora/islandora_solr_search/archive/7.x-1.6.zip" &&\
	mv 7.x-1.6.zip islandora_gsearch_module-7.x-1.6.zip &&\
	unzip islandora_gsearch_module-7.x-1.6.zip -d /var/www/html/drupal/sites/all/modules/contrib/

ADD drupal/flat_search /var/www/html/drupal/sites/all/modules/custom/

RUN cd /var/www/html/drupal/ &&\
	export PGPASSWORD=fedora &&\
	/etc/init.d/postgresql start &&\
	/var/www/fedora/tomcat/bin/startup.sh &&\
	bash -c "timeout 60 grep -q '(AuthFilterJAAS) initialised servlet filter: org.fcrepo.server.security.jaas.AuthFilterJAAS' <(tail -f $FEDORA_HOME/server/logs/fedora.log)" &&\
	/var/www/composer/vendor/drush/drush/drush en islandora_solr -y &&\
	/var/www/composer/vendor/drush/drush/drush en islandora_solr_config -y &&\
	/var/www/composer/vendor/drush/drush/drush en flat_search -y &&\
    /var/www/composer/vendor/drush/drush/drush vset islandora_basic_collection_display_backend islandora_solr_query_backend -y &&\
    /var/www/composer/vendor/drush/drush/drush vset islandora_compound_object_thumbnail_child 0 -y &&\
    /var/www/composer/vendor/drush/drush/drush vset islandora_compound_object_show_compound_parents_in_breadcrumbs 1 -y &&\
 sleep 10 &&\
	/var/www/fedora/tomcat/bin/shutdown.sh &&\
	bash -c "timeout 60 grep -q 'Server shutdown complete' <(tail -f $FEDORA_HOME/server/logs/fedora.log)" &&\
	/etc/init.d/postgresql stop

#Clean up APT when done.
RUN apt-get clean &&\
	rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*
