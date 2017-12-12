Add Proai to FLAT (*deprecated*)
================================

## Requirements ##
This docker file depends on the FLAT base image

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

## References ##
