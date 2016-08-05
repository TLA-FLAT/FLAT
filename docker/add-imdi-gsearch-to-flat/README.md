Add IMDI GSearch to FLAT
========================

## Requirements ##
This docker file depends on the FLAT base image, the FLAT Islandora SOLR image and the FLAT gsearch image.

## Provides ##

## Building the image ##
```sh
docker build -t flat ./add-imdi-gsearch-to-flat
```

## Running the image ##
```sh
docker run -p 80:80 -p 8443:8443 -p 8080:8080 -v ~/my-resources:/lat -t -i flat
```

## Additional configuration ##

This provides XPaths specific for CMDIfied IMDI and fallbacks to the VLO facet mapping.
Its based on [the DASISH setup for CMDIfied IMDI](https://github.com/DASISH/md-mapping/blob/master/mapfiles/cmdi.xml).
Adapt the mapping if you have additional or specific mappings for your own IMDI-based CMD profiles.

## Notes ##

## References ##
