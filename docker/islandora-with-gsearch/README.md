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

# References #

Installing Solr and GSearch:
 * https://wiki.duraspace.org/display/ISLANDORA714/Installing+Solr+and+GSearch