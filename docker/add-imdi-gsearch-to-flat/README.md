Add IMDI GSearch to FLAT
========================

## Requirements ##
This docker file depends on the FLAT base image, the FLAT Islandora SOLR image and the FLAT gsearch image.

## Provides ##

## Building the image ##
```sh
docker build -t flat-with-imdi-gsearch .
```

## Running the image ##
```sh
docker run -p 80:80 -p 8443:8443 -p 8080 -v ~/my-resources:/lat -t -i flat-with-imdi-gsearch
```

## Additional configuration ##

This provides XPaths specific for CMDIfied IMDI and fallbacks to the VLO facet mapping. Its based on the DASISH setup
for CMDIfied IMDI. Adapt the mapping if you have additional or specific mappings for your own IMDI-based CMD profiles.

## Notes ##

## References ##
