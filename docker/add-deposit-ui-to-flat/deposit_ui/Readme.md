# FLAT deposit UI configuration #

=======
## General ##
When using the docker environment most settings will be configured automatically. However, a few things still need to be done manually.


## Owncloud ##
If you want to use owncloud, you need to configure this app. The dockerfile will automatically do this for you, however, if you don't use docker, you will to do it yourself. You definitely need to change the permission of your owncloud installation. If you also want to be able open the owncloud web gui, you need to add the url of your server to the trusted domanins (see owncloud/addTrusted.php).


## File and Folder permissions ##
In order to ingest data without doorkeeper plugin, the sudoer file (visudo) needs to be adapted. Please copy/paste the code from the shell/sudoer.ini file using visudo

=======

```ssh
sudo /usr/sbin/visudo
:%d #deletes the whole content

```
=======
### SSH (optional) ###

In order to ssh to the ssh server on a running container, you need to run following command

```ssh
/usr/sbin/sshd
```

=======
### Fedora web gui (optional) ###
In order to see data on the fedora server(e.g. Foxml), we need to change a parameter in our native fedora config file.


```ssh
vim /var/www/fedora/server/config/fedora.fcfg
# change value of ENFORCE MODE to permit all requests: <param name="ENFORCE-MODE" value="permit-all-requests"/>
```


## When not using docker ##
All configurations are specified in the inc/config.inc file. If you don't use docker, you definitely need to adapt this file to your needs. As, most likely, problems might particularly relate to the ingest function, also consult the Helpers/Ingest_service.php and Helpers/Fedora_REST_API.inc.
Make sure that you create a freeze directory which is accessible by your web-server


### Owncloud ###
If you want to use owncloud, you need to configure this app. You definitely need to change the permission of your owncloud installation. If you also want to be able open the owncloud web gui, you need to add the url of your server to the trusted domanins (see owncloud/addTrusted.php).


## Notes and known issues##
Make sure that the java links to the correct library (check Dockerfile of flat master branch):

```ssh
unlink /usr/bin/java;
ln -s /opt/jdk1.8.0_72/bin/java /usr/bin/java
PATH=$JAVA_HOME:$PATH
```

I had the problem that the bag-script didn't recognize my $JAVA_HOME variable, although on command line sudo -u www-data echo $JAVA_HOME returns /opt/jdk1.8.0_72. As workaround, you may set $JAVA_BIN manually untill issue is solved.


The UI expects that FOXML object names start with lat. If this is violated the Ingest service will complain

When the user opens the 'commit changes' page before creating a first upload, some owncloud errors might occur.

