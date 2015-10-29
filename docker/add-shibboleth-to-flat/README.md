Add Shibboleth to the FLAT base image
=====================================

## Requirements ##
This docker file is based on the FLAT base image.

## Provides ##
 * Shibboleth Apache setup
 * Shibboleth Drupal module, exposed via the Islandora Drupal interface
sAdmin:fgsAdmin

## Building the image ##
```sh
docker build -t flat-with-shibboleth .
```

## Running the image ##
```sh
docker run -i -p 80:80 -p 8443:8443 -p 8080:8080 -t flat-with-shibboleth /sbin/my_init -- bash -l
```

## Additional configuration ##

Further configuration in /etc/shibboleth will be needed to turn the server into a valid SP and connect it to one or more IdPs

## Notes ##

TODO: bundle the block configuration of the drupal module in Islandora in a Drupal feature module.

TODO: describe how to test the Shibboleth setup using the [SSO-demo image](https://github.com/menzowindhouwer/sso-demo-docker).

## References ##
