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

    $status = $node_wrapper->flat_bundle_status->value();

    $fedora_id = $node_wrapper->flat_fid->value();

    $file = $node_wrapper->flat_cmdi_file->value();
    $fName = $file['uri'];

    $form['values']['status'] = array(
        '#type' => 'value',
        '#value' => $status
    );


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

            drupal_set_message("The CMDI metadata file of this bundle cannot be read as XML", 'error');
            return $form;

        }
    }

    $location = $node_wrapper->flat_location->value();

    $new_files = [];
    if (file_exists($location)) {

        $new_files = array_diff(preg_grep('/^([^.])/', scandir($location)), array('..', '.'));

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

        $form['table_container'][$count]['label'] = array(
            '#markup' => t('!label', array('!label' => $label)),
        );

        if (in_array($label, $new_files)) {

            $form['table_container'][$count]['checked'] = array(
                '#type' => 'checkbox',
                '#disabled' => TRUE,
            );
        }

        else {
            $form['table_container'][$count]['checked'] = array(
                '#type' => 'checkbox',
            );
        }

        $count++;
    }

    // check whether newly added files have the same name as existing files in the bundle and issue a warning if so
    $existing_files = array_column($data, 'label');

    $same_file_names = array_intersect($new_files, $existing_files);

    if ($same_file_names) {
        drupal_set_message(t("Your selected folder contains files that have the same name as files that are already present in the bundle: " . implode(', ', $same_file_names) . ".<br/>If you proceed,
         the new files will replace the existing ones and the existing ones will be versioned. If this is not your intention, please rename the newly added files first."), 'warning', FALSE);
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
    $form['data']['#value']['handle'] = (string)$cmd->children('cmd', TRUE)->Header->MdSelfLink;

    $form['buttons']['delete'] = array(
        '#type' => 'submit',
        '#value' => t('Remove selected file(s)'),
        '#submit' => array('flat_bundle_delete_cmdi_resource_submit'),
    );

    $form['buttons']['reset'] = array(
        '#type' => 'submit',
        '#value' => 'Reset',
        '#submit' => array('flat_bundle_reset_cmdi_resource_submit'),
	'#suffix' => '<div><br/></div>',
    );

    $form['note'] = array(
	'#prefix' => '<div id="update-resources-note">',
	'#markup' => t('Note: files selected for removal will be removed from the bundle and will not be visible when browsing and searching the archive. They will however not be deleted from the archive and can still be resolved via their persistent identifier.'),
	'#suffix' => '</div>',
    );  
    
    return $form;
}


function flat_bundle_delete_cmdi_resource_submit($form, &$form_state){

    $index = 0;
    $count = 0;

    $node = menu_get_object();
    $wrapper = entity_metadata_wrapper('node', $node);

    $resourceIDs = $wrapper->flat_deleted_resources->value() ? explode(',', $wrapper->flat_deleted_resources->value()) : [];

    module_load_include('inc','flat_deposit','Helpers/CMDI/class.CmdiHandler');
    
    $cmd = $form_state['values']['data']['cmdi'];
    foreach (array_slice($form_state['values']['table_container'], 0, -1) as $row){
        if ($row['checked'] == 1){
            $rid = (string)$form_state['values']['table_container']['data'][$index]['resource_id'];
            array_push($resourceIDs, $rid);
        $count++;
        }

	$index++;

    }

    if ($count > 0){
        $cmd->stripResources($resourceIDs);
        $wrapper->flat_deleted_resources->set(implode(",",$resourceIDs));
        $wrapper->save();
        $fid = $form_state['values']['data']['file_id'];
        $file = file_load($fid);
        $export = $cmd->asXML($file->uri);
        if ($export){
            file_save($file);
        }
    }

            drupal_set_message(t('!number file(s) have been removed from the bundle', array('!number' => (string)$count)));
}



function flat_bundle_reset_cmdi_resource_submit($form, &$form_state){

    $node = menu_get_object();
    $wrapper = entity_metadata_wrapper('node', $node);

    $fid = $form_state['values']['data']['fedora_id'];


    $fObj = islandora_object_load($fid);
    $ds = islandora_datastream_load('CMD', $fObj);

    $file_id = $form_state['values']['data']['file_id'];
    $file = file_load($file_id);

    $cmdi_file = $ds->getContent($file->uri);

    $wrapper->flat_deleted_resources->set('');
    $wrapper->save();


    if ($cmdi_file) {
        file_save($file);

        drupal_set_message(t('List of current files has been reset'));

    }
}

function flat_deposit_form_flat_bundle_update_cmdi_resources_form_alter(&$form, &$form_state, $form_id) {
    if($form_id == 'flat_bundle_update_cmdi_resources_form') {
        $status = $form['values']['status']['#value'];
        switch ($status) {
            case 'open': {
                $form['buttons']['delete']['#disabled'] = FALSE;
                $form['buttons']['reset']['#disabled'] = FALSE;
                break;
            }
            case 'validating': {
                $form['buttons']['delete']['#disabled'] = TRUE;
                $form['buttons']['reset']['#disabled'] = TRUE;
                break;
            }
            case 'failed': {
                $form['buttons']['delete']['#disabled'] = FALSE;
                $form['buttons']['reset']['#disabled'] = FALSE;
                break;
            }
            case 'valid': {
                $form['buttons']['delete']['#disabled'] = TRUE;
                $form['buttons']['reset']['#disabled'] = TRUE;
                break;
            }
            case 'processing': {
                $form['buttons']['delete']['#disabled'] = TRUE;
                $form['buttons']['reset']['#disabled'] = TRUE;
                break;
            }
        }
    }
}
