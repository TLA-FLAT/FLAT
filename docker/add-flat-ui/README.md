Add flat User interface
======================

## Requirements ##
This docker file depends on the FLAT base image.

## Provides ##
 * apache xdebug library
 * root account for ssh-login

## Building the image ##
```sh
docker build -t <name FLAT base image> flat-with-ui/
```

## Running the image ##
```sh
docker run -p 80:80 -p 8443:8443 -p 8080:8080 --name <Container> -i -t <name FLAT base image>
```

## Additional configuration ##

## Notes ##
need to run /usr/sbin/sshd from within container

## References ##
