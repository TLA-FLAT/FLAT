Add Proai to FLAT (*deprecated*)
================================

## Requirements ##
This docker file depends on the FLAT base image, and the use of the Mulgara triple store for the resource index. Mulgara is the default in Fedora Commons, but can be replaced by another triple store, e.g., [Blazegraph](../add-blazegraph-to-flat/).

## Provides ##
 * [OAI PMH endpoint](https://www.openarchives.org/pmh/) preconfigured to provide access to the CMD records, accessible via: http://IP/flat/oaiprovider/

## Building the image ##
```sh
docker build -t flat ./add-proai-to-flat
```

## Running the image ##
```sh
docker run -p 80:80 -v ~/my-resources:/lat -it flat
```

## Additional configuration ##

## Notes ##

This docker setup is provided for installations that can't (yet) replace Mulgara by Blazegraph.

## References ##
