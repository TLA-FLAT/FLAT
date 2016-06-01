Add flat User interface
======================

## Requirements ##
This docker file depends on the FLAT base image.

## Provides ##
 * apache xdebug library
 * root account for ssh-login
 * owncloud server
 * drupal modules (IMCE, wysiwyg, ckeditor)
 * Web based GUI for ingesting user data

## Preparation ##
```sh
cd <Where to place FLAT>

git clone https://github.com/TheLanguageArchive/FLAT.git

cd FLAT/docker

#build the FLAT base image
docker build --rm=true -t flat-base flat/
```

## Building the flat_deposit_ui image (minimal) (i.e. without gsearch,SOLR and solution packs) ##
```sh
docker build --rm=true -t flat add-sword-to-flat/
docker build --rm=true -t flat add-flat-ui/
```

## Building the image (full) ##
```sh
#because flat base image inclusive all additional dockerfile has > 127 parents we first need to flaten the base image. We do this by applying following procedure

#1) create a container (instance) of the flat base image
docker run -p 80:80 -p 8443:8443 --name flat-base-con -t -i flat-base

#2) exit manually the running container and export the container to image; resulting image will have 0 parents
docker export --output=flat-base.tar flat-base-con
cat flat-base.tar | docker import - flat

# 3) Finally we can add environment and entry points from original image to the flattened image
mkdir add-flat-env
echo "FROM flat" > add-flat-env/Dockerfile
egrep '^(ENV|CMD|ENTRYPOINT|EXPOSE|WORKDIR).*' flat/Dockerfile >> add-flat-env/Dockerfile
docker build --rm=true -t flat add-flat-env/

# Now we have a flattened docker image we can add gsearch and solar to image
docker build --rm=true -t flat add-imdi-conversion-to-flat/
docker build --rm=true -t flat add-gsearch-to-flat/
docker build --rm=true -t flat add-islandora-solr-to-flat/
docker build --rm=true -t flat add-imdi-gsearch-to-flat/

# add solution packs and sword (for safety make new image)
docker build --rm=true -t flat-with-sps add-solution-packs-to-flat/
docker build --rm=true -t flat-with-sps add-sword-to-flat/

#new image for deposit UI
docker build --rm=true -t flat_dvr add-flat-ui/

#cleanup
docker rm flat-base-con
docker rmi flat-base
rm flat-base.tar
```

## Running the image ##
```sh
docker run -p 80:80 -p 8443:8443 -p 8080:8080 --name <Container> -i -t <name FLAT base image>
```

## Additional configuration ##
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



## Notes ##

The UI expects that FOXML object names start with lat. If this is violated the Ingest service will complain




