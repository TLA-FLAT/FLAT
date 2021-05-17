FROM		flat

RUN apt-get update &&\
	apt-get -y dist-upgrade &&\
    apt-get -y install zip unzip

#
# easy-sword2
RUN cd /tmp &&\
    git clone https://github.com/DANS-KNAW/easy-sword2 easy-sword2 &&\
    cd /tmp/easy-sword2 &&\
    # checkout version 1.8.9
    git checkout v1.8.9 &&\
    sed -i 's|DANS-EASY|FLAT|g' src/main/scala/nl.knaw.dans.easy.sword2/StatementManagerImpl.scala &&\
    sed -i 's|DANS Default Data Collection|FLAT Deposit Bag Collection|g' src/main/scala/nl.knaw.dans.easy.sword2/ServiceDocumentManagerImpl.scala

RUN cd /tmp/easy-sword2 &&\
    mvn clean install assembly:single

RUN tar -xf /tmp/easy-sword2/target/easy-sword2-1.8.9.tar.gz -C /tmp/easy-sword2/ &&\    
    mkdir -p /app/easy-sword2 &&\
    mv /tmp/easy-sword2/easy-sword2-1.8.9/bin /app/easy-sword2 &&\
    mv /app/easy-sword2/bin/easy-sword2-1.8.9.jar /app/easy-sword2/bin/easy-sword2.jar &&\
    mv /tmp/easy-sword2/easy-sword2-1.8.9/lib /app/easy-sword2 &&\
    mv /tmp/easy-sword2/easy-sword2-1.8.9/cfg /app/easy-sword2/bin &&\
    mkdir /app/easy-sword2/cfg/ &&\
    rm -r /tmp/easy-sword2
    
RUN mkdir -p /app/flat/deposit/sword/tmp &&\
    mkdir -p /app/flat/deposit/bags
    
ADD sword/logback.xml /app/easy-sword2/cfg/logback.xml
ADD sword/application.properties /app/easy-sword2/cfg/application.properties
RUN sed -i "s|FLAT_HOST|${FLAT_HOST}|g" /app/easy-sword2/cfg/application.properties &&\
    sed -i "s|FLAT_NAME|${FLAT_NAME}|g" /app/easy-sword2/cfg/application.properties
    
ADD supervisor/supervisord-easy-deposit.conf /etc/supervisor/conf.d/supervisord-easy-deposit.conf
ADD supervisor/start.sh /start.sh
RUN	chmod u+x /start.sh

# Add proxy to Apache

RUN echo '# open up access to SWORD' >> /etc/apache2/apache2.conf &&\ 
    echo 'ProxyPass "/'${FLAT_NAME}'/easy-deposit" "http://localhost:8082"' >> /etc/apache2/apache2.conf

#
# Add bagit
#

RUN cd /tmp &&\
    wget https://github.com/LibraryOfCongress/bagit-java/releases/download/v4.12.3/bagit-v4.12.3.zip &&\
    cd /app/ &&\
    unzip /tmp/bagit-v4.12.3.zip &&\
    ln -s bagit-v4.12.3 bagit &&\
    ln -s /app/bagit/bin/bagit /app/flat/bin/bag

#
# Add FLAT's own scripts and tools 
ADD flat/scripts/* /app/flat/bin/
RUN chmod +x /app/flat/bin/*.sh

#Clean up APT when done.
RUN apt-get clean &&\
	rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*
