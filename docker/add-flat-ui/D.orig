FROM        flat
MAINTAINER	Daniel von Rhein <daniel.vonrhein@mpi.nl>

#

RUN apt-get update &&\
    apt-get -y dist-upgrade &&\
    apt-get -y install php-pear &&\
    apt-get -y install php5-dev  &&\
    pecl install xdebug


ADD xdebug/xdebug.conf /tmp/
RUN cat /tmp/xdebug.conf >> /etc/php5/apache2/php.ini


#Clean up when done.
RUN apt-get clean &&\
  rm -rf /tmp/* /var/tmp/*


