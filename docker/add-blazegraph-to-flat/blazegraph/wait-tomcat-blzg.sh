#!/bin/bash

# wait till the shutdown port is available
TO=`expr $FLAT_TIMEOUT / 10`
timeout $FLAT_TIMEOUT bash -c "until echo > /dev/tcp/localhost/8006; do sleep $TO; echo 'Waiting ("${TO}" sec.) for tomcat-blzg to be completely up and running ... '; done" 2> /dev/null

RES=$?
if [ $RES = 124 ]; then
    echo "Tomcat-blzg took too long (> ${FLAT_TIMEOUT} sec.) to become completely up and running!";
else
    echo "Tomcat-blzg is completely up and running.";
fi

echo "Tomcat log[/usr/share/tomcat-blzg/logs/catalina.out] tail>";
tail /usr/share/tomcat-blzg/logs/catalina.out
echo "<Tomcat log[/usr/share/tomcat-blzg/logs/catalina.out] tail";
echo "Fedora log[/usr/share/tomcat-blzg/logs/fedora.log tail>";

exit $RES
