FROM		flat
MAINTAINER	Menzo Windhouwer <menzo.windhouwer@meertens.knaw.nl>

RUN apt-get update &&\
	apt-get -y dist-upgrade &&\
	apt-get -y install file
# mediainfo

#
# FITS
#

RUN mkdir /tmp/fits &&\
    cd /tmp/fits &&\
    wget http://projects.iq.harvard.edu/files/fits/files/fits-1.0.2.zip &&\
    unzip fits-1.0.2.zip &&\
    mv fits-1.0.2 /app/fits &&\
    mkdir /app/fits/log-lib &&\
    mv /app/fits/lib/slf4j-log4j12-1.7.12.jar /app/fits/log-lib &&\
    sed -i 's|APPCLASSPATH=""|APPCLASSPATH="${FITS_HOME}/log-lib/slf4j-log4j12-1.7.12.jar"|g' /app/fits/fits-env.sh &&\
    chmod +x /app/fits/*.sh &&\
    wget http://projects.iq.harvard.edu/files/fits/files/fits-1.1.3.war &&\
    cp fits-1.1.3.war /var/www/fedora/tomcat/webapps/fits.war &&\
    mkdir /var/www/fedora/tomcat/webapps/fits &&\
    cd /var/www/fedora/tomcat/webapps/fits &&\
    unzip -n ../fits.war &&\
    echo "fits.home=/app/fits" >> $CATALINA_HOME/conf/catalina.properties &&\
    echo 'shared.loader=${fits.home}/lib/*.jar' >> $CATALINA_HOME/conf/catalina.properties

#
# The FLAT Doorkeeper
#

RUN cd /tmp &&\
    git clone https://github.com/menzowindhouwer/fedora-cargo-plugin.git fedora-cargo-plugin &&\
    cd /tmp/fedora-cargo-plugin &&\
    mvn clean install

RUN cd /tmp &&\
    git clone https://github.com/menzowindhouwer/fedora-client.git fedora-client &&\
    cd /tmp/fedora-client &&\
    mvn clean install

RUN cd /tmp &&\
    git clone -b develop https://github.com/TLA-FLAT/DoorKeeper.git DoorKeeper &&\
    cd /tmp/DoorKeeper &&\
    mvn -Dmaven.test.skip=true clean install &&\
    cp target/doorkeeper.jar /app/flat/lib/

ADD flat/deposit/ServiceFLAT /tmp/ServiceFLAT
RUN cd /tmp/ServiceFLAT &&\
    mvn clean install &&\
    cp target/flat.war /var/www/fedora/tomcat/webapps/flat.war &&\
    mkdir /var/www/fedora/tomcat/webapps/flat &&\
    cd /var/www/fedora/tomcat/webapps/flat &&\
    unzip ../flat.war
    
RUN mkdir -p /app/flat/deposit/easy
ADD flat/deposit/flat-deposit.xml /app/flat/deposit/flat-deposit.xml
ADD flat/deposit/policies /app/flat/deposit/policies
ADD flat/deposit/dummies /app/flat/deposit/dummies
ADD flat/deposit/test /app/flat/deposit/test
RUN mkdir -p /app/flat/deposit/bags &&\
    mkdir -p /app/flat/deposit/transforms &&\
    cd /app/flat/deposit/transforms &&\
    jar xf /app/flat/lib/lat2fox.jar cmd2fox.xsl

RUN mkdir -p /app/flat
ADD flat/scripts/* /app/flat/
RUN chmod +x /app/flat/*.sh

ENV CATALINA_OPTS "-Djsse.enableSNIExtension=false"

#Clean up APT when done.
RUN apt-get clean &&\
    rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*
