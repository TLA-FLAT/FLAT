This docker file is based on the flat/fedora base image

Additions to the flat/fedora base image:
 * GSearch 2.7.1, accessible via: http://IP:8080/fedoragsearch/rest
 * Solr 4.6.1, accessible via: http://IP:8080/solr/
 * Islandora_solr_search 7.x-1.3, exposed via the Islandora drupal interface

A new fedora users is created with permissions to manage the GSearch installation:
 * fgsAdmin:fgsAdmin

# Building the image #
docker build -t flat-with-gsearch .

# Running the image #
docker run -i -p 80:80 -p 8443:8443 -p 8080:8080 -t flat-with-gsearch /sbin/my_init -- bash -l

# Additional configuration #

FLAT uses the Component Metadata (CMD) Infrastructure and allows arbitary CMD profiles. For the Solr facets a mapping from these profiles to the facets need to be created.

# References #

Installing Solr and GSearch:
 * https://wiki.duraspace.org/display/ISLANDORA715/Installing+Solr+and+GSearch

Islandora Solr Search:
 * https://wiki.duraspace.org/display/ISLANDORA715/Islandora+Solr+Search

Search & Discovery in islandora:
 * https://wiki.duraspace.org/pages/viewpage.action?pageId=64326523
