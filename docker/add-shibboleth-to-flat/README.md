Add Shibboleth to FLAT (*experimental*)
=======================================

## Requirements ##
This docker file depends on the FLAT base image.

## Provides ##
 * Shibboleth Apache setup
 * Shibboleth Drupal module, exposed via the Islandora Drupal interface

## Building the image ##
```sh
docker build -t flat ./add-shibboleth-to-flat
```

## Running the image ##
```sh
docker run -p 80:80 -it flat
```

## Additional configuration ##

Further configuration in /etc/shibboleth will be needed to turn the server into a valid SP and connect it to one or more IdPs

## Notes ##

Configuring a Shibboleth Service Provider is very specific to a setup, so no generic Dockerfile can capture it.
This module should thus be seen as hints on which Drupal module to install and some basic configuration.

TODO: bundle the block configuration of the drupal module in Islandora in a Drupal feature module.

TODO: describe how to test the Shibboleth setup using the [SSO-demo image](https://github.com/menzowindhouwer/sso-demo-docker).

## References ##
