![FLAT logo](docker/flat/drupal/flat-logo.png) FLAT
===================================================
The FLAT project is currently an investigation into an easy and
maintainable archive setup for language resources with
[Component Metadata](http://www.clarin.eu/cmdi/). Its based on [Fedora Commons](http://fedora-commons.org/)
and [Islandora](http://islandora.ca/). It should meet the technical requirements
for a [CLARIN B centre](http://hdl.handle.net/1839/00-DOCS.CLARIN.EU-77), the [Data Seal of Approval](http://datasealofapproval.org/) and those from the
organizations, [Max Planck Institute for Psycholinguistic](http://www.mpi.nl/) and
the [Meertens Institute](http://www.meertens.knaw.nl/), that cooperate in [The Language Archive](http://tla.mpi.nl/). 

Currently the setup of this project consists of a series of docker setups:

 1. A [FLAT base image](docker/flat/) that
   1. installs Fedora Commons and Islandora
   2. provides tools and scripts to covert from IMDI to CMDI
   3. provides tools and scripts to import CMD records into Fedora
   4. adds support for rendering of CMD records in Islandora
   
 2. A [FLAT search image](docker/add-gsearch-to-flat) builds on the base image and
   1. installs generic search for Fedora Commons
   2. installs Islandora's SOLR modules
   3. provides tools and scripts to configure the index proces for a specific set of CMD records and profiles
   
 3. A [FLAT Shibbolet image](docker/add-shibboleth-to-flat) builds on the base image and
   1. installs Shibboleth
   2. installs Drupal's Shibboleth modules
   
 4. A [FLAT solution packs image](docker/add-solution-packs-to-flat) builds on the base image and
   1. installs Islandora solution packs
   2. provides scripts to trigger the addition of derived datastreams like thumbnails

The FLAT base image is required, but the other ones can be added to it as needed.

###Notes###
FLAT used to be known as EasyLAT, so occassionally documentation and code might still use that name.