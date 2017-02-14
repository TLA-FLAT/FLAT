<?php

/**
 * Flat_supercollection entity form. Necessary to create new collection member relations
 *
 * collection name is a pragmatic name for the collection,
 * collection tree the exact path from the archive root to the place where the collection should be placed
 * member is the name of the user (acronym) that will become member of this collection
 */
function flat_supercollection_form($form, $form_state, $entity){

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
        '#description' => 'Complete path of collection (from root collection to final subdirectory).',
        '#default_value' => isset($entity->collection_tree) ? $entity->collection_tree : '',
        '#required' => TRUE);

    $form ['member'] = array(
        '#type' => 'textfield',
        '#title' => t('Member Name'),
        '#description' => 'Name of the user (acronym) that will become member of this collection',
        '#default_value' => isset($entity->member) ? $entity->member : '',
        '#required' => TRUE);

    $form['buttons']['submit'] = array(
        '#type' => 'submit',
        '#value' => t('Save'),
        '#weight' => 999,
    );

    // Let Field module handle the rest.
    field_attach_form('collection', $entity, $form, $form_state);

    return $form;
}

/**
 * flat_supercollection_form validation
 * */
function flat_supercollection_form_validate($form, &$form_state){

    //Checks collection name
    // sanity
    if (!preg_match('!^[a-zA-Z0-9_]+$!', $form_state['values']['collection_name'])) {
        form_set_error('collection_name', t('The collection name must contain only letters, numbers, slashes, and underscores.'));
    }
    //Checks fullpath
    // sanity
    if (!preg_match('!^[a-zA-Z0-9_/]+$!', $form_state['values']['collection_tree'])) {
        form_set_error('collection_tree', t('The collection name must contain only letters, numbers, slashes, and underscores.'));
    }

    // uniqueness of path
    $collections = db_select('flat_supercollection','p')
        ->fields('p',array('collection_name','collection_tree','member'))
        ->condition('member', $form_state['values']['member'])
        ->execute()
        ->fetchAll(PDO::FETCH_ASSOC);

    // test if array returned by query (all collections user is member of) contains collection name assigned to current collection. In positive cases
    //this is a number. We also want to stop processing only if collection is new (i.e. allow editing an existing collection).
    if (is_numeric(array_search($form_state['values']['collection_tree'],array_column($collections,'collection_tree'))) AND
        $form['collection_tree']['#default_value'] != $form_state['values']['collection_tree'] ){

        $i = array_search($form_state['values']['collection_tree'],array_column($collections,'collection_tree'));
        form_set_error('collection_tree','Collection with exactly same path already exists (' . array_column($collections,'collection_name')[$i] . ')');
    }

    // check existence member in user database
    $match = FALSE;
    $users = entity_load('user');

    foreach ($users as $user){
        if ($user->name === $form_state['values']['member']) $match = TRUE;
    }
    if (!$match){
        form_set_error('member','Member does not exist in database');
    }


}

/*
 * Flat_collection Submission handler. Will generate values for mandatory fields (label, pid and date created).
 * As pid is a value retrieved from fedora this submission handler has to communicate with fedora as well.
 */
function flat_supercollection_form_submit($form, &$form_state){

    // In fedora the field with the name of a collection is called 'label'
    $labels = explode('/',$form_state['values']['collection_tree']);
    $data = get_hierarchical_pids_by_label($labels, 'create');

    #Add values for mandatory fields
    $form_state['values']['label'] = $form_state['values']['collection_name'] . '_' . $form_state['values']['member'];
    $form_state['values']['collection_pid'] = end($data['pid']);
    $form_state['values']['date_created'] = time();

    $entity = entity_ui_form_submit_build_entity($form, $form_state);
    $entity->save();

    // Redirect to the management page.
    $form_state['redirect'] = 'admin/supercollections';

    #drupal_set_message(sprintf("%s is now member of collection %s ", $member,$collection));

}

/**
 * returns data of all fedora objects that have a collection_cModel content type and are member of islandora:compound_collection
 *
 * @return array data
 */
function get_all_collection_cModel_data(){
    $tuque = islandora_get_tuque_connection();

    module_load_include('php', 'flat_deposit', 'inc/fedora_queries');
    $query = query_collection_CModels();

    $results = $tuque->repository->ri->sparqlQuery($query);
    $data=array();
    if (!empty($results)) {
        $data['pid'] = array_column(array_column($results, 'pid'),'value');
        $data['label'] = array_column(array_column($results, 'label'),'value');
    }

    return $data;
}


/**
 * Function returning fedora PIDs for hierarchical structured labels.

 * The function builds a sparql query. For labels with an index >0, the query adds information about the preceding object to the query,
 * allowing by this to have several fedora collection content type objects with the same label.
 *
 * @param array $labels strings containing the labels of fedora objects to be queried
 * @param string $method view existing or create new objects
 *
 * @return array data as stored in fedora. Data consists of the original label and the associated pid. For objects in
 * view-mode that do not exist NULL is returned
 */
function get_hierarchical_pids_by_label($labels, $method='view'){

    $tuque = islandora_get_tuque_connection();
    module_load_include('php', 'flat_deposit', 'inc/fedora_queries');
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
                $pid = create_new_collection_content_type_object($label, NULL, $pid_parent);
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


/**
 * @param $label string label value of the object to be created
 * @param null $pid PID to be assigned to the object. If not specified this is generated
 * @param null $pid_parent Object relationship (rel-ext) info to be added to the FoXML.
 * @return string
 */
function create_new_collection_content_type_object($label, $pid=NULL, $pid_parent=NULL){
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

