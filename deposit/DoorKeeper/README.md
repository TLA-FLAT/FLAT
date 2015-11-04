DoorKeeper
=========


Build
-----

FITS (File Information Tool Set) is necessary for the execution of DoorKeeper. To build it, some jars are needed from within FITS.
A zip file containing the whole tool set can be downloaded [in the FITS website](http://projects.iq.harvard.edu/fits/downloads) and can be unzipped into DoorKeeper/lib folder, for instance.
Then, for the build, the fits and ots jars should be installed in the local maven repository.

```sh
mvn install:install-file -Dfile=lib/fits-0.8.10/lib/fits.jar -DgroupId=edu.harvard.hul.ois -DartifactId=fits -Dversion=0.8.10 -Dpackaging=jar
mvn install:install-file -Dfile=lib/fits-0.8.10/lib/ots_1.0.17.jar -DgroupId=edu.harvard.hul.ois -DartifactId=ots -Dversion=1.0.17 -Dpackaging=jar
mvn install:install-file -Dfile=lib/fits-0.8.10/lib/jhove/jhove.jar -DgroupId=edu.harvard.hul.ois -DartifactId=jhove -Dversion=1.0 -Dpackaging=jar
mvn install:install-file -Dfile=lib/fits-0.8.10/lib/nzmetool/metadata.jar -DgroupId=nz.govt.natlib -DartifactId=metadata -Dversion=1.0 -Dpackaging=jar
mvn clean install
```