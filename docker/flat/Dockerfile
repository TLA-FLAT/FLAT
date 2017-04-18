FROM phusion/baseimage:0.9.19
MAINTAINER  Menzo Windhouwer <menzo.windhouwer@meertens.knaw.nl>

#
# Configure FLAT
#

ENV DRUPAL_PROJECT=flat
ENV TOMCAT_TIMEOUT=60
#ENV FLAT_HOST=192.168.99.100
ENV FLAT_HOST=localhost
ENV CMD_EXTENSION=cmdi
ENV FLAT_ICONS=/app/flat/data/icons


#
# Setup workdir
#

RUN mkdir -p /app/flat/lib &&\
	mkdir -p /app/flat/bin &&\
	mkdir -p /app/flat/tmp

WORKDIR /app/flat

ENV HOME /root
ENV PATH="/app/flat/bin:${PATH}"

# expose ports

EXPOSE 80
EXPOSE 8080
EXPOSE 8443

#
# set local timezone
#

RUN mv /etc/localtime /etc/localtime.bak &&\
    ln -sf /usr/share/zoneinfo/Europe/Amsterdam /etc/localtime

# Regenerate SSH host keys. baseimage-docker does not contain any, so you
# have to do that yourself. You may also comment out this instruction; the
# init system will auto-generate one during boot.
RUN /etc/my_init.d/00_regen_ssh_host_keys.sh

#
# Update and install all system libraries
#
RUN apt-get update &&\
	apt-get -y dist-upgrade &&\
	apt-get -y install curl wget openssl apache2 postgresql libapache2-mod-php php-gd php-pgsql php-xsl php-curl unzip default-jdk libapache2-mod-jk ssh rsync git maven libxml2-utils

#
# Configure apache
#   - set apache ulimit to 1024
ADD apache/envvars /etc/apache2/envvars
ADD apache/apache2.conf /etc/apache2/apache2.conf
RUN a2enmod ssl &&\
	a2enmod rewrite &&\
	a2enmod proxy &&\
	a2enmod proxy_http &&\
	a2enmod sed &&\
	sed -i "s|DRUPAL_PROJECT|${DRUPAL_PROJECT}|g" /etc/apache2/apache2.conf

#
# Configure postgres
#   - update postgresql.conf to bind to localhost
#   - create database user and update pg_hba_conf for this user
#   - initialise databases for fedora and drupal
#
# TODO: fix snakeoil key permission issue without copying
RUN cp /etc/ssl/certs/ssl-cert-snakeoil.pem /etc/postgresql/ssl-cert-snakeoil.pem &&\
	chown postgres:postgres /etc/postgresql/ssl-cert-snakeoil.pem &&\
	cp /etc/ssl/private/ssl-cert-snakeoil.key /etc/postgresql/ssl-cert-snakeoil.key &&\
	chown postgres:postgres /etc/postgresql/ssl-cert-snakeoil.key
ADD postgresql/pg_hba.conf /etc/postgresql/9.3/main/pg_hba.conf
ADD postgresql/postgresql.conf /etc/postgresql/9.3/main/postgresql.conf
RUN useradd -g postgres ssl-cert &&\
    /etc/init.d/postgresql start &&\
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

ENV FEDORA_HOME /var/www/fedora
ENV CATALINA_HOME /var/www/fedora/tomcat

RUN mkdir /tmp/fedora &&\
	cd /tmp/fedora &&\
	wget "https://github.com/fcrepo3/fcrepo/releases/download/v3.8.1/fcrepo-installer-3.8.1.jar"

ADD fedora/install.properties /tmp/fedora/install.properties
RUN /etc/init.d/postgresql start &&\
	cd /tmp/fedora &&\
	java -jar fcrepo-installer-3.8.1.jar install.properties &&\
	/etc/init.d/postgresql stop

# tell fedora about CMDI
RUN sed -i 's|</mappings>|<mime-mapping><mime-type>application/x-cmdi+xml</mime-type><extension>cmdi</extension></mime-mapping></mappings>|g' /var/www/fedora/server/config/mime-to-extensions.xml

#
# starting fedora will initialize required directories
#

ADD fedora/wait.sh /tmp/wait.sh
RUN chmod 0755 /tmp/wait.sh &&\
	/etc/init.d/postgresql start &&\
	/var/www/fedora/tomcat/bin/startup.sh &&\
	/tmp/wait.sh &&\
	bash -c "timeout $TOMCAT_TIMEOUT grep -q '(AuthFilterJAAS) initialised servlet filter: org.fcrepo.server.security.jaas.AuthFilterJAAS' <(tail -f $FEDORA_HOME/server/logs/fedora.log)" &&\
# apparantly the shutdown port is not directly available after startup
sleep 60 &&\
	/var/www/fedora/tomcat/bin/shutdown.sh &&\
	bash -c "timeout $TOMCAT_TIMEOUT grep -q 'Server shutdown complete' <(tail -f $FEDORA_HOME/server/logs/fedora.log)" &&\
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
	wget "https://github.com/Islandora/islandora_drupal_filter/releases/download/v7.1.3/fcrepo-drupalauthfilter-3.8.1.jar"
ADD fedora/jaas.conf /var/www/fedora/server/config/jaas.conf
ADD fedora/filter-drupal.xml /var/www/fedora/server/config/filter-drupal.xml

##Download and configure drupal (via drush)
RUN mkdir -p /var/www/composer &&\
	cd /var/www/composer &&\
	curl -sS https://getcomposer.org/installer | php &&\
	mv composer.phar composer
ENV COMPOSER_HOME=/var/www/composer
RUN $COMPOSER_HOME/composer global require drush/drush:7.*

RUN cd /var/www/html &&\
	/var/www/composer/vendor/drush/drush/drush -v dl drupal --drupal-project-rename=$DRUPAL_PROJECT
ENV PGPASSWORD=fedora
RUN /etc/init.d/postgresql start &&\
	cd /var/www/html/$DRUPAL_PROJECT &&\
	export PHP_OPTIONS="-d sendmail_path=`which true`" &&\
	/var/www/composer/vendor/drush/drush/drush site-install standard --site-name=FLAT --account-name=admin --account-pass=admin --db-su=fedora --db-su-pw=fedora --db-url=pgsql://fedora:fedora@localhost/drupal -y &&\
	/etc/init.d/postgresql stop

RUN mkdir -p /var/www/html/$DRUPAL_PROJECT/sites/all/modules/custom &&\
	chown www-data:www-data /var/www/html/$DRUPAL_PROJECT/sites/default/files &&\
	chown www-data:www-data /var/www/html/$DRUPAL_PROJECT/sites/default/settings.php &&\
	chown www-data:www-data /var/www/html/$DRUPAL_PROJECT/sites/all/modules/custom
    
ADD drupal/flat-logo.png /var/www/html/$DRUPAL_PROJECT/sites/default/files/flat-logo.png
ADD drupal/flat /var/www/html/$DRUPAL_PROJECT/sites/all/modules/custom/
ADD drupal/flat-logo.php /tmp/

RUN mkdir /tmp/tuque &&\
	cd /tmp/tuque &&\
	wget "https://github.com/islandora/tuque/archive/1.6.zip" &&\
	unzip 1.6.zip &&\
	mv /tmp/tuque/tuque-1.6 /var/www/html/$DRUPAL_PROJECT/sites/all/libraries/tuque &&\
	sed -i 's|public $verifyPeer = TRUE;|public $verifyPeer = FALSE;|g' /var/www/html/$DRUPAL_PROJECT/sites/all/libraries/tuque/HttpConnection.php

RUN mkdir -p /tmp/islandora &&\
	mkdir -p /var/www/html/$DRUPAL_PROJECT/sites/all/modules/contrib &&\
	mkdir -p /var/www/html/$DRUPAL_PROJECT/sites/all/themes &&\
	cd /tmp/islandora &&\
	wget "https://github.com/Islandora/islandora/archive/7.x-1.6.tar.gz" &&\
	tar -xf 7.x-1.6.tar.gz -C /var/www/html/$DRUPAL_PROJECT/sites/all/modules/contrib/ &&\
	cd /tmp/islandora &&\
	wget "https://github.com/islandora/islandora_solution_pack_collection/archive/7.x-1.6.zip" &&\
	mv 7.x-1.6.zip islandora_solution_pack_collection-7.x-1.6.zip &&\
	unzip islandora_solution_pack_collection-7.x-1.6.zip -d /var/www/html/$DRUPAL_PROJECT/sites/all/modules/contrib/ &&\
	cd /tmp/islandora &&\
	wget "https://github.com/Islandora/islandora_solution_pack_compound/archive/7.x-1.6.zip" &&\
	mv 7.x-1.6.zip islandora_solution_pack_compound-7.x-1.6.zip &&\
	unzip islandora_solution_pack_compound-7.x-1.6.zip -d /var/www/html/$DRUPAL_PROJECT/sites/all/modules/contrib/ &&\
    sed -i 's|function islandora_compound_object_islandora_compoundCModel_islandora_view_object|function islandora_compound_object_islandora_compoundCModel_islandora_view_object_DISABLED_BY_FLAT|g' /var/www/html/$DRUPAL_PROJECT/sites/all/modules/contrib/islandora_solution_pack_compound-7.x-1.6/islandora_compound_object.module
# overwrites the collection object view, so we get file details
ADD islandora/islandora_basic_collection.module /var/www/html/flat/sites/all/modules/contrib/islandora_solution_pack_collection-7.x-1.6/islandora_basic_collection.module

# add CMDI solution pack
RUN mkdir -p /var/www/html/$DRUPAL_PROJECT/sites/all/modules/contrib/islandora_solution_pack_cmdi
ADD flat/islandora_solution_pack_cmdi /var/www/html/$DRUPAL_PROJECT/sites/all/modules/contrib/islandora_solution_pack_cmdi

# tell islandora about CMDI
RUN sed -i "/\"json\"/a \"cmdi\" => \"application/x-cmdi+xml\"," /var/www/html/$DRUPAL_PROJECT/sites/all/modules/contrib/islandora-7.x-1.6/includes/mimetype.utils.inc &&\
	sed -i "/header('Content-type: ' . \$datastream/a if (\$datastream->ID == 'CMD') {\nheader('Content-type: ' . 'application/xml');\n}" /var/www/html/$DRUPAL_PROJECT/sites/all/modules/contrib/islandora-7.x-1.6/includes/datastream.inc
    
RUN mkdir -p /tmp/bootstrap &&\
	cd /tmp/bootstrap &&\
	wget "http://ftp.drupal.org/files/projects/bootstrap-7.x-3.8.zip" &&\
	unzip bootstrap-7.x-3.8.zip &&\
	mv /tmp/bootstrap/bootstrap /var/www/html/$DRUPAL_PROJECT/sites/all/themes/bootstrap

RUN cp -r /var/www/html/$DRUPAL_PROJECT/sites/all/themes/bootstrap/starterkits/cdn  /var/www/html/$DRUPAL_PROJECT/sites/all/themes/flat_bootstrap_theme &&\
	cd /var/www/html/$DRUPAL_PROJECT/sites/all/themes/flat_bootstrap_theme/ &&\
	mv cdn.starterkit flat_bootstrap_theme.info &&\
    echo ";; FLAT settings" >> flat_bootstrap_theme.info &&\
	echo "settings[bootstrap_cdn_jsdelivr_theme] = 'flatly'" >> flat_bootstrap_theme.info &&\
    echo "settings[bootstrap_fluid_container] = 1" >> flat_bootstrap_theme.info
# determine our own object view
ADD islandora/islandora-default.tpl.php /var/www/html/$DRUPAL_PROJECT/sites/all/themes/flat_bootstrap_theme/templates/islandora-default.tpl.php
# overwrite the compound behavior, which does special things for the first member (as being the representative)
ADD islandora/template.php /var/www/html/$DRUPAL_PROJECT/sites/all/themes/flat_bootstrap_theme/template.php

RUN export PGPASSWORD=fedora &&\
	/etc/init.d/postgresql start &&\
	/var/www/fedora/tomcat/bin/startup.sh &&\
	bash -c "timeout $TOMCAT_TIMEOUT grep -q '(AuthFilterJAAS) initialised servlet filter: org.fcrepo.server.security.jaas.AuthFilterJAAS' <(tail -f $FEDORA_HOME/server/logs/fedora.log)" &&\
sleep 10 &&\
	cd /var/www/html/$DRUPAL_PROJECT/ &&\
	/var/www/composer/vendor/drush/drush/drush vset islandora_base_url http://localhost:8080/fedora &&\
	/var/www/composer/vendor/drush/drush/drush en islandora -y &&\
	/var/www/composer/vendor/drush/drush/drush en islandora_basic_collection -y &&\
	/var/www/composer/vendor/drush/drush/drush en islandora_compound_object -y &&\
	/var/www/composer/vendor/drush/drush/drush en islandora_cmdi -y &&\
	/var/www/composer/vendor/drush/drush/drush vset islandora_metadata_display cmdi &&\
	/var/www/composer/vendor/drush/drush/drush dl features_extra -y &&\
	/var/www/composer/vendor/drush/drush/drush en flat -y &&\
	/var/www/composer/vendor/drush/drush/drush php-script flat-logo --script-path=/tmp &&\
	/var/www/composer/vendor/drush/drush/drush en jquery_update -y &&\
	/var/www/composer/vendor/drush/drush/drush en bootstrap -y &&\
	/var/www/composer/vendor/drush/drush/drush en flat_bootstrap_theme -y &&\
	/var/www/composer/vendor/drush/drush/drush vset theme_default flat_bootstrap_theme &&\
	/var/www/fedora/tomcat/bin/shutdown.sh &&\
	bash -c "timeout $TOMCAT_TIMEOUT grep -q 'Server shutdown complete' <(tail -f $FEDORA_HOME/server/logs/fedora.log)" &&\
	/etc/init.d/postgresql stop
    
# security patch for islandora
RUN export PGPASSWORD=fedora &&\
	/etc/init.d/postgresql start &&\
	/var/www/fedora/tomcat/bin/startup.sh &&\
	bash -c "timeout $TOMCAT_TIMEOUT grep -q '(AuthFilterJAAS) initialised servlet filter: org.fcrepo.server.security.jaas.AuthFilterJAAS' <(tail -f $FEDORA_HOME/server/logs/fedora.log)" &&\
sleep 10 &&\
	cd /tmp/islandora &&\
	git clone https://github.com/Islandora-Labs/islandora_patches.git &&\
	cd islandora_patches &&\
	mv disable_dl_clip /var/www/html/$DRUPAL_PROJECT/sites/all/modules/contrib/ &&\
	mv disable_print_endpoint  /var/www/html/$DRUPAL_PROJECT/sites/all/modules/contrib/ &&\
	cd /var/www/html/$DRUPAL_PROJECT/ &&\
	/var/www/composer/vendor/drush/drush/drush en disable_dl_clip -y &&\
	/var/www/composer/vendor/drush/drush/drush en disable_print_endpoint -y &&\
	/var/www/fedora/tomcat/bin/shutdown.sh &&\
	bash -c "timeout $TOMCAT_TIMEOUT grep -q 'Server shutdown complete' <(tail -f $FEDORA_HOME/server/logs/fedora.log)" &&\
	/etc/init.d/postgresql stop

#
# proai
#

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
# Add LAT2FOX
#

ENV MAVEN_OPTS -Xss10M

RUN cd /tmp &&\
    git clone -b develop https://github.com/TheLanguageArchive/SchemAnon.git SchemAnon &&\
    cd /tmp/SchemAnon &&\
    mvn clean install &&\
	mv target/SchemAnon.jar /app/flat/lib/SchemAnon.jar

RUN mkdir -p /tmp/LAT2FOX
ADD flat/LAT2FOX /tmp/LAT2FOX
RUN cd /tmp/LAT2FOX &&\
    mvn clean package &&\
    mv target/lat2fox.jar /app/flat/lib/lat2fox.jar
	
#
# Add FLAT's own scripts, tools and icons
#

ADD flat/scripts/* /app/flat/bin/
RUN chmod +x /app/flat/bin/*.sh &&\
	mv /app/flat/bin/do-*.sh /app/flat/

RUN mkdir -p /app/flat/policies
ADD flat/policies/default-resource-policy.xml /app/flat/deposit/policies/default-resource-policy.xml

RUN mkdir -p /app/flat/data/icons
ADD flat/icons $FLAT_ICONS
    
#
# Add start script to automatically start required services
#

ADD fedora/tomcat-fedora.sh /var/www/fedora/tomcat/bin/tomcat-fedora.sh
ADD my_init.d/start.sh /etc/my_init.d/start.sh
RUN chmod u+x /var/www/fedora/tomcat/bin/tomcat-fedora.sh &&\
	chmod u+x /etc/my_init.d/start.sh

# Use baseimage-docker's init system.
ENTRYPOINT ["/sbin/my_init"]
CMD ["--", "/bin/bash", "-l"]

#
# Cleanup
#

# clean up APT and /tmp when done.
RUN apt-get clean &&\
	rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*
