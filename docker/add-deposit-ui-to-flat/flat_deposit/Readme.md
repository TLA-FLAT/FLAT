# FLAT deposit UI configuration #

=======
## General ##
When using the docker environment most settings will be configured automatically. However, a few things still need to be done manually.


## File and Folder permissions ##
In order to ingest data, the sudoer file (visudo) needs to be adapted. Please copy/paste the code from the shell/sudoer.ini file using visudo

=======

```ssh
sudo /usr/sbin/visudo
:%d #deletes the whole content

```

Also make sure that the bag folder is writeable by the apache-user


## Index for solr ##
In order to see ingested data, start the indexing scripts 


```ssh
./do-3-config-cmd-gsearch.sh
./do-4-index.sh
```

## Parameter change ##

To derive dublin core information from cmdi file, we need to adapt parameters in the flat-deposit.xml file. Moreover, we need to specify locally which cmdi profiles to accept. Therefore we need to adapt the policies.



```ssh
vim /app/flat/deposit/flat-deposit.xml
vim /app/flat/deposit/policies/rules.sch
```



## When not using docker ##
All configurations are specified in the inc/config.inc file. If you don't use docker, you definitely need to adapt this file to your needs. As, most likely, problems might particularly relate to the ingest function, also consult the Helpers/Ingest_service.php and Helpers/Fedora_REST_API.inc.
Make sure that you create a freeze directory which is accessible by your web-server


### Owncloud ###
If you want to use owncloud, you need to configure this app. You definitely need to change the permission of your owncloud installation. If you also want to be able open the owncloud web gui, you need to add the url of your server to the trusted domanins (see owncloud/addTrusted.php).



