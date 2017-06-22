<?php

function flat_fedora_collections_form($form, &$form_state)
{

    $form['table_container'] = array
    (
        '#theme' => 'flat_collection_form_table',
        // Need to set #tree to be able to differentiate
        // between the various delete buttons upon
        // submission.
        '#tree' => TRUE,
    );

    // query existing fedora collection objects
    global $user;
    $user_name= $user->name;

    $connection = islandora_get_tuque_connection();
    $ri = $connection->repository->ri;

    module_load_include('php', 'flat_deposit','/inc/fedora_queries');
    $query = query_owned_collections($user_name);
    $sparql_query_results = $ri->sparqlQuery($query);

    $count = 0;
    $data = [];



    // query already existing links to collections
    $query = new EntityFieldQuery();
    $query->entityCondition('entity_type', 'node')
        ->entityCondition('bundle', 'flat_collection')
        ->propertyCondition('uid', $user->uid);

    $user_collection_nodes = $query->execute();
    $existing = [];


    if (!empty($user_collection_nodes['node'])) {
        foreach ($user_collection_nodes['node'] as $entity) {
            $nid = $entity->nid;
            $collection_node = node_load($nid);
            $wrapper = entity_metadata_wrapper('node', $collection_node);
            $fid = $wrapper->flat_fid->value();
            $existing[] = $fid;
        }
    }


    foreach ($sparql_query_results as $result){


        $pid = $result['pid']['value'];
        $label = $result['label']['value'];
        $url_link = 'islandora/object/' . $pid ;

        if (in_array($pid, $existing)){
            continue;
        }

        //
        #$obj = $ri = $connection->repository->getObject($pid);

        $data[$count]['label'] = $label;
        $data[$count]['pid'] = $pid;

    $form['table_container'][$count]['label'] = array(
            '#markup' => t('!label', array('!label' => $label)),
        );
        $form['table_container'][$count]['pid'] = array(
            '#markup' => t('!pid', array('!pid' => $pid)),
        );
        $form['table_container'][$count]['link'] = array(
            '#markup' => l(t('link',  array('!pid' => $pid)), $url_link),
        );


        $form['table_container'][$count]['activate'] = array(
            '#type' => 'submit',
            '#value' => t('Activate'),
            // Setting '#name' is required, and must be unique
            '#name' => 'flat_collection_form_table_activate_' . $count,
        );
        $count++;
    }

    $form['table_container']['data'] = array(
        '#type' => 'value',
        '#value' => $data,

    );
    return $form;
}


function flat_fedora_collections_form_submit($form, &$form_state){

    $button =  preg_grep('/^flat_collection_form_table_activate_/', array_keys($form_state['values']));
    $index = str_replace('flat_collection_form_table_activate_', '', implode('///',$button));


    if (is_numeric($index)){

        $data = $form_state['values']['table_container']['data'][$index];

        global $user;
        $uid = $user->uid;

        module_load_include('inc', 'flat_deposit','/inc/flat_collection.add_collection');
        create_collection_node($data['label'], $uid, $data['pid']);


    }




}