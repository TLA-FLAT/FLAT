FROM		flat
MAINTAINER	Menzo Windhouwer <menzo.windhouwer@meertens.knaw.nl>

RUN apt-get update &&\
	apt-get -y dist-upgrade &&\
	apt-get -y install ant

RUN mkdir /tmp/islandora &&\
	cd /tmp/islandora &&\
	wget "https://github.com/Islandora/islandora_solr_search/archive/7.x-1.9.zip" &&\
	mv 7.x-1.9.zip islandora_gsearch_module-7.x-1.9.zip &&\
	unzip islandora_gsearch_module-7.x-1.9.zip -d /var/www/html/$FLAT_NAME/sites/all/modules/contrib/

ADD drupal/flat_search /var/www/html/$FLAT_NAME/sites/all/modules/custom/

RUN	service supervisor start &&\
	/wait-postgres.sh &&\
	supervisorctl start tomcat &&\
	/wait-tomcat.sh &&\
	cd /var/www/html/$FLAT_NAME/ &&\
	/var/www/composer/vendor/drush/drush/drush en islandora_solr -y &&\
	/var/www/composer/vendor/drush/drush/drush en islandora_solr_config -y &&\
	/var/www/composer/vendor/drush/drush/drush en flat_search -y &&\
    /var/www/composer/vendor/drush/drush/drush vset islandora_basic_collection_display_backend islandora_solr_query_backend -y &&\
    /var/www/composer/vendor/drush/drush/drush vset islandora_compound_object_thumbnail_child 0 -y &&\
    /var/www/composer/vendor/drush/drush/drush vset islandora_compound_object_show_compound_parents_in_breadcrumbs 1 -y &&\
	supervisorctl stop all &&\
	service supervisor stop &&\
	sleep 1

#Clean up APT when done.
RUN apt-get clean &&\
	rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*
