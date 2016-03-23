FROM        flat-with-sps
MAINTAINER	Daniel von Rhein <daniel.vonrhein@mpi.nl>

#Build xdebug library
RUN apt-get update &&\
    apt-get -y dist-upgrade &&\
    apt-get -y install php-pear &&\
    apt-get -y install php5-dev  &&\
    pecl install xdebug

#Add xdebug configuration to php.ini
COPY xdebug/xdebug.conf /tmp/
RUN cat /tmp/xdebug.conf >> /etc/php5/apache2/php.ini


#Add SSH root-account to docker image
RUN echo 'root:root' | chpasswd  &&\
    sed -i 's/PermitRootLogin without-password/PermitRootLogin yes/' /etc/ssh/sshd_config &&\
    sed 's@session\s*required\s*pam_loginuid.so@session optional pam_loginuid.so@g' -i /etc/pam.d/sshd

ENV NOTVISIBLE "in users profile"
RUN echo "export VISIBLE=now" >> /etc/profile

# Add deposit module to installation
RUN git clone https://github.com/danrhe/flat_deposit_ui.git /var/www/html/drupal/sites/all/modules/custom/flat_deposit_ui

# add WYSIWYG editor
RUN cd /var/www/html/drupal/sites/all/libraries/ &&\
    wget http://download.cksource.com/CKEditor/CKEditor/CKEditor%203.6.2/ckeditor_3.6.2.tar.gz &&\
    tar xzvf ckeditor_3.6.2.tar.gz

# add extra modules
RUN cd /var/www/html/drupal/ &&\
    export PGPASSWORD=fedora &&\
	/etc/init.d/postgresql start &&\
	/var/www/fedora/tomcat/bin/startup.sh &&\
	bash -c "timeout 60 grep -q '(AuthFilterJAAS) initialised servlet filter: org.fcrepo.server.security.jaas.AuthFilterJAAS' <(tail -f $FEDORA_HOME/server/logs/fedora.log)" &&\
    sleep 10 &&\
    /var/www/composer/vendor/drush/drush/drush dis toolbar -y &&\
    /var/www/composer/vendor/drush/drush/drush dl admin_menu -y &&\
    /var/www/composer/vendor/drush/drush/drush en admin_menu -y &&\
    /var/www/composer/vendor/drush/drush/drush dl wysiwyg -y  &&\
    /var/www/composer/vendor/drush/drush/drush en wysiwyg -y  &&\
    /var/www/composer/vendor/drush/drush/drush dl imce -y  &&\
    /var/www/composer/vendor/drush/drush/drush en imce -y  &&\
    /var/www/composer/vendor/drush/drush/drush dl imce_wysiwyg -y  &&\
    /var/www/composer/vendor/drush/drush/drush en imce_wysiwyg -y   &&\
    /var/www/composer/vendor/drush/drush/drush en flat_deposit_ui -y

# add module configuration
ENV PGPASSWORD=fedora
COPY drupal/wysiwyg.sql /tmp/wysiwyg.sql
COPY drupal/settings_imce.php /tmp/settings_imce.php

RUN /etc/init.d/postgresql start &&\
	cd /var/www/html/drupal/ &&\
    	/var/www/composer/vendor/drush/drush/drush sql-query "$(cat /tmp/wysiwyg.sql)" -y &&\
    	/var/www/composer/vendor/drush/drush/drush php-script settings_imce --script-path=/tmp &&\
#    	/var/www/composer/vendor/drush/drush/drush cache-clear all
	/etc/init.d/postgresql stop

# add owncloud to version
RUN wget -nv https://download.owncloud.org/download/repositories/stable/xUbuntu_14.04/Release.key -O Release.key &&\
	apt-key add - < Release.key

RUN sh -c "echo 'deb http://download.owncloud.org/download/repositories/stable/xUbuntu_14.04/ /' >> /etc/apt/sources.list.d/owncloud.list" &&\
	apt-get update &&\
	apt-get install -y owncloud

RUN /etc/init.d/postgresql start &&\
    su -c "createdb -O fedora owncloud" postgres &&\
    /etc/init.d/postgresql stop

RUN /etc/init.d/postgresql start &&\
	cd /var/www/owncloud &&\
    	sudo -u www-data php occ maintenance:install --database "pgsql" --database-name "owncloud" --database-user "fedora" --database-pass "fedora" --admin-user "admin" --admin-pass "admin"

COPY owncloud/addTrusted.php /tmp/addTrusted.php
RUN mv /var/www/owncloud/config/config.php /var/www/owncloud/config/config.backup &&\
    printf "<?php\n\$CONFIG = $(php /tmp/addTrusted.php) ;" >/var/www/owncloud/config/config.php


COPY owncloud/changeRights.sh /tmp/changeRights.sh
RUN /tmp/changeRights.sh


#Add bagit to image
RUN cd /app/flat &&\
	git clone https://github.com/LibraryOfCongress/bagit-java.git bagit &&\
	cd bagit &&\
	git checkout bagit-4.9.0 &&\
	mvn -Dmaven.test.skip=true clean install &&\
	cp target/bagit-4.9.0-bin.zip /usr/local/bin/ &&\
	cd /usr/local/bin/ &&\
	unzip bagit-4.9.0-bin.zip &&\
	PATH=/usr/local/bin/bagit-4.9.0/bin:$PATH &&\
	apt-get -y install zip unzip

#Clean up when done.
RUN apt-get clean &&\
  rm -rf /tmp/* /var/tmp/*

# Comment this line of code if you don't have a local installation of the deposit UI that is mounted as /var/www/html/drupal/sites/all/modules/custom/flat_deposit_ui
RUN rm -rf /var/www/html/drupal/sites/all/modules/custom/flat_deposit_ui

