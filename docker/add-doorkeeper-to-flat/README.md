Add the DoorKeeper to FLAT (*experimental*)
===========================================

## Requirements ##
This docker file depends on the FLAT base image.

## Provides ##
 * the DoorKeeper tool/library that can validate a SIP and deposits it in Fedora Commons
 * the flat REST API to trigger a DoorKeeper check/deposition for a SIP uploaded via SWORD

## Building the image ##
```sh
docker build -t flat ./add-doorkeeper-to-flat
```

## Running the image ##
```sh
docker run -p 80:80 -p 8443:8443 -p 8080:8080 -v ~/my-resources:/lat -t -i flat
```

A test SIP is included:

```sh
/app/flat# ./doorkeeper.sh base=/app/flat/deposit user=pukp
```

## Additional configuration ##

[/app/flat/deposit/flat-deposit.xml](flat/deposit/flat-deposit.xml) contains the set of actions a SIP is pushed through. All actions should succeed to result in a succesful deposit.

## Notes ##

## References ##
