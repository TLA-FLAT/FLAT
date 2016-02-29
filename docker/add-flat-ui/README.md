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


#option B: flat base image has > 127 parents (all dockerfiles are added to the image)

#create a base image container
docker build --rm=true -t flat-base flat/
docker run -p 80:80 -p 8443:8443 --name flat-base-con -t -i flat-base

#exit manually and export container to image; resulting image will have 0 parents
docker export --output=flat-base.tar flat-base-con
cat flat-base.tar | docker import - flat

#add environment and entry points from original image to the flattened image
mkdir add-flat-env
echo "FROM flat" > add-flat-env/Dockerfile
egrep '^(ENV|CMD|ENTRYPOINT|EXPOSE|WORKDIR).*' flat/Dockerfile >> add-flat-env/Dockerfile
docker build --rm=true -t flat add-flat-env/

#add all parents to image
docker build --rm=true -t flat add-imdi-conversion-to-flat/
docker build --rm=true -t flat add-gsearch-to-flat/
docker build --rm=true -t flat add-islandora-solr-to-flat/
docker build --rm=true -t flat add-imdi-gsearch-to-flat/
docker build --rm=true -t flat add-sword-to-flat/

# optional with solutionpacks (for safety make new image)
docker build --rm=true -t flat-with-sps add-solution-packs-to-flat/
docker tag <image-ID> flat-with-sps

#new image for deposit UI
docker build --rm=true -t flat_dvr add-flat-ui/

#cleanup
docker rm flat-base-con
docker rmi flat-base
rm flat-base.tar
```

## Notes ##
need to run /usr/sbin/sshd from within container

In order to see the data on the fedora server, we need to change a parameter in our native fedora config file (/var/www/fedora/server/config/fedora.fcfg):

```ssh
<param name="ENFORCE-MODE" value="permit-all-requests"/>
```

## References ##
