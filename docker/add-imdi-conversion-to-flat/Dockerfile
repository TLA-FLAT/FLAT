FROM        flat
MAINTAINER  Menzo Windhouwer <menzo.windhouwer@meertens.knaw.nl>

#
# Update and install all system libraries
#
RUN apt-get update &&\
	apt-get -y dist-upgrade

#
# Add FLAT's own scripts and tools 
#

RUN mkdir -p /app/flat
ADD flat/scripts/* /app/flat/
RUN chmod +x /app/flat/*.sh

ADD proai/proai.properties /var/www/fedora/tomcat/webapps/oaiprovider/WEB-INF/classes/proai.properties

RUN cd /tmp &&\
    git clone -b development https://github.com/TheLanguageArchive/MetadataTranslator.git MetadataTranslator &&\
    cd /tmp/MetadataTranslator &&\
    mvn clean install
    
RUN cd /tmp &&\
    wget https://nexus.clarin.eu/content/repositories/clarin-snapshot/eu/clarin/cmdi/cmdi-validator-core/0.0.6-SNAPSHOT/cmdi-validator-core-0.0.6-20160203.101033-2.jar &&\
    mvn install:install-file -Dfile=cmdi-validator-core-0.0.6-20160203.101033-2.jar -DgroupId=eu.clarin.cmdi -DartifactId=cmdi-validator-core -Dversion=0.0.6-SNAPSHOT -Dpackaging=jar
 
RUN cd /tmp &&\
    git clone https://github.com/TheLanguageArchive/ISLE2CLARIN.git ISLE2CLARIN &&\
    cd /tmp/ISLE2CLARIN &&\
    mvn clean package &&\
    mkdir -p /app/flat/lib &&\
    mv target/isle2clarin.jar /app/flat/lib/isle2clarin.jar
    
#Clean up APT when done.
RUN apt-get clean &&\
	rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*
