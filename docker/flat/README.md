This readme will describe how to use the docker file to build your docker
container and finalize the setup.

## Requirements ##

Make sure you have docker installed. Check out for http://docker.io for more 
information.

Quick installation link: https://docs.docker.com/installation/#installation

## Building the image ##

1. Start your docker environment
2. Run: 
    "docker build -t flat ."

## Running your docker container ##

1. Start your docker environment
2. Run: 
    "docker run -p 80:80 -p 8443:8443 -v ~/my-resources:/lat -t -i flat /sbin/my_init -- bash -l"

This will start your docker container with the following properties:
- Mapped each port specified with a "-p" parameter between your container and your host
- Mount your resources directory at the /lat directory in your container
- Open a bash shell in your container

## Importing metadata and resources ##

Inside the container the /app/flat directory contains various scripts to convert and import metadata into the FLAT repository:

- do-1-convert.sh: converts all IMDI metdata files found in /app/flat/src into CMD records, which will be stored in /app/flat/cmd.

- do-2-fox.sh: converts the CMD records found into /app/flat/cmd into FOX files, which will be stored in /app/flat/fox.

- do-3-import.sh: imports the FOX files into Fedora Commons.

## Accessing the FLAT repository ##

On mac osx you can find our your docker ip via the "docker-machine ip default" command. On unix machines you can use localhost.

Goto http://\<docker ip\>/drupal to see the Islandora UI of your FLAT repository.

Goto http://\<docker ip\>:8443/fedora/admin to see the Fedora Commons management interface for your FLAT repository.

## Information about the VM ##

### Installed software ###
- Apache
- Postgresql
- Fedora with bundled tomcat
- Drupal
- Islandora
- Tuque
- Various islandora modules and tools
- Various FLAT scripts and conversion tools

### Accounts ###

drupal admin account:
admin:admin

fedora admin account:
fedoraAdmin:fedora

database account for "fedora" and "drupal" database:
fedora:fedora

### Notes ###

TODO: the Object XML can't be viewed from the Fedora admin console
