<?php


function query_existing_labels_in_collection($collection_pid){


    $query = "
PREFIX fm: <info:fedora/fedora-system:def/model#>
PREFIX relex: <info:fedora/fedora-system:def/relations-external#>

SELECT ?pid ?label
FROM <#ri>
WHERE {
    ?object <http://purl.org/dc/elements/1.1/identifier> ?pid;
    fm:label ?label;
    fm:state fm:Active;
    relex:isMemberOfCollection <info:fedora/COLLECTION_PID>;
    }
";

    $query = str_replace('COLLECTION_PID',$collection_pid,$query);

    return $query;

}


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
                ?pid fm:label ?label ;
                fm:state fm:Active;
                fm:label ?label;
                fm:createdDate ?created;
                fm:ownerId ?owner;
                fm:hasModel <info:fedora/islandora:collectionCModel>;
                fm:ownerId '" . $user . "'
            }
            ORDER BY DESC(?created)";
    return $query;
}

/**
 * Gets a list of all data ingested since last login
 */

function query_collection_CModels()
{
    $query = "PREFIX fm: <" . FEDORA_MODEL_URI . ">
    PREFIX frelx: <info:fedora/fedora-system:def/relations-external#>
            SELECT ?pid ?label ?created
            FROM <#ri>
            WHERE {
                ?object <http://purl.org/dc/elements/1.1/identifier> ?pid;
                fm:state fm:Active;
                fm:label ?label;
                fm:createdDate ?created;
                fm:hasModel <info:fedora/islandora:collectionCModel>;
            }
            ORDER BY DESC(?created)";
    return $query;
}


function query_collection_CModels_begin($label)
{
    $query = "PREFIX fm: <" . FEDORA_MODEL_URI . ">
    PREFIX frelx: <info:fedora/fedora-system:def/relations-external#>
            SELECT ?pid ?label ?created
            FROM <#ri>
            WHERE {
                ?object <http://purl.org/dc/elements/1.1/identifier> ?pid;
                fm:state fm:Active;
                fm:label ?label;
                fm:createdDate ?created;
                fm:label '" . $label . "';
                fm:hasModel <info:fedora/islandora:collectionCModel>;";
    return $query;

}

function query_collection_CModels_end()
{
    $query = "}
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