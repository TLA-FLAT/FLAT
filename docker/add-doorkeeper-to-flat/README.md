Add the DoorKeeper to FLAT
==========================

## Requirements ##
This docker file depends on the FLAT base image.

## Provides ##
 * the DoorKeeper tool/library that can validate a SIP and deposits it in Fedora Commons
 * the flat REST API to trigger a DoorKeeper check/deposition for a SIP uploaded via SWORD, available at http://IP/flat/doorkeeper

## Building the image ##
```sh
docker build -t flat ./add-doorkeeper-to-flat
```

## Running the image ##
```sh
docker run -p 80:80 -v ~/my-resources:/lat -it flat
```

A test SIP is included:

```sh
/app/flat# ./doorkeeper.sh base=/app/flat/deposit user=pukp
```

## Additional configuration ##

[/app/flat/deposit/flat-deposit.xml](flat/deposit/flat-deposit.xml) contains the set of actions a SIP is pushed through. All actions should succeed to result in a succesful deposit. See the [DoorKeeper documentation](https://github.com/TLA-FLAT/DoorKeeper) for a description of this file and of the available actions.

## Notes ##

To blend in a local DoorKeeper development replace the dummy DoorKeeper directory by the local development directory. If you use a soft link you'll have to tar the Dockerfile context:

```sh
cd ./add-doorkeeper-to-flat
tar -czh . | docker build -t flat -
cd ..
```

## References ##
