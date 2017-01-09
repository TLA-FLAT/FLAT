Add FLAT development tools
=================================================

## Preface ##
This add on for the flat-docker image contains a toolbox with different development tools useful for php-code development.

## Requirements ##
This docker file depends on the FLAT base image.

## Provides ##
 * apache xdebug library
 * root account for ssh-login
 * drupal modules (admin_menu, devel)

## Preparation ##
```sh
cd <Where to place FLAT>

git clone https://github.com/TheLanguageArchive/FLAT.git

cd FLAT/docker

docker build --rm=true -t flat add-develtools-to-flat/
```


=======
### optional configuration ###

## SSH () ##

In order to ssh to the ssh server on a running container, you need to run following command

```ssh
/usr/sbin/sshd
```


## Fedora web gui ##
In order to see data on the fedora server(e.g. Foxml), we need to change a parameter in our native fedora config file.


```ssh
vim /var/www/fedora/server/config/fedora.fcfg
# change value of ENFORCE MODE to permit all requests: <param name="ENFORCE-MODE" value="permit-all-requests"/>
```

