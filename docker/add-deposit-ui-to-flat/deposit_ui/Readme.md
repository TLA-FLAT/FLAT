# FLAT deposit UI configuration #

## Owncloud ##
If you want to use owncloud, you need to configure this app. The dockerfile will automatically do this for you, however, if you don't use docker, you will to do it yourself. You definitely need to change the permission of your owncloud installation. If you also want to be able open the owncloud web gui, you need to add the url of your server to the trusted domanins (see owncloud/addTrusted.php).


```
# RewriteRule .* index.php [PT,E=PATH_INFO:$1]
```


# File and Folder permissions # 
In order to ingest data without doorkeeper plugin, the sudoer file (visudo) needs to be adapted. Please copy/paste the code from the shell/sudoer.ini file using visudo

```ssh
sudo /usr/sbin/visudo
:%d #deletes the whole content

```
Also, the deposit folder needs to be accessible by the php server (e.g. www-data)

The ingest service which picks up data bags from the freeze directory is at the moment a cronjob (crontab) which may run every 45 minutes. This service needs to be installed manually 

In order to ssh to the ssh server on a running container, you need to run following command (optional)
```ssh
/usr/sbin/sshd 
```

In order to see the data on the fedora server, we need to change a parameter in our native fedora config file (optional) 

```ssh
vim /var/www/fedora/server/config/fedora.fcfg
# change value of ENFORCE MODE to permit all requests: <param name="ENFORCE-MODE" value="permit-all-requests"/>
```



## Notes and knwon issues##

The UI expects that FOXML object names start with lat. If this is violated the Ingest service will complain

When the user opens the 'commit changes' page before creating a first upload, some owncloud errors might occur.

