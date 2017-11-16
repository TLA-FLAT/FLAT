FROM        flat
MAINTAINER  Paul Trilsbeek <Paul.Trilsbeek@mpi.nl>

# adds Blazegraph triple store and DGI trippi-sail to replace Mulgara
# procedure largely copied from Giancarlo Birello http://v2p2dev.to.cnr.it/doku.php?id=repo381:blazeg
RUN apt-get update &&\
	apt-get -y dist-upgrade

RUN service supervisor stop

# install second tomcat instance
RUN mkdir /tmp/tomcat &&\
    cd /tmp/tomcat &&\
    wget http://apache.mirror1.spango.com/tomcat/tomcat-7/v7.0.82/bin/apache-tomcat-7.0.82.tar.gz &&\
    tar xf apache-tomcat-7.0.82.tar.gz &&\
    mv apache-tomcat-7.0.82 /usr/share/tomcat-blzg

RUN	cd /tmp &&\
		rm -r /tmp/tomcat

ADD tomcat/server.xml /usr/share/tomcat-blzg/conf/server.xml
ADD tomcat/setenv.sh /usr/share/tomcat-blzg/bin/setenv.sh
ADD supervisor/supervisord-tomcat-blzg.conf /etc/supervisor/conf.d/
ADD blazegraph/wait-tomcat-blzg.sh /wait-tomcat-blzg.sh
RUN chmod u+x /wait-tomcat-blzg.sh
ADD supervisor/start.sh /start.sh
RUN chmod u+x /start.sh
RUN service supervisor start

# install Blazegraph
RUN mkdir /tmp/blazegraph_conf &&\
		mkdir -p /var/bigdata/logs &&\
		mkdir -p /etc/bigdata/ &&\
		cd /tmp/blazegraph_conf &&\
		git clone https://github.com/discoverygarden/blazegraph_conf &&\
		cp blazegraph_conf/RWStore.properties /etc/bigdata &&\
		cp blazegraph_conf/log4j.properties /etc/bigdata &&\
		cd /usr/share/tomcat-blzg/webapps &&\
		wget https://sourceforge.net/projects/bigdata/files/bigdata/2.1.4/blazegraph.war/download -O blazegraph.war &&\
		supervisorctl start tomcat-blzg &&\
		/wait-tomcat-blzg.sh &&\
		rm -r /tmp/blazegraph_conf

# build and install DGI trippi-sail
RUN mkdir /tmp/trippi-sail &&\
		cd /tmp/trippi-sail &&\
		git clone https://github.com/discoverygarden/trippi-sail.git &&\
		cd trippi-sail &&\
		mvn package -Dfedora.version=3.8.1 &&\
		cd trippi-sail-blazegraph-remote/target &&\
		tar xf trippi-sail-blazegraph-remote-0.0.1-SNAPSHOT-bin.tar.gz &&\
		mv trippi-sail-blazegraph-remote-0.0.1-SNAPSHOT /opt/trippi-sail &&\
		rm -r /tmp/trippi-sail  &&\
		supervisorctl stop tomcat

# reconfigure fedora
ADD fedora/fedora.xml /var/www/fedora/tomcat/conf/Catalina/localhost/fedora.xml
ADD fedora/remote-blazegraph.xml /var/www/fedora/server/config/spring/remote-blazegraph.xml
ADD fedora/fedora.fcfg /usr/local/fedora/server/config/fedora.fcfg
ADD fedora/env-server.sh /var/www/fedora/server/bin/env-server.sh

# rebuild the resource index
RUN supervisorctl start postgres &&\
		/wait-postgres.sh
RUN /var/www/fedora/server/bin/fedora-rebuild.sh -r org.fcrepo.server.resourceIndex.ResourceIndexRebuilder
RUN /var/www/fedora/server/bin/fedora-rebuild.sh -r org.fcrepo.server.utilities.rebuild.SQLRebuilder
