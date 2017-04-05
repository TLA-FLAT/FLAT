FROM		flat
MAINTAINER	Menzo Windhouwer <menzo.windhouwer@meertens.knaw.nl>

RUN apt-get update &&\
	apt-get -y dist-upgrade &&\
	apt-get -y install ant
	
#
# SOLR configuration
#

ADD drupal/flat_imdi_search /var/www/html/$DRUPAL_PROJECT/sites/all/modules/custom/

RUN cd /var/www/html/$DRUPAL_PROJECT/ &&\
	export PGPASSWORD=fedora &&\
	/etc/init.d/postgresql start &&\
	/var/www/fedora/tomcat/bin/startup.sh &&\
	bash -c "timeout $TOMCAT_TIMEOUT grep -q '(AuthFilterJAAS) initialised servlet filter: org.fcrepo.server.security.jaas.AuthFilterJAAS' <(tail -f $FEDORA_HOME/server/logs/fedora.log)" &&\
	/var/www/composer/vendor/drush/drush/drush en flat_imdi_search -y &&\
	/var/www/composer/vendor/drush/drush/drush vset islandora_solr_limit_result_fields 1 &&\
	/var/www/composer/vendor/drush/drush/drush vset islandora_solr_query_fields 'dc.title^5 dc.subject^2 dc.description^2 cmd.fulltext^2 dc.creator^2 dc.contributor^1 dc.type' &&\
sleep 10 &&\
	/var/www/fedora/tomcat/bin/shutdown.sh &&\
	bash -c "timeout $TOMCAT_TIMEOUT grep -q 'Server shutdown complete' <(tail -f $FEDORA_HOME/server/logs/fedora.log)" &&\
	/etc/init.d/postgresql stop

#
# CMD GSearch mapping
#

RUN mkdir -p /app/flat/deposit/policies/
ADD flat/gsearch-mapping-template.xml /app/flat/deposit/policies/

#Clean up APT when done.
RUN apt-get clean &&\
	rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*
