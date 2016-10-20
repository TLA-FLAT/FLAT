<?php

/**
 * returns data of all fedora objects that have a collection_cModel content type and are member of islandora:compound_collection
 *
 * @return array data
 */
function get_all_collection_cModel_data(){
    $tuque = islandora_get_tuque_connection();

    $query = query_collection_CModels();

    $results = $tuque->repository->ri->sparqlQuery($query);
    $data=array();
    if (!empty($results)) {
        $data['pid'] = array_column(array_column($results, 'pid'),'value');
        $data['label'] = array_column(array_column($results, 'label'),'value');
    }

    return $data;
}




function get_collection_tree_data($labels, $method='view'){

    $tuque = islandora_get_tuque_connection();
    $data=[];

    $count = 0;

    foreach ($labels as $label){
        // query where collection label = $label
        $query_begin = query_collection_CModels_begin($label);

        // root collection should be found in compound collection
        if ($count==0){
            $pid_parent = NULL;
            $query_mid = "frelx:isMemberOfCollection <info:fedora/islandora:compound_collection>;";
        } else {
        // add where isMemberOfCollection $pid_parent for subcollections
            $pid_parent = $data['pid'][$count-1];
            $query_mid = "frelx:isMemberOfCollection <info:fedora/" . $pid_parent . ">;";
        }

        $query_end = query_collection_CModels_end();
        $full_query = $query_begin . $query_mid . $query_end;

        // run sparql query for sub collections only if pid for all parent collections are known
        if ($count>0 AND !$pid_parent){
            $results = [];
        } else {
            $results = $tuque->repository->ri->sparqlQuery($full_query);
        }

        // if query returns values extract these from nested array
        if (!empty($results)){
            $data['pid'][] = array_column(array_column($results, 'pid'),'value')[0];
            $data['label'][] = array_column(array_column($results, 'label'),'value')[0];
        } else {
        // otherwise create collection first or return empty value
            if ($method=='create'){
                $pid = ingest_new_collection($label, NULL, $pid_parent);
                $data['pid'][] = $pid;
                $data['label'][] = $label;
            } else {
                $data['pid'][] = NULL;
                $data['label'][] = $label;
            }
        }
        $count += 1;
    }

    return $data;

}




function ingest_new_collection($label, $pid=NULL, $pid_parent=NULL){
    // $pid may also be a namespace.
    $pid = $pid ? $pid : 'lat';

    $connection = islandora_get_tuque_connection();

    $new_object = $connection->repository->constructObject($pid);
    $connection->repository->ingestObject($new_object);

    $new_object->label = $label;

    $new_object->models = array('islandora:collectionCModel');

    if ($pid_parent){
        $new_object->relationships->add(FEDORA_RELS_EXT_URI, 'isMemberOfCollection', $pid_parent);
    } else {
        $new_object->relationships->add(FEDORA_RELS_EXT_URI, 'isMemberOfCollection', 'islandora:compound_collection');
    }

    return ($new_object->id);

    //TODO maybe add datastreams like Thumbnail and collection policies to content model
    #$datastream_id = "TN";
    #$new_datastream = $new_object->constructDatastream($datastream_id);
    #$new_object->ingestDatastream($new_datastream);

    #$connection->repository->purgeObject($pid_new);

#if(!$new_object){
    #$new_object = islandora_object_load($pid_new);
    #$existing_object = $connection->repository->getObject($existing_pid);
}

