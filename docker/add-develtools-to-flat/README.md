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


