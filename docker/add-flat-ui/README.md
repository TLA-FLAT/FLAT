Add flat User interface
======================

## Requirements ##
This docker file depends on the FLAT base image.

## Provides ##
 * xdebug environment
 * ssh daemon

## Building the image ##
```sh
docker build -t flat-with-ui .
```

## Running the image ##
```sh
docker run -p 80:80 -p 8443:8443 -p 8080:8080 -i -t flat-with-UI
```

## Additional configuration ##

## Notes ##

## References ##
