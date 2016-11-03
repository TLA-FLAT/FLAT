<?php


function collection_form($form, $form_state, $entity){

    $form = array();

    $form['collection_name'] = array(
        '#title' => t('Collection Name'),
        '#type' => 'textfield',
        '#description' => 'Unique, pragmatic name for the collection (e.g. acronym)',
        '#default_value' => isset($entity->collection_name) ? $entity->collection_name : '',
        '#required' => TRUE);

    $form['collection_tree'] = array(
        '#title' => t('Collection full path'),
        '#type' => 'textfield',
        '#description' => 'Complete path of collection (from root collection to final subdirectory)',
        '#default_value' => isset($entity->collection_tree) ? $entity->collection_tree : '',
        '#required' => TRUE);

    $form ['member'] = array(
        '#type' => 'textfield',
        '#title' => t('Member Name'),
        '#description' => 'User that is member of this collection',
        '#default_value' => isset($entity->member) ? $entity->member : '',
        '#required' => TRUE);

    $form['actions'] = array(
        '#type' => 'actions');

    $form['actions']['submit'] = array(
        '#type' => 'submit',
        '#value' => t('Save'),
        '#submit' => array('collection_form_submit'),
        '#weight' => 999,
    );

    // Let Field module handle the rest.
    field_attach_form('collection', $entity, $form, $form_state);

    return $form;
}

/**
 *Add validation logic
 *
 */
function collection_form_validate($form, &$form_state){

    module_load_include('inc','flat_deposit_ui','inc/flat_deposit_ui.collections');
    $method = $form_state['build_info']['args'][1];

    switch ($method){
        case 'edit':{

        }

    }

    //Checks collection name
    // sanity
    if (!preg_match('!^[a-zA-Z0-9_]+$!', $form_state['values']['collection_name'])) {
        form_set_error('collection_creation', t('The collection name must contain only letters, numbers, slashes, and underscores.'));
        return $form;
    }

    //Checks fullpath
    // sanity
    if (!preg_match('!^[a-zA-Z0-9_/]+$!', $form_state['values']['collection_tree'])) {
        form_set_error('collection_creation', t('The collection name must contain only letters, numbers, slashes, and underscores.'));
        return $form;
    }

    // uniqueness of path
    $collections = db_select('flat_deposit_ui_collection','p')
        ->fields('p',array('collection_name','collection_tree','member'))
        ->condition('member', $form_state['values']['member'])
        ->execute()
        ->fetchAll(PDO::FETCH_ASSOC);

    // test if array returned by query (all collections user is member of) contains collection name assigned to current collection. In positive cases
    //this is a number. We also want to stop processing only if collection is new (i.e. allow editing an existing collection).
    if (is_numeric(array_search($form_state['values']['collection_tree'],array_column($collections,'collection_tree'))) AND
        $form['collection_tree']['#default_value'] != $form_state['values']['collection_tree'] ){
        $i = array_search($form_state['values']['collection_tree'],array_column($collections,'collection_tree'));
        form_set_error('collection_creation','Collection with exactly same path already exists (' . array_column($collections,'collection_name')[$i] . ')');
        return $form;

    }

}


function collection_form_submit($form, &$form_state){

    $labels = explode('/',$form_state['values']['collection_tree']);
    $data = get_collection_tree_data($labels, 'create');

    #Add values for mandatory fields
    $form_state['values']['label'] = $form_state['values']['collection_name'] . '_' . $form_state['values']['member'];
    $form_state['values']['collection_pid'] = end($data['pid']);
    $form_state['values']['date_created'] = time();

    $entity = entity_ui_form_submit_build_entity($form, $form_state);
    $entity->save();

    // Redirect to the management page.
    $form_state['redirect'] = 'managecollections';

    #drupal_set_message(sprintf("%s is now member of collection %s ", $member,$collection));

}

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

