<?php
/**
 * Created by PhpStorm.
 * User: danrhe
 * Date: 23/02/16
 * Time: 11:44
 */

/**
 * Gets a list of all data ingested since last login
 */

function query_owned_collections($user)
{
    $query = "PREFIX fm: <" . FEDORA_MODEL_URI . ">
            PREFIX fv: <info:fedora/fedora-system:def/view#>
            SELECT ?pid ?label ?created
            FROM <#ri>
            WHERE {
                ?object <http://purl.org/dc/elements/1.1/identifier> ?pid;
                fm:state fm:Active;
                fm:label ?label;
                fm:createdDate ?created;
                fm:ownerId ?owner;
                fm:hasModel <info:fedora/islandora:compoundCModel>;
                fm:ownerId '" . $user . "'
            }
            ORDER BY DESC(?created)";
    return $query;
}

/**
 * Fedora SPARQL query for all owned data.
 */

function create_query_all_owned_files($user) {

    $query = "PREFIX fm: <" . FEDORA_MODEL_URI . ">
            PREFIX fv: <info:fedora/fedora-system:def/view#>
            PREFIX relex: <info:fedora/fedora-system:def/relations-external#>

            SELECT DISTINCT ?pid ?label ?created
            FROM <#ri>
            WHERE {
                ?object <http://purl.org/dc/elements/1.1/identifier> ?pid;
                fm:state fm:Active;
                fm:label ?label;
                fm:createdDate ?created;
                fm:ownerId ?owner;
                fv:disseminates ?dis;
                relex:isConstituentOf ?ischild;
                fm:ownerId '" . $user . "'
            }
            ORDER BY DESC(?created)";
    return $query;
}

#function export_array_to_db($array) {

#db_create_table('temp_ingested_data');


#}