FROM		flat
MAINTAINER	Menzo Windhouwer <menzo.windhouwer@meertens.knaw.nl>

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

RUN mkdir /tmp/solr &&\
	cd /tmp/solr &&\
	wget "https://archive.apache.org/dist/lucene/solr/4.6.1/solr-4.6.1.zip" &&\
	unzip solr-4.6.1.zip &&\
	mkdir /var/www/fedora/solr &&\
	cp -r /tmp/solr/solr-4.6.1/example/solr/* /var/www/fedora/solr &&\
	sed -i 's/\.\.\/\.\.\/\.\.\//\.\.\//g' /var/www/fedora/solr/collection1/conf/solrconfig.xml  &&\
	# based on http://wiki.apache.org/solr/SolrLogging#Using_the_example_logging_setup_in_containers_other_than_Jetty
	cp -r /tmp/solr/solr-4.6.1/dist/ /var/www/fedora/solr &&\
	cp -r /tmp/solr/solr-4.6.1/contrib/ /var/www/fedora/solr &&\
	cp -r /tmp/solr/solr-4.6.1/example/webapps/solr.war /var/www/fedora/tomcat/webapps/solr.war &&\
	mkdir /var/www/fedora/tomcat/webapps/solr &&\
	cd /var/www/fedora/tomcat/webapps/solr/ &&\
	unzip ../solr.war &&\
	# slf4j-log4j12-1.6.6.jar leads to clashes with fedoragsearch and fedora. So we copy the logging jars to the solr webapp lib instead of tomcat/lib
	cp -r /tmp/solr/solr-4.6.1/example/lib/ext/* WEB-INF/lib &&\
	mkdir WEB-INF/classes &&\
	cp /tmp/solr/solr-4.6.1/example/resources/log4j.properties WEB-INF/classes &&\
	# configure logging to use the same directory and format as fedora and fedoragsearch servlets
	sed -ie "s|logs\/|$FEDORA_HOME\/server\/logs\/|g" WEB-INF/classes/log4j.properties &&\
	sed -i 's/RollingFileAppender/DailyRollingFileAppender/g' WEB-INF/classes/log4j.properties &&\
	sed -i '/MaxFileSize/d' WEB-INF/classes/log4j.properties &&\
	sed -i '/MaxBackupIndex/d' WEB-INF/classes/log4j.properties &&\
	sed -i 's/\/solr\.log/\/solr\.daily\.log/g' WEB-INF/classes/log4j.properties &&\
	sed -i 's/, CONSOLE//g' WEB-INF/classes/log4j.properties &&\
	rm -r /tmp/solr

#
# Configuration
#

ADD fedora/fedora-users.xml /var/www/fedora/server/config/fedora-users.xml
ADD gsearch/fgsconfig-basic-for-islandora.properties /var/www/fedora/tomcat/webapps/fedoragsearch/FgsConfig/fgsconfig-basic-for-islandora.properties
RUN sed -i 's/fedoragsearch.xsltProcessor.*=.*xalan/fedoragsearch.xsltProcessor = saxon/g' /var/www/fedora/tomcat/webapps/fedoragsearch/FgsConfig/FgsConfigTemplate/fedoragsearch.properties &&\
#	sed -i 's/fgsconfig-basic.properties/fgsconfig-basic-for-islandora.properties/g' /var/www/fedora/tomcat/webapps/fedoragsearch/FgsConfig/fgsconfig-basic.xml &&\
	cd /var/www/fedora/tomcat/webapps/fedoragsearch/FgsConfig/ &&\
	ant -Dlocal.FEDORA_HOME=$FEDORA_HOME -f fgsconfig-basic.xml -propertyfile fgsconfig-basic-for-islandora.properties &&\
	mv "/var/www/fedora/solr/collection1/conf/schema.xml" "/var/www/fedora/solr/collection1/conf/schema.xml.bak" &&\
	cp "/var/www/fedora/tomcat/webapps/fedoragsearch/WEB-INF/classes/fgsconfigFinal/index/FgsIndex/conf/schema-4.6.1-for-fgs-2.7.xml" "/var/www/fedora/solr/collection1/conf/schema.xml"
ADD solr/solr.xml /var/www/fedora/tomcat/conf/Catalina/localhost/solr.xml

#
# Saxon
#

RUN mkdir /tmp/saxon &&\
	cd /tmp/saxon &&\ 
	wget -O SaxonHE9-6-0-7J.zip "http://downloads.sourceforge.net/project/saxon/Saxon-HE/9.6/SaxonHE9-6-0-7J.zip?r=&use_mirror=netix" &&\
	unzip "SaxonHE9-6-0-7J.zip" &&\
	mkdir -p /app/flat/lib/saxon &&\
	rm "SaxonHE9-6-0-7J.zip" &&\
	mv * /app/flat/lib/saxon/ &&\
	rm -r /tmp/saxon

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
RUN apt-get clean &&\
	rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*
