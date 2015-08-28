# Fedora + Islandora, postgresql based 
#
# See:
#   https://github.com/phusion/baseimage-docker
# VERSION               0.0.1
#
# docker build -t easylat .
#
# docker run --rm -p 80:80 -p 8443:8443 -h fedora.test.lan -t -i -name=easylat easylat /sbin/my_init -- bash -l
#
# On mac run "boot2docker ip" to figure out where to reach your running containers.
# 192.168.59.103 seems to be the default.
#
# http://192.168.59.103/drupal/
#
# https://192.168.59.103:8443/fedora/admin/
#
# TODO: the Object XML can't be viewed
#
# Database user for "drupal" and "fedora" database:
#   fedora:fedora
#
# Drupal admin account:
#    admin:admin
#
# Fedora admin account:
#    fedoraAdmin:fedora
#
# /var/www/fedora
# /var/www/html/drupal
#

FROM        phusion/baseimage
MAINTAINER  Menzo Windhouwer <menzo.windhouwer@meertens.knaw.nl>

# Set correct environment variables.
ENV HOME /root

# Regenerate SSH host keys. baseimage-docker does not contain any, so you
# have to do that yourself. You may also comment out this instruction; the
# init system will auto-generate one during boot.
RUN /etc/my_init.d/00_regen_ssh_host_keys.sh

# Use baseimage-docker's init system.
CMD ["/sbin/my_init"]

#
# Update and install all system libraries
#
RUN apt-get update &&\
    apt-get -y dist-upgrade &&\
    apt-get -y install curl wget openssl apache2 postgresql php5 php5-gd php5-pgsql php5-xsl php5-curl unzip default-jdk libapache2-mod-jk expect ssh rsync

#
# Configure apache
#   - set apache ulimit to 1024
ADD apache/envvars /etc/apache2/envvars
ADD apache/apache2.conf /etc/apache2/apache2.conf
RUN cd /etc/apache2/mods-enabled &&\
    ln -s ../mods-available/rewrite.load . &&\
    ln -s ../mods-available/proxy.conf . &&\
    ln -s ../mods-available/proxy.load . &&\
    ln -s ../mods-available/proxy_http.load .

#
# Configure postgres
#   - update postgresql.conf to bind to localhost
#   - create database user and update pg_hba_conf for this user
#   - initialise databases for fedora and drupal
#
# TODO: fix snakeoil key permission issue without copying
RUN cp /etc/ssl/certs/ssl-cert-snakeoil.pem /etc/postgresql/ssl-cert-snakeoil.pem &&\
    chown postgres:postgres /etc/postgresql/ssl-cert-snakeoil.pem
RUN cp /etc/ssl/private/ssl-cert-snakeoil.key /etc/postgresql/ssl-cert-snakeoil.key &&\
    chown postgres:postgres /etc/postgresql/ssl-cert-snakeoil.key
ADD postgresql/pg_hba.conf /etc/postgresql/9.3/main/pg_hba.conf
ADD postgresql/postgresql.conf /etc/postgresql/9.3/main/postgresql.conf
RUN useradd -g postgres ssl-cert
RUN /etc/init.d/postgresql start &&\
    su -c "psql -c \"CREATE USER fedora WITH PASSWORD 'fedora'\"" postgres &&\
    su -c "createdb -O fedora fedora" postgres &&\
    su -c "createdb -O fedora proai" postgres &&\
    su -c "createdb -O fedora --encoding=UNICODE -T template0 drupal" postgres &&\
    /etc/init.d/postgresql stop

#
# Download packages for the islandora setup
#   - fedora commons
#   - proai
#   - drupal
#       - tuque
#   - islandora
#

RUN mkdir /tmp/fedora
RUN cd /tmp/fedora &&\ 
    wget "http://downloads.sourceforge.net/project/fedora-commons/fedora/3.8.0/fcrepo-installer-3.8.0.jar"

ADD fedora/install.properties /tmp/fedora/install.properties
RUN /etc/init.d/postgresql start &&\
    cd /tmp/fedora &&\
    java -jar fcrepo-installer-3.8.0.jar install.properties &&\
    /etc/init.d/postgresql stop

#
# starting fedora will initialize required directories
#
ADD fedora/wait.sh /tmp/wait.sh
RUN chmod 0755 /tmp/wait.sh
RUN /etc/init.d/postgresql start &&\
    export FEDORA_HOME=/var/www/fedora/ &&\
    export CATALINA_HOME=/var/www/fedora/tomcat/ &&\
    /var/www/fedora/tomcat/bin/startup.sh &&\
    /tmp/wait.sh &&\
    /var/www/fedora/tomcat/bin/shutdown.sh &&\
    sleep 10 &&\
    /etc/init.d/postgresql stop
RUN rm -v /var/www/fedora/data/fedora-xacml-policies/repository-policies/default/deny-purge-*
ADD fedora/permit-anything-to-localhost.xml /var/www/fedora/data/fedora-xacml-policies/repository-policies/default/permit-anything-to-localhost.xml
ADD fedora/deny-unallowed-file-resolution.xml /var/www/fedora/data/fedora-xacml-policies/repository-policies/default/deny-unallowed-file-resolution.xml
RUN cd /var/www/fedora/data/fedora-xacml-policies/repository-policies/ &&\
    wget https://github.com/Islandora/islandora-xacml-policies/archive/master.zip &&\
    unzip master.zip &&\
    rm master.zip &&\
    mv islandora-xacml-policies-master islandora
RUN cd /var/www/fedora/tomcat/webapps/fedora/WEB-INF/lib &&\ 
  wget "https://github.com/Islandora/islandora_drupal_filter/releases/download/v7.1.3/fcrepo-drupalauthfilter-3.8.0.jar"
ADD fedora/jaas.conf /var/www/fedora/server/config/jaas.conf
ADD fedora/filter-drupal.xml /var/www/fedora/server/config/filter-drupal.xml

##Download and configure drupal (via drush)
RUN mkdir -p /var/www/composer
RUN cd /var/www/composer &&\
    curl -sS https://getcomposer.org/installer | php &&\
    mv composer.phar composer
ENV COMPOSER_HOME=/var/www/composer
RUN $COMPOSER_HOME/composer global require drush/drush:6.*

RUN cd /var/www/html &&\
    /var/www/composer/vendor/drush/drush/drush dl drupal --drupal-project-rename=drupal
ENV PGPASSWORD=fedora
RUN /etc/init.d/postgresql start &&\
    cd /var/www/html/drupal &&\
    export PHP_OPTIONS="-d sendmail_path=`which true`" &&\
    /var/www/composer/vendor/drush/drush/drush site-install standard --site-name=EasyLAT --account-name=admin --account-pass=admin --db-su=fedora --db-su-pw=fedora --db-url=pgsql://fedora:fedora@localhost/drupal -y &&\
    /etc/init.d/postgresql stop

RUN mkdir -p /var/www/html/drupal/sites/all/modules/custom
RUN chown www-data:www-data /var/www/html/drupal/sites/default/files &&\
    chown www-data:www-data /var/www/html/drupal/sites/default/settings.php &&\
    chown www-data:www-data /var/www/html/drupal/sites/all/modules/custom
    
ADD drupal/easylat-logo.png /var/www/html/drupal/sites/default/files/easylat-logo.png
ADD drupal/easylat /var/www/html/drupal/sites/all/modules/custom/
ADD drupal/easylat-logo.php /tmp/

RUN mkdir /tmp/tuque &&\
    cd /tmp/tuque &&\
    wget "https://github.com/islandora/tuque/archive/1.4.zip" &&\
    unzip 1.4.zip &&\
    mv /tmp/tuque/tuque-1.4 /var/www/html/drupal/sites/all/libraries/tuque
RUN sed -i 's|public $verifyPeer = TRUE;|public $verifyPeer = FALSE;|g' /var/www/html/drupal/sites/all/libraries/tuque/HttpConnection.php

RUN mkdir /tmp/islandora
RUN mkdir /var/www/html/drupal/sites/all/modules/contrib
RUN cd /tmp/islandora &&\
    wget "https://github.com/Islandora/islandora/archive/7.x-1.5.tar.gz" &&\
    tar -xf 7.x-1.5.tar.gz -C /var/www/html/drupal/sites/all/modules/contrib/
RUN cd /tmp/islandora &&\
	wget "https://github.com/islandora/islandora_solution_pack_collection/archive/7.x-1.5.zip" &&\
	mv 7.x-1.5.zip islandora_solution_pack_collection-7.x-1.5.zip &&\
	unzip islandora_solution_pack_collection-7.x-1.5.zip -d /var/www/html/drupal/sites/all/modules/contrib/
RUN cd /tmp/islandora &&\
	wget "https://github.com/Islandora/islandora_solution_pack_compound/archive/7.x-1.5.zip" &&\
	mv 7.x-1.5.zip islandora_solution_pack_compound-7.x-1.5.zip &&\
	unzip islandora_solution_pack_compound-7.x-1.5.zip -d /var/www/html/drupal/sites/all/modules/contrib/
RUN cd /tmp/islandora &&\
    wget "https://github.com/TheLanguageArchive/EasyLAT/archive/master.zip" &&\
    mv master.zip EasyLAT.zip &&\
    unzip EasyLAT.zip 'EasyLAT-master/islandora/islandora_solution_pack_cmdi/*' &&\
    mv EasyLAT-master/islandora/islandora_solution_pack_cmdi /var/www/html/drupal/sites/all/modules/contrib/
    
RUN mkdir /tmp/feature
RUN cd /tmp/feature &&\
	wget "http://ftp.drupal.org/files/projects/features-7.x-2.5.zip" &&\
	unzip features-7.x-2.5.zip -d /var/www/html/drupal/sites/all/modules/contrib/
RUN cd /tmp/feature &&\
	wget "http://ftp.drupal.org/files/projects/features_extra-7.x-1.0-beta1.zip" &&\
	unzip features_extra-7.x-1.0-beta1.zip -d /var/www/html/drupal/sites/all/modules/contrib/

RUN cd /var/www/html/drupal/ &&\
    export PGPASSWORD=fedora &&\
    /etc/init.d/postgresql start &&\
    export FEDORA_HOME=/var/www/fedora/ &&\
    export CATALINA_HOME=/var/www/fedora/tomcat/ &&\
    /var/www/fedora/tomcat/bin/startup.sh &&\
    sleep 60 &&\
#    cat /var/www/fedora/server/logs/fedora.log &&\
    /var/www/composer/vendor/drush/drush/drush vset islandora_base_url https://localhost:8443/fedora &&\
    /var/www/composer/vendor/drush/drush/drush en islandora -y &&\
    /var/www/composer/vendor/drush/drush/drush en islandora_basic_collection -y &&\
    /var/www/composer/vendor/drush/drush/drush en islandora_compound_object -y &&\
    /var/www/composer/vendor/drush/drush/drush en islandora_cmdi -y &&\
    /var/www/composer/vendor/drush/drush/drush vset islandora_metadata_display cmdi &&\
    /var/www/composer/vendor/drush/drush/drush en features -y &&\
    /var/www/composer/vendor/drush/drush/drush en fe_block -y &&\
    /var/www/composer/vendor/drush/drush/drush en easylat -y &&\
    /var/www/composer/vendor/drush/drush/drush php-script easylat-logo --script-path=/tmp &&\
    /var/www/fedora/tomcat/bin/shutdown.sh &&\
    sleep 10 &&\
    /etc/init.d/postgresql stop
    
# add the generated certificate for localhost
ADD cert/InstallCert.jar /tmp/InstallCert.jar
ADD cert/InstallCert.sh /tmp/InstallCert.sh
RUN chmod +x /tmp/InstallCert.sh
RUN cd /opt &&\
    /etc/init.d/postgresql start &&\
    export FEDORA_HOME=/var/www/fedora/ &&\
    export CATALINA_HOME=/var/www/fedora/tomcat/ &&\
    /var/www/fedora/tomcat/bin/startup.sh &&\
    sleep 60 &&\
    /tmp/InstallCert.sh &&\
    /var/www/fedora/tomcat/bin/shutdown.sh &&\
    sleep 10 &&\
    /etc/init.d/postgresql stop

##Download proai
RUN mkdir /tmp/proai &&\
    cd /tmp/proai &&\
    wget http://downloads.sourceforge.net/project/fedora-commons/services/3.4/oaiprovider-1.2.2.zip &&\
    unzip oaiprovider-1.2.2.zip &&\
    cp oaiprovider-1.2.2/oaiprovider.war /var/www/fedora/tomcat/webapps/oaiprovider.war &&\
    mkdir -p /var/www/fedora/tomcat/webapps/oaiprovider &&\
    cd /var/www/fedora/tomcat/webapps/oaiprovider &&\
    jar xf ../oaiprovider.war
ADD proai/proai.properties /var/www/fedora/tomcat/webapps/oaiprovider/WEB-INF/classes/proai.properties
ADD proai/identify.xml /var/www/html/identify.xml

#
# Add start script to automatically start required services
#

ENV JAVA_OPTS "-Djavax.net.ssl.trustStore=/opt/jssecacerts -Djavax.net.ssl.trustStorePassword=changeit"

ADD fedora/tomcat-fedora.sh /var/www/fedora/tomcat/bin/tomcat-fedora.sh
RUN chmod u+x /var/www/fedora/tomcat/bin/tomcat-fedora.sh

ADD my_init.d/start.sh /etc/my_init.d/start.sh
RUN chmod u+x /etc/my_init.d/start.sh

#Clean up APT when done.
RUN apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

EXPOSE 80
EXPOSE 443
EXPOSE 8080
EXPOSE 8443
