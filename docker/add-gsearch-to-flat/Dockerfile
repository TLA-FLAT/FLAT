FROM		flat
MAINTAINER	Menzo Windhouwer <menzo.windhouwer@meertens.knaw.nl>

ENV FEDORA_HOME="/var/www/fedora/"
ENV CATALINA_HOME="/var/www/fedora/tomcat/"

RUN apt-get clean &&\
	rm -rf /var/lib/apt/lists/*
RUN apt-get update &&\
	apt-get -y dist-upgrade &&\
	apt-get -y install ant

#
#Fedora GSearch
#

RUN mkdir /tmp/fedora-gsearch &&\
	cd /tmp/fedora-gsearch &&\ 
	wget -O genericsearch-2.7.1.zip "http://downloads.sourceforge.net/project/fedora-commons/services/3.7/fedoragsearch-2.7.1.zip?r=&use_mirror=heanet" &&\
	unzip "genericsearch-2.7.1.zip" &&\
	cp "fedoragsearch-2.7.1/fedoragsearch.war" "/var/www/fedora/tomcat/webapps/fedoragsearch.war" &&\
	mkdir /var/www/fedora/tomcat/webapps/fedoragsearch &&\
	cd /var/www/fedora/tomcat/webapps/fedoragsearch/ &&\
	unzip ../fedoragsearch.war &&\
	rm -r /tmp/fedora-gsearch

#
# SOLR
#

RUN mkdir /tmp/solr
RUN cd /tmp/solr &&\
	wget "https://archive.apache.org/dist/lucene/solr/4.6.1/solr-4.6.1.zip" &&\
	unzip solr-4.6.1.zip &&\
	mkdir /var/www/fedora/solr &&\
	cp -r /tmp/solr/solr-4.6.1/example/solr/* /var/www/fedora/solr &&\
# following http://wiki.apache.org/solr/SolrLogging#Using_the_example_logging_setup_in_containers_other_than_Jetty
	cp -r /tmp/solr/solr-4.6.1/example/lib/ext/* /var/www/fedora/tomcat/lib &&\
# slf4j-log4j12-1.6.6.jar leads to clashes with fedoragsearch which contains the same class
# removing the jar leads to a no-operation logger implementation, see http://www.slf4j.org/codes.html#StaticLoggerBinder
	mv /var/www/fedora/tomcat/lib/slf4j-log4j12-1.6.6.jar /var/www/fedora/tomcat/lib/slf4j-log4j12-1.6.6.jar.BAK &&\
	cp /tmp/solr/solr-4.6.1/example/resources/log4j.properties /var/www/fedora/tomcat/lib &&\
	cp -r /tmp/solr/solr-4.6.1/example/webapps/solr.war /var/www/fedora/tomcat/webapps/solr.war &&\
	mkdir /var/www/fedora/tomcat/webapps/solr &&\
	cd /var/www/fedora/tomcat/webapps/solr/ &&\
	unzip ../solr.war &&\
	rm -r /tmp/solr

#
# Configuration
#

ADD fedora/fedora-users.xml /var/www/fedora/server/config/fedora-users.xml
ADD gsearch/fgsconfig-basic-for-islandora.properties /var/www/fedora/tomcat/webapps/fedoragsearch/FgsConfig/fgsconfig-basic-for-islandora.properties
RUN sed -i 's/fedoragsearch.xsltProcessor.*=.*xalan/fedoragsearch.xsltProcessor = saxon/g' /var/www/fedora/tomcat/webapps/fedoragsearch/FgsConfig/FgsConfigTemplate/fedoragsearch.properties
#RUN sed -i 's/fgsconfig-basic.properties/fgsconfig-basic-for-islandora.properties/g' /var/www/fedora/tomcat/webapps/fedoragsearch/FgsConfig/fgsconfig-basic.xml
RUN cd /var/www/fedora/tomcat/webapps/fedoragsearch/FgsConfig/ &&\
	ant -Dlocal.FEDORA_HOME=$FEDORA_HOME -f fgsconfig-basic.xml -propertyfile fgsconfig-basic-for-islandora.properties &&\
	mv "/var/www/fedora/solr/collection1/conf/schema.xml" "/var/www/fedora/solr/collection1/conf/schema.xml.bak" &&\
	cp "/var/www/fedora/tomcat/webapps/fedoragsearch/WEB-INF/classes/fgsconfigFinal/index/FgsIndex/conf/schema-4.6.1-for-fgs-2.7.xml" "/var/www/fedora/solr/collection1/conf/schema.xml"
ADD solr/solr.xml /var/www/fedora/tomcat/conf/Catalina/localhost/solr.xml

RUN mkdir /tmp/islandora &&\
	cd /tmp/islandora &&\
	wget "https://github.com/Islandora/islandora_solr_search/archive/7.x-1.5.zip" &&\
	mv 7.x-1.5.zip islandora_gsearch_module-7.x-1.5.zip &&\
	unzip islandora_gsearch_module-7.x-1.5.zip -d /var/www/html/drupal/sites/all/modules/contrib/

RUN cd /var/www/html/drupal/ &&\
	export PGPASSWORD=fedora &&\
	/etc/init.d/postgresql start &&\
	/var/www/fedora/tomcat/bin/startup.sh &&\
	bash -c "timeout 60 grep -q '(AuthFilterJAAS) initialised servlet filter: org.fcrepo.server.security.jaas.AuthFilterJAAS' <(tail -f $FEDORA_HOME/server/logs/fedora.log)" &&\
	/var/www/composer/vendor/drush/drush/drush en islandora_solr -y &&\
	/var/www/composer/vendor/drush/drush/drush en islandora_solr_config -y &&\
	/var/www/fedora/tomcat/bin/shutdown.sh &&\
	bash -c "timeout 60 grep -q 'Server shutdown complete' <(tail -f $FEDORA_HOME/server/logs/fedora.log)" &&\
	/etc/init.d/postgresql stop

#
# Add FLAT's own scripts and tools 
#

RUN mkdir -p /app/flat
ADD flat/scripts/* /app/flat/
RUN chmod +x /app/flat/*.sh

RUN mkdir -p /tmp/FindProfiles
ADD flat/FindProfiles /tmp/FindProfiles
RUN cd /tmp/FindProfiles &&\
    mvn package &&\
    mkdir -p /app/flat/lib &&\
    mv target/findProfiles.jar /app/flat/lib/findProfiles.jar
    
#Clean up APT when done.
RUN apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

EXPOSE 80
EXPOSE 443
EXPOSE 8443
EXPOSE 8080 
