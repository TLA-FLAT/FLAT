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

ADD flat/lib/xercesImpl-patched-2.11.0.jar /tmp/xercesImpl-patched-2.11.0.jar
RUN cd /tmp &&\
    mvn install:install-file -Dfile=xercesImpl-patched-2.11.0.jar -DgroupId=xerces -DartifactId=xercesImpl-patched -Dversion=2.11.0 -Dpackaging=jar
 
RUN cd /tmp &&\
    git clone https://github.com/TheLanguageArchive/ISLE2CLARIN.git ISLE2CLARIN &&\
    cd /tmp/ISLE2CLARIN &&\
    mvn clean package &&\
    mkdir -p /app/flat/lib &&\
    mv target/isle2clarin.jar /app/flat/lib/isle2clarin.jar
    
#Clean up APT when done.
RUN apt-get clean &&\
	rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*
