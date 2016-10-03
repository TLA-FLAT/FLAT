FROM		flat
MAINTAINER	Menzo Windhouwer <menzo.windhouwer@meertens.knaw.nl>

RUN apt-get update &&\
	apt-get -y dist-upgrade

#
# easy-deposit
#

RUN cd /tmp &&\
    git clone https://github.com/DANS-KNAW/easy-deposit easy-deposit &&\
    cd /tmp/easy-deposit &&\
    # checkout version 1.0.2 (has no tag)
    git checkout 64378e6be6ff83994fd675eb61a6ea332b39ff67 &&\
    sed -i 's|https://easy.dans.knaw.nl/maven/|http://maven.dans.knaw.nl/|g' pom.xml &&\
    sed -i 's|DANS-EASY|FLAT|g' src/main/scala/nl/knaw/dans/api/sword2/StatementManagerImpl.scala &&\
    sed -i 's|DANS Default Data Collection|FLAT Deposit Bag Collection|g' src/main/scala/nl/knaw/dans/api/sword2/ServiceDocumentManagerImpl.scala

ADD sword/web.xml /tmp/easy-deposit/src/main/webapp/WEB-INF/web.xml

RUN cd /tmp/easy-deposit &&\
    mvn clean install &&\    
	cp target/easy-deposit.war /var/www/fedora/tomcat/webapps/easy-deposit.war &&\
	mkdir /var/www/fedora/tomcat/webapps/easy-deposit &&\
	cd /var/www/fedora/tomcat/webapps/easy-deposit/ &&\
	unzip ../easy-deposit.war &&\
	rm -r /tmp/easy-deposit
    
RUN mkdir -p /app/flat/deposit/sword/cfg &&\
    mkdir -p /app/flat/deposit/sword/tmp &&\
    mkdir -p /app/flat/deposit/bags
    
ADD sword/logback.xml /app/flat/deposit/sword/cfg/logback.xml
ADD sword/application.properties /app/flat/deposit/sword/cfg/application.properties
RUN sed -i "s|FLAT_HOST|${FLAT_HOST}|g" /app/flat/deposit/sword/cfg/application.properties
    
ENV EASY_DEPOSIT_HOME /app/flat/deposit/sword

#
# Add FLAT's own scripts and tools 
#

ADD flat/deposit/test/test-sip.zip /app/flat/deposit/test/test-sip.zip

RUN mkdir -p /app/flat
ADD flat/scripts/* /app/flat/
RUN sed -i "s|FLAT_HOST|${FLAT_HOST}|g" /app/flat/do-sword-upload.sh
RUN chmod +x /app/flat/*.sh

#Clean up APT when done.
RUN apt-get clean &&\
	rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*
