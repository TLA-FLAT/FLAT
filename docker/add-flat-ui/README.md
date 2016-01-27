Add flat User interface
======================

## Requirements ##
This docker file depends on the FLAT base image.

## Provides ##
 * apache xdebug library
 * root account for ssh-login

## Building the image ##
```sh
docker build -t <name FLAT base image> flat-with-ui/
```

## Running the image ##
```sh
docker run -p 80:80 -p 8443:8443 -p 8080:8080 --name <Container> -i -t <name FLAT base image>
```

## Additional configuration ##
```sh
cd <directory FLAT>

#if newest version is needed
git clone https://github.com/TheLanguageArchive/FLAT.git

cd FLAT/docker

#option A: if flat base image has < 127 parents
docker build --rm=true -t flat flat/
docker build --rm=true -t flat add-Flat-UI/


#option B: flat base image has > 127 parents

#create a base image container
docker build --rm=true -t flat-base flat/
docker run -p 80:80 -p 8443:8443 --name flat-base-con -t -i flat-base

#exit manually and export container to image; resulting image will have 0 parents
docker export --output=flat-base.tar flat-base-con
cat flat-base.tar | docker import - flat

#add all parents to image
docker build --rm=true -t flat add-imdi-conversion-to-flat/
docker build --rm=true -t flat add-gsearch-to-flat/
docker build --rm=true -t flat add-islandora-solr-to-flat/
docker build --rm=true -t flat add-imdi-gsearch-to-flat/
docker build --rm=true -t flat add-Flat-UI/

#cleanup
docker rm flat-base-con
docker rmi flat-base
```sh

## Notes ##
need to run /usr/sbin/sshd from within container

## References ##
