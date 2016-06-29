Add IMDI conversion to FLAT
===========================

## Requirements ##
This docker file depends on the FLAT base image.

## Provides ##

 * isle2clarin converstion tool
 * CMDIified IMDI to Dublin Core stylesheet

## Building the image ##
1. Start your docker environment
2. Run: 
```sh
docker build -t flat ./add-imdi-conversion-to-flat
```

## Running the image ##
1. Start your docker environment
2. Run
```sh 
docker run -p 80:80 -p 8443:8443 -p 8080:8080 -v ~/my-resources:/lat -t -i flat
```

## Additional configuration ##

### Importing metadata and resources ###

Inside the container the /app/flat directory contains various scripts to convert and import metadata into the FLAT repository:

- [do-0-convert.sh](flat/scripts/do-0-convert.sh): converts all IMDI metdata files found in `/app/flat/src` into CMD records, which will be stored in `/app/flat/cmd`.

## Notes ##

## References ##
