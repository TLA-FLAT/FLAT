FLAT base image
===============

## Requirements ##
Make sure you have docker installed. Check out for http://docker.io for more 
information.

Quick installation link: https://docs.docker.com/installation/#installation

## Provides ##

The following endpoints come available:

 * Islandora, accessible via: http://localhost/flat
 * Proai, accessible via: http://localhost/flat/oaiprovider
 * Fedora Commons API-A, accessible via: https://localhost/flat/objects
 * Fedora Commons admin interface, accessible via: https://localhost:8080/fedora/admin (__dev only__)

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
docker run -p 80:80 -p 8080:8080 -v ~/my-resources:/lat -t -i flat
```

This will start your docker container with the following properties:
- Mapped each port specified with a "-p" parameter between your container and your host
- Mount your resources directory at the /lat directory in your container
- Open a bash shell in your container

## Additional configuration ##

The Dockerfile contains some global environment variables, which might have to be adapted:
- ``TOMCAT_TIMEOUT`` gives the timeout (default: ``60 ``seconds) to wait for a Tomcat to startup, if you have a slow/busy machine this might have to be increased
- ``FLAT_HOST`` the hostname/IP (default: ``localhost``) used to access the FLAT services from the host, if you use the Docker Toolbox you have to change this into the IP assigned to your docker-machine (default: ``192.168.99.100``)
- ``DRUPAL_PROJECT`` the base name (default: ``flat``) for FLAT's UI and APIs 
- ``CMD_EXTENSION`` the extension (default: ``cmdi``) used by CMDI files to be imported, change to ``xml`` if you import an OAI harvest
- ``FLAT_ICONS`` the directory (default: ``/app/flat/data/icons``) where the icons for the UI are placed

### CMD to Dublin Core ###

CMD is a very flexible metadata format and there is no generic mapping to Dublin Core. Fedora Commons does require Dublin Core. Place a
``cmd2fox.xsl`` in ``/app/flat/deposit/policies/`` which overwrites the default Dublin Core mapping for your CMDI files. (See [cmd2fox.xsl](../add-imdi-conversion-to-flat/flat/cmd2fox.xsl) for an example.)
The [FLAT search image](../add-gsearch-to-flat) does also provide a way to derive the Dublin Core mapping based on the available VLO facet mappings.

### Importing metadata and resources ###

Inside the container the /app/flat directory contains various scripts to convert and import metadata into the FLAT repository:

- [do-1-fox.sh](flat/scripts/do-1-fox.sh): converts the CMD records found into `/app/flat/cmd` into FOX files, which will be stored in `/app/flat/fox`.

- [do-2-import.sh](flat/scripts/do-2-import.sh): imports the FOX files into Fedora Commons.

## Notes ##

TODO: the Object XML can't be viewed from the Fedora admin console

## References ##
