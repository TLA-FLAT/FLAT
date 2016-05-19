DoorKeeper
=========

Prepare
-------

FITS (File Information Tool Set) is necessary for the execution of DoorKeeper.
A zip file containing the whole tool set can be downloaded [in the FITS website](http://projects.iq.harvard.edu/fits/downloads) and can be unzipped into the "DoorKeeper/lib" folder, for instance.

At the time of writing, the latest version of FITS is 0.8.10. The following necessary steps are applicable to that version.

Due to incompatibilities between the Saxon version used in DoorKeeper and the one included in FITS, some jars were removed from the lib directory within FITS, and replaced.
This required further changes, due to some incompatibilities between versions.
We did the following:

* Removed the following jars from within "DoorKeeper/lib/fits-0.8.10/lib/"
 * saxon9-dom.jar
 * saxon9-jdom.jar
 * saxon9.jar
* Downloaded [saxon9-6-0-7source.zip](http://www.saxonica.com/download/download_page.xml) from the Saxonica website
* Extracted it and got the java files from "net/sf/saxon/option/jdom/"
* Changed the name of classes JDOMDocumentWrapper and JDOMNodeWrapper to DocumentWrapper and NodeWrapper, respectively (updating the references between each other)
* Changed the package name of these classes to "net.sf.saxon.jdom"
* Added a constructor with three arguments to the DocumentWrapper class:
```sh
    public DocumentWrapper(Document doc, String baseURI, Configuration config) {
    	this(doc, config);
    	( (Document) node).setBaseURI(baseURI);
    }
```
* Created a jar containing all these classes from package "net/sf/saxon/option/jdom/" (in this case we used a maven project with the resulting jar being "saxon-jdom-1.0.jar")
* Added a call, in edu.harvard.hul.ois.fits.tools.ToolBase java, that registers the functions to be called from some XSLT files
 * to make sure that the XLST processor is the same used in DoorKeeper and FITS
 * this is done through a call to the "registerAll" method in the SaxonExtensionFunctions class from package "nl.mpi.tla.flat.deposit.util":
```sh
	public Document transform(String xslt, Document input) throws FitsToolException {
		Document doc = null;
		try {
			Configuration config = ( (TransformerFactoryImpl) tFactory).getConfiguration();
			SaxonExtensionFunctions.registerAll(config);
			...
``` 
* Added an assembly execution in the DoorKeeper pom file, in order to create a jar file (doorkeeper-util.jar) containing only the SaxonExtensionFunctions class
* Added this jar to the lib directory within FITS
* Re-compiled FITS (ant compile) and re-created the zip file (ant release)
* Included this new FITS jar, together with the saxon-jdom-1.0.jar and doorkeeper-util.jar) within DoorKeeper, in the FITS lib folder (lib/fits-0.8.10/lib/)

Build
-----

Then, for the build, the fits and ots jars, as well as some jars for the specific tools, should be installed in the local maven repository. 

```sh
mvn install:install-file -Dfile=lib/fits-0.8.10/lib/fits.jar -DgroupId=edu.harvard.hul.ois -DartifactId=fits -Dversion=0.8.10 -Dpackaging=jar
mvn install:install-file -Dfile=lib/fits-0.8.10/lib/ots_1.0.17.jar -DgroupId=edu.harvard.hul.ois -DartifactId=ots -Dversion=1.0.17 -Dpackaging=jar
mvn install:install-file -Dfile=lib/fits-0.8.10/lib/jhove/jhove.jar -DgroupId=edu.harvard.hul.ois -DartifactId=jhove -Dversion=1.0 -Dpackaging=jar
mvn install:install-file -Dfile=lib/fits-0.8.10/lib/nzmetool/metadata.jar -DgroupId=nz.govt.natlib -DartifactId=metadata -Dversion=1.0 -Dpackaging=jar
mvn clean install
```

Execute
-------

In order to execute DoorKeeper, some additions are necessary to the classpath, due to the presence of FITS and its tools.
The following entries should be present in the classpath:
* target/doorkeeper.jar
* lib/fits-0.8.10/lib/*
* lib/fits-0.8.10/lib/droid/*
* lib/fits-0.8.10/lib/jhove/*
* lib/fits-0.8.10/lib/mediainfo/*
* lib/fits-0.8.10/lib/nzmetool/*
* lib/fits-0.8.10/lib/nzmetool/adapters/*
* lib/fits-0.8.10/xml/nlnz/

