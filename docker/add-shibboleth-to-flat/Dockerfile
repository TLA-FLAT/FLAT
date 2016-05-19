FROM		flat 
MAINTAINER	Menzo Windhouwer <menzo.windhouwer@meertens.knaw.nl>

RUN mkdir -p /tmp/islandora &&\
	mkdir -p /var/www/html/drupal/sites/all/modules/contrib

# mod_shib

RUN apt-get update &&\
    apt-get -y dist-upgrade &&\
    apt-get -y install libapache2-mod-shib2

RUN cd /var/www/html/$DRUPAL_PROJECT/ &&\
	export PGPASSWORD=fedora &&\
	/etc/init.d/postgresql start &&\
	/var/www/fedora/tomcat/bin/startup.sh &&\
	bash -c "timeout $TOMCAT_TIMEOUT grep -q '(AuthFilterJAAS) initialised servlet filter: org.fcrepo.server.security.jaas.AuthFilterJAAS' <(tail -f $FEDORA_HOME/server/logs/fedora.log)" &&\
	/var/www/composer/vendor/drush/drush/drush en shib_auth -y &&\
	/var/www/composer/vendor/drush/drush/drush vset shib_auth_full_handler_url "http://192.168.99.100/Shibboleth.sso/Login" &&\
	/var/www/composer/vendor/drush/drush/drush vset shib_auth_full_logout_url "http://192.168.99.100/Shibboleth.sso/Logout" &&\
	/var/www/composer/vendor/drush/drush/drush vset shib_auth_enable_custom_mail "1" &&\
	/var/www/composer/vendor/drush/drush/drush vset shib_auth_login_url "/islandora" &&\
	/var/www/composer/vendor/drush/drush/drush sqlq "INSERT INTO shib_auth (field,regexpression,role,sticky) VALUES ('REMOTE_USER','.%2B','a:1:{i:0;s:1:"'"2"'";}',1);" &&\
sleep 10 &&\
	/var/www/fedora/tomcat/bin/shutdown.sh &&\
	bash -c "timeout $TOMCAT_TIMEOUT grep -q 'Server shutdown complete' <(tail -f $FEDORA_HOME/server/logs/fedora.log)" &&\
	/etc/init.d/postgresql stop
    
RUN echo "/etc/init.d/shibd start" >> /etc/my_init.d/start.sh

#Clean up APT when done.
RUN apt-get clean &&\
	rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*
