<?php

function flat_bundle_update_cmdi_resources_form($form, &$form_state)
{

    // hide blocks for function calls that are not related to nodes
    $node = $form_state['build_info']['args'][0]['node'];
    if (!$node){
        return $form;
    }

    // hide blocks for nodes without assigned fedora ID (to be updated bundles) or $nodes without cmdi file
    $node_wrapper = entity_metadata_wrapper('node', $node);


    $fedora_id = $node_wrapper->flat_fid->value();

    $file = $node_wrapper->flat_cmdi_file->value();
    $fName = $file['uri'];



    $form['table_container'] = array
    (
        '#theme' => 'flat_bundle_cmdi_resource_table',
        // Need to set #tree to be able to differentiate
        // between the various delete buttons upon
        // submission.
        '#tree' => TRUE,
    );


    module_load_include('inc','flat_deposit','Helpers/CMDI/class.CmdiHandler');


    if(!file_exists(drupal_realpath($fName))){
        drupal_set_message("Unable to read bundle metadata", 'error');
        return $form;
    } else {

        $cmd = @CmdiHandler::simplexml_load_cmdi_file(drupal_realpath($fName));
        if (!$cmd){

            drupal_set_message("Record cmdi of this bundle can not be read using Simplexml", 'error');
            return $form;

        }
    }


    $count = 0;
    $data = [];
    foreach ($cmd->children('cmd',true)->Resources->ResourceProxyList->ResourceProxy as $resource){

        $resource_type = (String)$resource->children('cmd',true)->ResourceType;
        if ($resource_type == 'LandingPage'){
            continue;
        }

        $resource_id = (string)$resource->attributes()->id;
        $resourceref_attributes = $resource->children('cmd',true)->ResourceRef->attributes('lat', TRUE);
        $fid    = $resourceref_attributes['flatURI'];

        $fObj = islandora_object_load($fid);
        $label = $fObj->label;

        $data[$count]['resource_id'] = $resource_id;
        $data[$count]['label'] = $label;

        $form['table_container'][$count]['resource_id'] = array(
            '#markup' => t('!resource_id', array('!resource_id' => $resource_id)),
        );

        $form['table_container'][$count]['label'] = array(
            '#markup' => t('!label', array('!label' => $label)),
        );


        $form['table_container'][$count]['checked'] = array(
            '#type' => 'checkbox',
        );

        $count++;
    }


    $form['table_container']['data'] = array(
        '#type' => 'value',
        '#value' => $data,
    );
    $form['data'] = array(
        '#type' => 'value',
        '#value' => array(
            'cmdi' =>$cmd,
            'file_id' => $file['fid'],
            'fedora_id' => $fedora_id,
        )
    );
/*
    $form['buttons']['delete'] = array(
        '#type' => 'submit',
        '#value' => t('Remove selected file(s)'),
        '#submit' => array('flat_bundle_delete_cmdi_resource_submit'),
    );

    $form['buttons']['reset'] = array(
        '#type' => 'submit',
        '#value' => 'Reset bundle',
        '#submit' => array('flat_bundle_reset_cmdi_resource_submit'),
    );
*/
    return $form;
}


function flat_bundle_delete_cmdi_resource_submit($form, &$form_state){

    $index = 0;
    module_load_include('inc','flat_deposit','Helpers/CMDI/class.CmdiHandler');
    $cmd = $form_state['values']['data']['cmdi'];
    $id = $cmd->Header->MdProfile;
    $profile_name = CmdiHandler::getNameById($id);


    foreach ($form_state['input']['table_container'] as $row){


        if ($row['checked']){


            $rid = $form_state['values']['table_container']['data'][$index]['resource_id'];

            @CmdiHandler::stripSingleResource($cmd, $profile_name, $rid);
             $index++;
        }


    }

    if ($index > 0){
        $fid = $form_state['values']['data']['file_id'];
        $file = file_load($fid);
        $export = $cmd->asXML($file->uri);
        if ($export){
            file_save($file);
        }
    }

            drupal_set_message(t('!number resource(s) have been removed from bundle', array('!number' => (string)$index)));
}



function flat_bundle_reset_cmdi_resource_submit($form, &$form_state){
    $fid = $form_state['values']['data']['fedora_id'];


    $fObj = islandora_object_load($fid);
    $ds = islandora_datastream_load('CMD', $fObj);

    $file_id = $form_state['values']['data']['file_id'];
    $file = file_load($file_id);

    $cmdi_file = $ds->getContent($file->uri);


    if ($cmdi_file) {
        file_save($file);

        drupal_set_message(t('Original metadata restored'));

    }
}
