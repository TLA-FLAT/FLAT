#
# Based on the EasyLat islandora base image
# See:  https://github.com/TheLanguageArchive/EasyLAT/tree/master/docker/islandora
#       https://wiki.duraspace.org/display/ISLANDORA713/Installing+Solr+and+GSearch

# Versions:
#   GSearch 2.6     http://192.168.59.103:8080/fedoragsearch/rest
#   Solr 4.2.0      http://192.168.59.103:8080/solr/
#       version 4.10.2 was giving issues about index version being too new
#   Islandora_solr_search 7.x-1.3
#
# Building the image:
#   docker build -t easylat/islandora-gsearch .
#
# Running the image:
#   docker run -i -p 80:80 -p 8443:8443 -p 8080:8080 -h fedora.test.lan -t easylat/islandora-gsearch:latest /sbin/my_init -- bash -l
#
#
FROM           tlatest04.mpi.nl:5000/islandora:latest
MAINTAINER	Willem Elbers <willem.elbers@mpi.nl>

ENV FEDORA_HOME /var/www/fedora/
ENV CATALINA_HOME /var/www/fedora/tomcat/

RUN apt-get update
RUN apt-get -y install ant

#
#Fedora GSearch
#

RUN mkdir /tmp/fedora-gsearch &&\
    cd /tmp/fedora-gsearch &&\ 
    wget -O genericsearch-2.6.zip "http://downloads.sourceforge.net/project/fedora-commons/services/3.6/fedoragsearch-2.6.zip?r=&use_mirror=heanet" &&\
    unzip "genericsearch-2.6.zip" &&\
    cp "fedoragsearch-2.6/fedoragsearch.war" "/var/www/fedora/tomcat/webapps/fedoragsearch.war" &&\
    mkdir /var/www/fedora/tomcat/webapps/fedoragsearch &&\
    cd /var/www/fedora/tomcat/webapps/fedoragsearch/ &&\
    unzip ../fedoragsearch.war &&\
    rm -r /tmp/fedora-gsearch

#
# SOLR
#

RUN mkdir /tmp/solr
RUN cd /tmp/solr &&\
    wget "https://archive.apache.org/dist/lucene/solr/4.2.0/solr-4.2.0.zip" &&\
    unzip solr-4.2.0.zip &&\
    mkdir /var/www/fedora/solr &&\
    #cp /tmp/solr/solr-4.2.0/dist/*.war /var/www/solr &&\
    cp -r /tmp/solr/solr-4.2.0/example/solr/* /var/www/fedora/solr &&\
    cp -r /tmp/solr/solr-4.2.0/example/webapps/solr.war /var/www/fedora/tomcat/webapps/solr.war &&\
    mkdir /var/www/fedora/tomcat/webapps/solr &&\
    cd /var/www/fedora/tomcat/webapps/solr/ &&\
    unzip ../solr.war &&\
    rm -r /tmp/solr

#
# Configuration
#
ADD fedora/fedora-users.xml /var/www/fedora/server/config/fedora-users.xml
ADD gsearch/fgsconfig-basic-for-islandora.properties /var/www/fedora/tomcat/webapps/fedoragsearch/FgsConfig/fgsconfig-basic-for-islandora.properties
RUN sed -i 's/fgsconfig-basic.properties/fgsconfig-basic-for-islandora.properties/g' /var/www/fedora/tomcat/webapps/fedoragsearch/FgsConfig/fgsconfig-basic.xml
RUN cd /var/www/fedora/tomcat/webapps/fedoragsearch/FgsConfig/ &&\
    ant -f fgsconfig-basic.xml &&\
    mv "/var/www/fedora/solr/collection1/conf/schema.xml" "/var/www/fedora/solr/collection1/conf/schema.xml.bak" &&\
    cp "/var/www/fedora/tomcat/webapps/fedoragsearch/WEB-INF/classes/fgsconfigFinal/index/FgsIndex/conf/schema-4.2.0-for-fgs-2.6.xml" "/var/www/fedora/solr/collection1/conf/schema.xml"
ADD solr/solr.xml /var/www/fedora/tomcat/conf/Catalina/localhost/solr.xml

RUN     mkdir /tmp/islandora &&\
        cd /tmp/islandora &&\
	wget "https://github.com/Islandora/islandora_solr_search/archive/7.x-1.3.zip" &&\
	mv 7.x-1.3.zip islandora_gsearch_module-7.x-1.3.zip &&\
	unzip islandora_gsearch_module-7.x-1.3.zip -d /var/www/html/drupal/sites/all/modules/contrib/

#Clean up APT when done.
RUN apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

EXPOSE 80
EXPOSE 443
EXPOSE 8443
EXPOSE 8080 