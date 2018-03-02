export JAVA_OPTS="-server -Xmx2000m -Dcom.bigdata.rdf.sail.webapp.ConfigParams.propertyFile=/etc/bigdata/RWStore.properties -Dlog4j.configuration=/etc/bigdata/log4j.properties -Dfile.encoding=UTF-8 -Dsun.jnu.encoding=UTF-8 -XX:+UseParallelOldGC"
export CATALINA_OPTS="$CATALINA_OPTS -Djsse.enableSNIExtension=false"
export BLZG_CONF=/etc/bigdata
export CATALINA_PID="/usr/share/tomcat-blzg/catalina.pid"
export BLZG_USER=blazegraph
export CATALINA_HOME=/usr/share/tomcat-blzg
