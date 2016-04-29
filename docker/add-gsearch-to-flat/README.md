Add GSearch to FLAT
===================

## Requirements ##
This docker file depends on the FLAT base image and the FLAT Islandora SOLR image.

## Provides ##
 * GSearch, accessible via: http://IP:8080/fedoragsearch/rest
 * Solr, accessible via: http://IP:8080/solr/

The following accounts are created: 

 * Fedora Commons user with rights to manage GSearch: fgsAdmin:fgsAdmin

## Building the image ##
```sh
docker build -t flat ./add-gsearch-to-flat
```

## Running the image ##
```sh
docker run -p 80:80 -p 8443:8443 -p 8080:8080 -v ~/my-resources:/lat -t -i flat
```

## Additional configuration ##

FLAT uses the Component Metadata (CMD) Infrastructure and allows arbitary CMD profiles. For the Solr facets a mapping from these profiles to the wanted facets needs to be created.
This mapping is specified in the [/app/flat/gsearch-mapping-template.xml](flat/scripts/gsearch-mapping-template.xml) file. The core of the mapping is a facet (or field) specification like

```xml
<field name="Language">
      <xpath val="lower-case(.)">.//cmd:CMD/cmd:Components/cmd:Session/cmd:MDGroup/cmd:Content/cmd:Content_Languages/cmd:Content_Language/cmd:Name</xpath>
      <cmd:facet>language</cmd:facet>
      <cmd:concept>http://hdl.handle.net/11459/CCR_c-5358_3cd089fe-ad03-6181-b20c-635ea41ed818</cmd:concept>
</field>
```

This shows that the mapping can be based on:
 * hardcoded XPaths (version 1.0)
 * using the facet mapping of the [VLO](http://vlo.clarin.eu/) 
 * using a concept from a concept registry like the [CLARIN Concept Registry](http://www.clarin.eu/conceptregistry)
 
Potentially the facet values found can be further manipulated with a XPath 1.0 expression, which have to be placed in the `@val` attribute on the `xpath`, `cmd:facet` or `cmd:concept` elements.

The current mapping has a general CMDI based on the VLO facet mapping. Adapt the mapping if you have
specific mappings for your own CMD profiles. (See the [CMDIfied IMDI mapping](../add-imdi-gsearch-to-flat/flat/scripts/gsearch-mapping-template.xml) for an example.)
 
This mapping is expanded and applied to the records using these scripts in the /app/flat directory inside the container:
 
- [do-3-config-cmd-gsearch.sh](flat/scripts/do-3-config-cmd-gsearch.sh): expands the mapping based on the profiles used by the CMD records in `/app/flat/cmd`

- [do-4-index.sh](flat/scripts/do-4-index.sh): trigger the indexing of the CMD records

Once the indexing is done the Islandora SOLR module has to be further configured by selecting facets as Display fields and Facet fields.

## Notes ##

## References ##

- [Installing Solr and GSearch](https://wiki.duraspace.org/display/ISLANDORA715/Installing+Solr+and+GSearch)
- [Islandora Solr Search](https://wiki.duraspace.org/display/ISLANDORA715/Islandora+Solr+Search)
- [Search & Discovery in islandora](https://wiki.duraspace.org/pages/viewpage.action?pageId=64326523)
