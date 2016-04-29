FLAT base image
===============

## Requirements ##
Make sure you have docker installed. Check out for http://docker.io for more 
information.

Quick installation link: https://docs.docker.com/installation/#installation

## Provides ##

On OSX you can find our your docker IP via the "docker-machine ip default" command. On unix machines you can use localhost.

 * Islandora, accessible via: http://IP/flat
 * Fedora Commons, accessible via: https://IP:8443/fedora/admin
 * Proai, accessible via: http://IP/oaiprovider

The following accounts are created: 

 * Drupal account: admin:admin
 * Fedora Commons account: fedoraAdmin:fedora
 * PostgreSQL account: fedora:fedora

## Building the image ##
1. Start your docker environment
2. Run: 
```sh
docker build -t flat .
```

## Running the image ##
1. Start your docker environment
2. Run
```sh 
docker run -p 80:80 -p 8443:8443 -p 8080:8080 -v ~/my-resources:/lat -t -i flat
```

This will start your docker container with the following properties:
- Mapped each port specified with a "-p" parameter between your container and your host
- Mount your resources directory at the /lat directory in your container
- Open a bash shell in your container

## Additional configuration ##

### CMD to Dublin Core ###

CMD is a very flexible metadata format and there is no generic mapping to Dublin Core. Fedora Commons does require Dublin Core. Place a
``cmd2dc.xsl`` in /app/flat with the mapping for your CMDI files. (See [cmd2dc](../add-imdi-conversion-to-flat/flat/scripts/cmd2dc.xsl) for an example.)

### Importing metadata and resources ###

Inside the container the /app/flat directory contains various scripts to convert and import metadata into the FLAT repository:

- [do-1-fox.sh](flat/scripts/do-1-fox.sh): converts the CMD records found into /app/flat/cmd into FOX files, which will be stored in /app/flat/fox.

- [do-2-import.sh](flat/scripts/do-2-import.sh): imports the FOX files into Fedora Commons.

## Notes ##

TODO: the Object XML can't be viewed from the Fedora admin console

## References ##
