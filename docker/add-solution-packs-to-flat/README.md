Add solution packs to FLAT
==========================

## Requirements ##
This docker file depends on the FLAT base image.

## Provides ##
 * additional Islandora solution packs, exposed via the Islandora Drupal interface and Fedora Commons

## Building the image ##
```sh
docker build -t flat-with-solution-packs .
```

## Running the image ##
```sh
docker run -p 80:80 -p 8443:8443 -p 8080:8080 -t -i flat-with-solution-packs
```

## Additional configuration ##

The derivatives defined and needed by the solution packs can be created using this script in the /app/flat directory inside the container:

- [do-5-derivatives.sh](flat/scripts/do-5-derivatives.sh): trigger the creation of derivatives for the CMD records

## Notes ##

TODO: add support for more solution packs like video and PDF.

## References ##
