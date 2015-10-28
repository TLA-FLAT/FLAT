Add GSearch to the FLAT base image
==================================

# Requirements #
This docker file is based on the flat base image

# Provides #
 * GSearch 2.7.1, accessible via: http://IP:8080/fedoragsearch/rest
 * Solr 4.6.1, accessible via: http://IP:8080/solr/
 * Islandora_solr_search 7.x-1.3, exposed via the Islandora drupal interface

The following accounts are created: 

 * Fedora Commons user with rights to manage GSearch: fgsAdmin:fgsAdmin

# Building the image #
```sh
docker build -t flat-with-gsearch .
```

# Running the image #
```sh
docker run -i -p 80:80 -p 8443:8443 -p 8080:8080 -t flat-with-gsearch /sbin/my_init -- bash -l
```

# Additional configuration #

FLAT uses the Component Metadata (CMD) Infrastructure and allows arbitary CMD profiles. For the Solr facets a mapping from these profiles to the facets need to be created.
This mapping is specified in the [/app/flat/gsearch-mapping-template.xml](flat/scripts/gsearch-mapping-template.xml) file. The core of the mapping is a facet (or field) specification like

```xml
<field name="Language">
      <xpath>.//cmd:CMD/cmd:Components/cmd:Session/cmd:MDGroup/cmd:Content/cmd:Content_Languages/cmd:Content_Language/cmd:Name/text()</xpath>
      <cmd:facet>language</cmd:facet>
      <cmd:concept>http://hdl.handle.net/11459/CCR_C-5358_3cd089fe-ad03-6181-b20c-635ea41ed818</cmd:concept>
</field>
```

This shows that the mapping can be based on:
 * hardcoded xpaths
 * using the facet mapping of the [VLO](http://vlo.clarin.eu/)
 * using a concept from a concept registry like the [CLARIN Concept Registry](http://www.clarin.eu/conceptregistry)
 
 This mapping is expanded and applied to the records using these scripts in the /app/flat directory inside the container:
 
- do-4-config-cmd-gsearch.sh: expands the mapping based on the profiles used by the CMD records in /app/flat/cmd

- do-5-search.sh: trigger the indexing of the CMD records

# Notes #

TODO: bundle the configuration of the SOLR modules in Islandora in a Drupal feature module.

# References #

Installing Solr and GSearch:
 * https://wiki.duraspace.org/display/ISLANDORA715/Installing+Solr+and+GSearch

Islandora Solr Search:
 * https://wiki.duraspace.org/display/ISLANDORA715/Islandora+Solr+Search

Search & Discovery in islandora:
 * https://wiki.duraspace.org/pages/viewpage.action?pageId=64326523
