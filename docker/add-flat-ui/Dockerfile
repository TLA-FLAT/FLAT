FROM        flat
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
RUN git clone path... /var/www/html/drupal/sites/all/modules/custom/flat_deposit_ui
http://download.cksource.com/CKEditor/CKEditor/CKEditor%203.6.2/ckeditor_3.6.2.tar.gz
RUN cd /var/www/html/drupal/ &&\
    /var/www/composer/vendor/drush/drush/drush en admin_menu &&\
    /var/www/composer/vendor/drush/drush/drush en imce            &&\
    /var/www/composer/vendor/drush/drush/drush en flat_deposit_ui

#Clean up when done.
RUN apt-get clean &&\
  rm -rf /tmp/* /var/tmp/*


