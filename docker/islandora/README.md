This readme will describe how to use the docker file to build your docker
container and finalize the setup.

## Requirements ##

Make sure you have docker installed. Check out for http://docker.io for more 
information.

Quick installation link: https://docs.docker.com/installation/#installation

## Building the image ##

1. Start your docker environment
2. Run: 
    "docker build -t islandora/v3 ."

## Running your docker container ##

1. Start your docker environment
2. Run: 
    "docker run --rm -p 80:80 -p 8443:8443 -h fedora.test.lan -t -i islandora/v3 /sbin/my_init -- bash -l"

This will start your docker container with the following properties:
- Mapped each port specified with a "-p" parameter between your container and your host
- Using the hostname specified with the "-h" parameter
- Open a bash shell in your container

## Finalizing your setup ##

On mac osx you can find our your docker ip via the "boot2docker ip" command run
inside the docker shell.

Goto http://<docker ip>/drupal 

## Information about the VM ##

### Installed software ###
- Apache
- Postgresql
- Fedora with bundled tomcat
- Drupal
- Islandora
- Tuque
- Various islandora modules and tools

### Accounts ###

drupal admin account:
admin:admin

fedora admin account:
fedoraAdmin:fedora

database account for "fedora" and "drupal" database:
fedora:fedora
