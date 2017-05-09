Add Islandora SOLR to FLAT
==========================

## Requirements ##
This docker file depends on the FLAT base image.

## Provides ##
 * Islandora_solr_search, exposed via the Islandora drupal interface

## Building the image ##
```sh
docker build -t flat ./add-islandora-solr-to-flat
```

## Running the image ##
```sh
docker run -p 80:80 -v ~/my-resources:/lat -it flat
```

## Additional configuration ##

Add the Islandora SOLR modules to FLAT, but doesn't do the actual setup of SOLR and its population. One option is to use the 
[add-gsearch-to-flat](../add-gsearch-to-flat) Docker setup.

## Notes ##

## References ##
