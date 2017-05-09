Add solution packs to FLAT (*experimental*)
===========================================

## Requirements ##
This docker file depends on the FLAT base image.

## Provides ##
 * additional Islandora solution packs, exposed via the Islandora Drupal interface and Fedora Commons

## Building the image ##
```sh
docker build -t flat ./add-solution-packs-to-flat
```

## Running the image ##
```sh
docker run -p 80:80 -it flat
```

## Additional configuration ##

The derivatives defined and needed by the solution packs can be created using this script in the /app/flat directory inside the container:

- [do-5-derivatives.sh](flat/scripts/do-5-derivatives.sh): trigger the creation of derivatives for the CMD records

## Notes ##

This module is not used and tested a lot yet and should be considered a proof of concept.

TODO: add support for more solution packs like video and PDF.

## References ##
