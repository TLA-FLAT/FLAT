FROM		flat
MAINTAINER	Menzo Windhouwer <menzo.windhouwer@meertens.knaw.nl>

RUN apt-get update &&\
	apt-get -y dist-upgrade &&\
	update-ca-certificates -f &&\
	apt-get install --reinstall ca-certificates-java 

# add sample SIP
RUN mkdir -p /app/flat/test
ADD flat/deposit/test/*.* /app/flat/test/
ADD flat/deposit/test/cmd /app/flat/test/cmd
ADD flat/deposit/test/data /app/flat/test/data
ADD flat/deposit/test/test-sip /app/flat/test/test-sip

# add sample user(s)
ENV TOMCAT_TIMEOUT=600
RUN	cd /var/www/html/$DRUPAL_PROJECT/ &&\
	export PGPASSWORD=fedora &&\
	/etc/init.d/postgresql start &&\
	/var/www/fedora/tomcat/bin/startup.sh &&\
	bash -c "timeout $TOMCAT_TIMEOUT grep -q '(AuthFilterJAAS) initialised servlet filter: org.fcrepo.server.security.jaas.AuthFilterJAAS' <(tail -f $FEDORA_HOME/server/logs/fedora.log)" &&\
	echo "TOMCAT runs" &&\
	/var/www/composer/vendor/drush/drush/drush user-create "john@example.com" --mail="john@example.com" --password="password" -y &&\
	/var/www/composer/vendor/drush/drush/drush user-create "jane@example.com" --mail="jane@example.com" --password="password" -y &&\
sleep 60 &&\
	/var/www/fedora/tomcat/bin/shutdown.sh &&\
	bash -c "timeout $TOMCAT_TIMEOUT grep -q 'Server shutdown complete' <(tail -f $FEDORA_HOME/server/logs/fedora.log)" &&\
	/etc/init.d/postgresql stop

#
# Add FLAT's own scripts and tools 
#

ADD flat/scripts/* /app/flat/bin/
RUN chmod +x /app/flat/bin/*.sh &&\
    mv /app/flat/bin/do-* /app/flat/ &&\
    mv /app/flat/bin/do.sh /app/flat/do.sh &&\
	mkdir -p /app/flat/deposit/cache/schemas

#Clean up APT when done.
RUN apt-get clean &&\
	rm -rf /var/lib/apt/lists/* /var/tmp/*
# /tmp/*
