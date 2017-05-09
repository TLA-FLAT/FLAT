![FLAT logo](docker/flat/drupal/flat-logo.png) FLAT
===================================================
The FLAT project aims to develop an easy to use and
maintainable archive setup for language resources with
[Component Metadata](http://www.clarin.eu/cmdi/). Its based on [Fedora Commons](http://fedora-commons.org/)
and [Islandora](http://islandora.ca/). It should meet the technical requirements
for a [CLARIN B centre](http://hdl.handle.net/1839/00-DOCS.CLARIN.EU-77), the [Data Seal of Approval](http://datasealofapproval.org/) and those from the
organizations, the [Max Planck Institute for Psycholinguistics](http://www.mpi.nl/) and
the [Meertens Institute](http://www.meertens.knaw.nl/), that cooperate in [The Language Archive](http://tla.mpi.nl/).

Currently the setup of this project consists of a series of docker setups:

1. A [FLAT base image](docker/flat/) that
    1. installs Fedora Commons and Islandora
    2. provides tools and scripts to import CMD records into Fedora
    3. adds support for rendering of CMD records in Islandora

2. A [FLAT Islandora SOLR image](docker/add-islandora-solr-to-flat) builds on the base image and
    1. installs Islandora's SOLR modules

3. A [FLAT search image](docker/add-gsearch-to-flat) builds on the base image and the Islandora SOLR image
    1. installs generic search for Fedora Commons
    2. provides tools and scripts to configure the index proces for a specific set of CMD records and profiles

4. A [FLAT SWORD image](docker/add-sword-to-flat) builds on the base image and
    1. installs a SWORD v2 API to receive bags
   
5. A [FLAT DoorKeeper image](docker/add-doorkeeper-to-flat) builds on the base image and
    1. installs the DoorKeeper, which guards the repository and checks new or updated resources and metadata
    2. installs the DoorKeeper API to process bags

6. A [FLAT example setup image](docker/add-example-setup-to-flat) builds on the doorkeeper image and
    1. adds a comic book collection,
    2. a comic book SIP, and
    3. related users

7. An *experimental* [FLAT deposit UI image](docker/add-deposit-ui-to-flat) builds on the base image and the SWORD image
    1. installs a module that provides an UI for users to deposit data
   
8. An *experimental* [FLAT Shibbolet image](docker/add-shibboleth-to-flat) builds on the base image and
    1. installs Shibboleth
    2. installs Drupal's Shibboleth modules
   
9. An *experimental* [FLAT solution packs image](docker/add-solution-packs-to-flat) builds on the base image and
    1. installs Islandora solution packs
    2. provides scripts to trigger the addition of derived datastreams like thumbnails

The FLAT base image is required, but the other ones can be added to it as needed (but might depend on eachother).

Additionally there are two docker setups specific for IMDI and CMDIfied IMDI:

10. A [FLAT IMDI conversion image](docker/add-imdi-conversion-to-flat) builds on the base image and
    1. provides tools and scripts to convert from IMDI to CMDI

11. A [FLAT IMDI search image](docker/add-imdi-gsearch-to-flat) builds on the search image and
    1. provides the mapping to configure the index proces for CMDIfied IMDI records and profiles

## Building a FLAT docker image ##

This description assumes you're using a recent native Docker (17 or higher).

CMDI records can vary a lot. Based on the VLO configuration a mapping to Dublin Core is determined. You might want to tweak that to your specific needs (see the [FLAT search image](docker/add-gsearch-to-flat) [configuration section](docker/add-gsearch-to-flat#additional-configuration)). If you have IMDI records extra conversion and configuration is needed (see [section](#building-a-flat-docker-image-if-you-have-imdi-records) below).

__NOTE__: simple passwords are included in the setup, they should not be take along to a production environment!

The following commands show how to build a setup that supports FLAT base plus facetted search and the DoorKeeper:

```sh
cd docker
#start with the FLAT base
docker build --squash -t flat flat/
#add Fedora gsearch + SOLR
docker build --squash -t flat add-gsearch-to-flat/
#add Islandora SOLR module
docker build --squash -t flat add-islandora-solr-to-flat/
#add SWORD API
docker build --squash -t flat add-sword-to-flat/
#add the DoorKeeper
docker build --squash -t flat add-doorkeeper-to-flat/
#add the example setup
docker build --squash -t flat add-example-setup-to-flat/
```

## Running a FLAT docker image ##

Now the FLAT docker image can be run:

```sh
docker run -p 80:80 -it flat
```

In the container shell run: 

```sh
#run all the steps to batch import the example comic book collection
do.sh
#and add the example SIP
#- packup the SIP directory
flat-create-sip.sh /app/flat/test/test-sip
#- upload the SIP via SWORD
flat-sword-upload.sh test-sip.zip test
#- trigger the DoorKeeper run for the SIP
wget --method=PUT http://localhost:8080/flat/doorkeeper/test
#- inspect the result
wget http://localhost:8080/flat/doorkeeper/test
#- inspect the developers log
tail -f deposit/bags/test/bag-test-sip/data/test-sip/logs/devel.log
```

Now visit FLAT in your [browser](http://localhost/flat).

## Building a FLAT docker image if you have IMDI records ##

If you have IMDI records you can add the conversion to CMDI and the configuration for CMDIfied IMDI search.

```sh
cd docker
#start with the FLAT base
docker build --squash -t flat flat/
#add IMDI conversion
docker build --squash -t flat add-imdi-conversion-to-flat/
#add Fedora gsearch + SOLR
docker build --squash -t flat add-gsearch-to-flat/
#add Islandora SOLR module
docker build --squash -t flat add-islandora-solr-to-flat/
#add configuration for CMDIfied IMDI search
docker build --squash -t flat add-imdi-gsearch-to-flat/
#add SWORD API
docker build --squash -t flat add-sword-to-flat/
#add the DoorKeeper
docker build --squash -t flat add-doorkeeper-to-flat/
```

## Running a FLAT docker image if you have IMDI records ##

Now the FLAT docker image can be run:

```
docker run -p 80:80 -v ./some-directory:/lat -it flat
```

Run the various ```do-*.sh``` scripts in their natural order. And visit FLAT in your [browser](http://localhost/flat).

## Known problems ##

* _PROBLEM_: Starting and stopping the Tomcat application server can take longer than expected, as it depends on the power or activity of the host.
  * _SOLUTION_: Increase the [```TOMCAT_TIMEOUT```](docker/flat/Dockerfile).
* _PROBLEM_: During the compilation of the MediaShelf fedora-client the test sometimes runs into a locking problem.
  * _SOLUTION_: Just restart the build, the test will most likely succeed this time.

## Publications, Presentations & Demonstrations ##

* M. Windhouwer. [Fedora Commons in the CLARIN Infrastructure](http://www.slideshare.net/mwindhouwer/fedora-commons-in-the-clarin-infrastructure). Presentation at [_CLARIN-PLUS workshop: "Facilitating the Creation of National Consortia - Repositories"_](https://www.clarin.eu/event/2017/clarin-plus-workshop-facilitating-creation-national-consortia-repositories). Prague, Czech Republic, February 9 - 10, 2017.
* P. Trilsbeek, M. Windhouwer. [FLAT: A CLARIN-compatible repository solution based on Fedora Commons](https://www.clarin.eu/content/abstracts-overview-clarin-annual-conference-2016#Z). At the [_CLARIN Annual Conference_](https://www.clarin.eu/event/2016/clarin-annual-conference-2016-aix-en-provence-france). Aix-en-Provence, France, 26-28 October, 2016. 
* M. Windhouwer, M. Kemps-Snijders, P. Trilsbeek, A. Moreira, B. van der Veen, G. Silva, D. von Rhein. [FLAT: constructing a CLARIN compatible home for language resources](http://www.lrec-conf.org/proceedings/lrec2016/summaries/476.html). In _Proceedings of the Tenth International Conference on Language Resources and Evaluation_ ([LREC 2016](http://lrec2016.lrec-conf.org/en/)), European Language Resources Association ([ELRA](http://www.elra.info/)), Portorož, Slovenia, May 23 - 28, 2016. ([Local updated version](documents/2016-LREC-FLAT.pdf))

___
FLAT was once upon a time known as EasyLAT, so occassionally documentation and code might still use that name.
