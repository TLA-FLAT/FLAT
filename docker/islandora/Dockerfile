# Fedora + Islandora, postgresql based 
#
# See:
#   https://github.com/phusion/baseimage-docker
# VERSION               0.0.1
#
# docker build -t islandora/v3 .
#
# docker run --rm -p 80:80 -p 8443:8443 -h fedora.test.lan -t -i --name=islandora islandora/v3 /sbin/my_init -- bash -l
#
# On mac run "boot2docker ip" to figure out where to reach your running containers.
# 192.168.59.103 seems to be the default.
#
# http://192.168.59.103/drupal/
#
# The installation of the fedora root collection and the solution packs need to be completed:
# - login
# - Islandora -> Solution packs
# - install top-level collection
# - Modules
# - enable 'Islandora basic collection' and 'Islandora compound object'. save configuration
#
# https://192.168.59.103:8443/fedora/admin/
#
# TODO: the Object XML can't be viewed
#
# Database user for "drupal" and "fedora" database:
#   fedora: fedora
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

#FROM      	ubuntu
FROM            phusion/baseimage
MAINTAINER	Willem Elbers <willem.elbers@mpi.nl>

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
RUN apt-get update
RUN apt-get -y dist-upgrade
RUN apt-get -y install curl wget openssl apache2 postgresql php5 php5-gd php5-pgsql php5-xsl php5-curl unzip default-jre libapache2-mod-jk expect ssh rsync

#
# Configure apache
#   - set apache ulimit to 1024
ADD apache/envvars /etc/apache2/envvars
ADD apache/apache2.conf /etc/apache2/apache2.conf
RUN mv /etc/apache2/mods-available/rewrite.load /etc/apache2/mods-enabled/rewrite.load

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
    su -c "createdb -O fedora --encoding=UNICODE -T template0 drupal" postgres

#
# Download packages for the islandora setup
#   - fedora commons
#   - drupal
#       - tuque
#   - islandora
#

RUN mkdir /tmp/fedora
RUN cd /tmp/fedora &&\ 
    wget "http://downloads.sourceforge.net/project/fedora-commons/fedora/3.7.1/fcrepo-installer-3.7.1.jar"

ADD fedora/install.properties /tmp/fedora/install.properties
RUN /etc/init.d/postgresql start &&\
    cd /tmp/fedora &&\
    java -jar fcrepo-installer-3.7.1.jar install.properties

#
# starting fedora will initialize required directories
#
ADD fedora/wait.sh /tmp/wait.sh
RUN chmod 0755 /tmp/wait.sh
RUN export FEDORA_HOME=/var/www/fedora/ &&\
    export CATALINA_HOME=/var/www/fedora/tomcat/ &&\
    /var/www/fedora/tomcat/bin/startup.sh &&\
    /tmp/wait.sh
RUN rm -v /var/www/fedora/data/fedora-xacml-policies/repository-policies/default/deny-purge-*
ADD fedora/permit-anything-to-localhost.xml /var/www/fedora/data/fedora-xacml-policies/repository-policies/default/permit-anything-to-localhost.xml
ADD fedora/deny-unallowed-file-resolution.xml /var/www/fedora/data/fedora-xacml-policies/repository-policies/default/deny-unallowed-file-resolution.xml
RUN mkdir /var/www/fedora/data/fedora-xacml-policies/repository-policies/islandora &&\
    cd /var/www/fedora/data/fedora-xacml-policies/repository-policies/islandora &&\
    wget https://raw.githubusercontent.com/Islandora/islandora/7.x-1.3/policies/permit-apim-to-authenticated-user.xml &&\
    wget https://raw.githubusercontent.com/Islandora/islandora/7.x-1.3/policies/permit-getDatastream-unrestricted.xml &&\
    wget https://raw.githubusercontent.com/Islandora/islandora/7.x-1.3/policies/permit-getDatastreamHistory-unrestricted.xml &&\
    wget https://raw.githubusercontent.com/Islandora/islandora/7.x-1.3/policies/permit-upload-to-authenticated-user.xml
RUN cd /var/www/fedora/tomcat/webapps/fedora/WEB-INF/lib &&\ 
  wget "https://github.com/Islandora/islandora_drupal_filter/releases/download/v7.1.3/fcrepo-drupalauthfilter-3.7.1.jar"
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
    /var/www/composer/vendor/drush/drush/drush site-install standard --account-name=admin --account-pass=admin --db-su=fedora --db-su-pw=fedora --db-url=pgsql://fedora:fedora@localhost/drupal -y

RUN chown www-data:www-data /var/www/html/drupal/sites/default/files &&\
    chown www-data:www-data /var/www/html/drupal/sites/default/settings.php

RUN mkdir /tmp/tuque &&\
    cd /tmp/tuque &&\
    wget "https://github.com/islandora/tuque/archive/1.3.zip" &&\
    unzip 1.3.zip &&\
    mkdir /var/www/html/drupal/sites/all/libraries &&\
    mv /tmp/tuque/tuque-1.3 /var/www/html/drupal/sites/all/libraries/tuque
ADD tuque/HttpConnection.php /var/www/html/drupal/sites/all/libraries/tuque/HttpConnection.php

RUN mkdir /tmp/islandora
RUN mkdir /var/www/html/drupal/sites/all/modules/contrib
RUN cd /tmp/islandora &&\
    wget "https://github.com/Islandora/islandora/archive/7.x-1.3.tar.gz" &&\
    tar -xf 7.x-1.3.tar.gz -C /var/www/html/drupal/sites/all/modules/contrib/
RUN cd /tmp/islandora &&\
	wget "https://github.com/islandora/islandora_solution_pack_collection/archive/7.x-1.3.zip" &&\
	mv 7.x-1.3.zip islandora_solution_pack_collection-7.x-1.3.zip &&\
	unzip islandora_solution_pack_collection-7.x-1.3.zip -d /var/www/html/drupal/sites/all/modules/contrib/
RUN cd /tmp/islandora &&\
	wget "https://github.com/Islandora/islandora_solution_pack_compound/archive/7.x-1.3.zip" &&\
	mv 7.x-1.3.zip islandora_solution_pack_compound-7.x-1.3.zip &&\
	unzip islandora_solution_pack_compound-7.x-1.3.zip -d /var/www/html/drupal/sites/all/modules/contrib/

RUN cd /var/www/html/drupal/ &&\
    export PGPASSWORD=fedora &&\
    /etc/init.d/postgresql start &&\
    export FEDORA_HOME=/var/www/fedora/ &&\
    export CATALINA_HOME=/var/www/fedora/tomcat/ &&\
    /var/www/fedora/tomcat/bin/startup.sh &&\
    sleep 10 &&\
    /var/www/composer/vendor/drush/drush/drush vset islandora_base_url https://localhost:8443/fedora &&\
    /var/www/composer/vendor/drush/drush/drush en islandora islandora_basic_collection islandora_compound_object -y &&\
    /var/www/fedora/tomcat/bin/shutdown.sh &&\
    sleep 10 &&\
    /etc/init.d/postgresql stop

#
# Add start script to automatically start required services
#

ADD fedora/tomcat-fedora.sh /var/www/fedora/tomcat/bin/tomcat-fedora.sh
RUN chmod u+x /var/www/fedora/tomcat/bin/tomcat-fedora.sh

ADD my_init.d/start.sh /etc/my_init.d/start.sh
RUN chmod u+x /etc/my_init.d/start.sh

#Clean up APT when done.
RUN apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

EXPOSE 80
EXPOSE 443
EXPOSE 8443
