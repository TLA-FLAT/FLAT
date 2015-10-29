Add solution packs to the FLAT base image
=========================================

## Requirements ##
This docker file is based on the FLAT base image.

## Provides ##
 * additional Islandora solution packs, exposed via the Islandora Drupal interface and Fedora Commons

## Building the image ##
```sh
docker build -t flat-with-solution-packs .
```

## Running the image ##
```sh
docker run -i -p 80:80 -p 8443:8443 -p 8080:8080 -t flat-with-solution-packs /sbin/my_init -- bash -l
```

## Additional configuration ##

The derivatives defined and needed by the solution packs can be created using this script in the /app/flat directory inside the container:

- [do-6-derivatives.sh](flat/scripts/do-5-search.sh): trigger the creation of derivatives for the CMD records

## Notes ##

TODO: add support for more solution packs like video and PDF.

## References ##
