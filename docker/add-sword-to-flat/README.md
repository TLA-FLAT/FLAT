Add SWORD to FLAT
=================

## Requirements ##
This docker file depends on the FLAT base image.

## Provides ##
 * [easy-deposit](https://github.com/DANS-KNAW/easy-deposit) which provides a [SWORD v2 API](http://swordapp.github.io/SWORDv2-Profile/SWORDProfile.html) to receive [bags](https://tools.ietf.org/html/bagit), accessible via: https://IP:8443/easy-deposit

The following accounts are created: 

 * SWORD user with rights to deposit: flat:sword (CHANGE in production!)

## Building the image ##
```sh
docker build -t flat ./add-sword-to-flat
```

## Running the image ##
```sh
docker run -p 80:80 -v ~/my-resources:/lat -it flat
```

## Additional configuration ##

## Notes ##

 * Hardcoded logins and paths are found in [/app/flat/deposit/sword/cfg/application.properties](sword/application.properties), [/app/flat/deposit/sword/cfg/logback.xml](sword/logback.xml) and [/app/flat/do-sword-upload.sh](flat/scripts/do-sword-upload.sh)
 * [/app/flat/bin/flat-sword-upload.sh](flat/scripts/flat-sword-upload.sh) uses ``curl -k`` because the certificate is self-signed, this shoud not be needed in production!

## References ##
