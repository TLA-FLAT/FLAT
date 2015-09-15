#
# Based on the EasyLat islandora base image
# See:  https://github.com/TheLanguageArchive/EasyLAT/tree/master/docker/easylat

# Building the image:
#   docker build -t easylat-with-shibboleth .
#
# Running the image:
#   docker run -i -p 80:80 -p 8443:8443 -p 8080:8080 -h fedora.test.lan -t easylat-with-shibboleth /sbin/my_init -- bash -l
#
# NOTE: further configuration in /etc/shibboleth will be needed to create an SP and connect it to one or more IdPs
#
FROM        easylat
MAINTAINER	Menzo Windhouwer <menzo.windhouwer@meertens.knaw.nl>

ENV FEDORA_HOME /var/www/fedora/
ENV CATALINA_HOME /var/www/fedora/tomcat/

RUN apt-get clean &&\
    rm -rf /var/lib/apt/lists/*

RUN mkdir -p /tmp/islandora
RUN mkdir -p /var/www/html/drupal/sites/all/modules/contrib

# mod_shib

RUN apt-get update &&\
    apt-get -y dist-upgrade &&\
    apt-get -y install libapache2-mod-shib2

# drupal shibboleth module
RUN cd /tmp/islandora &&\
    wget "http://ftp.drupal.org/files/projects/shib_auth-7.x-4.3.tar.gz" &&\
    tar -xf shib_auth-7.x-4.3.tar.gz -C /var/www/html/drupal/sites/all/modules/contrib/

RUN cd /var/www/html/drupal/ &&\
    export PGPASSWORD=fedora &&\
    /etc/init.d/postgresql start &&\
    export FEDORA_HOME=/var/www/fedora/ &&\
    export CATALINA_HOME=/var/www/fedora/tomcat/ &&\
    /var/www/fedora/tomcat/bin/startup.sh &&\
    sleep 60 &&\
    /var/www/composer/vendor/drush/drush/drush en shib_auth -y &&\
    /var/www/composer/vendor/drush/drush/drush vset shib_auth_full_handler_url "http://192.168.99.100/Shibboleth.sso/Login" &&\
    /var/www/composer/vendor/drush/drush/drush vset shib_auth_full_logout_url "http://192.168.99.100/Shibboleth.sso/Logout" &&\
    /var/www/composer/vendor/drush/drush/drush vset shib_auth_enable_custom_mail "1" &&\
    /var/www/composer/vendor/drush/drush/drush vset shib_auth_login_url "/islandora" &&\
    /var/www/composer/vendor/drush/drush/drush sqlq "INSERT INTO shib_auth (field,regexpression,role,sticky) VALUES ('REMOTE_USER','.%2B','a:1:{i:0;s:1:"'"2"'";}',1);" &&\
    /var/www/fedora/tomcat/bin/shutdown.sh &&\
    sleep 10 &&\
    /etc/init.d/postgresql stop

#Clean up APT when done.
RUN apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

EXPOSE 80
EXPOSE 443
EXPOSE 8443
EXPOSE 8080 
