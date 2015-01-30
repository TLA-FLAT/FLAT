This docker file is based on the easylat/fedora base image and requires access
to the TLA private docker registry.

Additions to the easylat/fedora base image:
 * GSearch 2.6, accessible via: http://IP:8080/fedoragsearch/rest
 * Solr 4.2.0, accessible via: http://IP:8080/solr/
 * Islandora_solr_search 7.x-1.3, exposed via the Islandora drupal interface

A new fedora users is created with permissions to manage the GSearch installation:
 * fgsAdmin:fgsAdmin

# Building the image #
docker build -t easylat/islandora-gsearch .

# Running the image #
docker run -i -p 80:80 -p 8443:8443 -p 8080:8080 -h fedora.test.lan -t easylat/islandora-gsearch:latest /sbin/my_init -- bash -l

## Manual configuration ##

When you run the image as is, some manual configuration steps are required to actually enable search is Islandora:

 * go to http://{docker-ip}/drupal and loging (admin:admin)
 * go to 'Modules' (in the top menu bar)
 * scroll all the way down and enable the modules under 'ISLANDORA SEARCH'
 
Optionally commit your changes to a new local image

# References #

Installing Solr and GSearch:
 * https://wiki.duraspace.org/display/ISLANDORA714/Installing+Solr+and+GSearch

Islandora Solr Search:
 * https://wiki.duraspace.org/display/ISLANDORA714/Islandora+Solr+Search

Search & Discovery in islandora:
 * https://wiki.duraspace.org/pages/viewpage.action?pageId=64326523
