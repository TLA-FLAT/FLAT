<?php


function flat_deposit_cmdi_form($form, &$form_state){

    // exit form generation if no parent collection has been selected
    if (isset($form_state['values']['data']['parent_nid'])){

        $parent_nid = $form_state['values']['data']['parent_nid'];

    } else {

        $node = menu_get_object();
        $wrapper = entity_metadata_wrapper('node', $node);
        $parent_nid = $wrapper->flat_parent_nid->value();

    }

    if ($parent_nid === '0'){
        drupal_set_message('Cannot generate or edit form because collection is not specified', 'error');
        return $form;

    }

    if (!empty($node->flat_cmdi_file))
    {
        drupal_set_message('Cannot generate CMDI because file is already attached', 'error');
        return $form;

    }



    // Set selected profile as this is updated on every AJAX request
    $selected_profile = !empty($form_state['values']['select_profile_name']) ? $form_state['values']['select_profile_name'] : "MPI_Bundle";

    // get all available template xml
    module_load_include('php','flat_deposit','Helpers/CMDI/CmdiHandler');

    $available_profiles = CmdiHandler::getAvailableTemplates('flat_bundle');

    // set '$load_profile_specific_form' which will trigger loading of container with form elements
    $load_profile_specific_form = array_search($selected_profile, $available_profiles);

    global $user;
    $form['owner'] = array(
        '#title' => t('Owner of the collection'),
        '#type' => 'textfield',
        '#required' => TRUE,
        '#default_value' => $user->name,

    );


    $form['trigger']['select_profile_name'] = array(
        '#title' => t('Which profile do you want to use?'),
        '#type' => 'select',
        '#empty_option' => '-- Select --',
        '#required' => TRUE,
        '#options' => $available_profiles,
        '#default_value' => $selected_profile,
        '#ajax' => array(
            'callback' => 'select_profile_name_ajax_callback',
            'wrapper' => 'template-form',
            'method' => 'replace',
            'effect' => 'fade',
        ),
    );


    $form['template_container'] = array(
        '#type' => 'container',
        '#tree' => TRUE,
        '#attributes' => array(
            'id' => array('template-form'),
        ),
    );




    if (!user_access('Admin collections')){$form['owner']['#disabled'] = TRUE; }

    // attach hidden data
    $form['data'] = array(
        '#type' => 'value',
        '#value' => array(
            'parent_nid' => $parent_nid,
        ),
    );



    //**************************************************
    // Get profile specific form and attach to container
    //**************************************************

    // load form element container if profile with available template is chosen
    if ($load_profile_specific_form){

        $fields = CmdiHandler::generateDrupalForm($selected_profile);

        // check if everything worked as expected
        if (is_array($fields)){

            $form['template_container']['elements'] = $fields;

        } else {

            drupal_set_message($fields,'warning');

        }


    }

    $form['Submit'] = array(
        '#type' => 'submit',
        '#value' => t('Submit'),
        '#validate' => array('flat_deposit_cmdi_form_validate'),
    );



    // Fill form field with loaded data
    if ($load_profile_specific_form) {
        $parent = node_load($parent_nid);
        $pwrapper = entity_metadata_wrapper('node', $parent);
        $pFid = $pwrapper->flat_fid->value();
        $parentCmdi = CmdiHandler::getCmdiFromDatastream($pFid);

        module_load_include('inc', 'flat_deposit', 'Helpers/CMDI/Cmdi2FormParser');
        $parser = new Cmdi2FormParser;
        try {
            $result = $parser->getDefaultValuesFromCmdi($selected_profile, $parentCmdi);
            $formPlusDefaults = array_merge_recursive($form['template_container']['elements'], $result);
            $form['template_container']['elements'] = $formPlusDefaults;

        } catch (Cmdi2FormParserException $exception) {

            drupal_set_message($exception->getMessage(), 'warning');
        }
    }





    #$data =  json_decode(file_get_contents(drupal_realpath(drupal_get_path('module', 'flat_deposit') . '/Helpers/CMDI/FormTemplates/Example_MPI_Collection.json')), TRUE);


    if ($load_profile_specific_form){
        // review why it does not work if extra condition is chosen
        #if (isset ($form_state['values']['import_trigger']) AND $form_state['values']['import_trigger'] === 1) {

        #$result2 = exchange_numeric_key_with_default_value_property($result);

        #$form['template_container']['elements'] = $result2;
        #}
    }



    return $form;


}



function flat_deposit_cmdi_form_validate($form, &$form_state)
{

    // Validate owner.
    $owner = $form_state['values']['owner'];
    if (!user_load_by_name($owner)) {

        form_set_error('owner', 'Specified owner is unknown to drupal');
        return $form;
    };

    // Validate chosen option
    if ($form_state['values']['select_profile_name'] === '-- Select --') {
        form_set_error('select_profile_name', 'Please choose correct option');
        return $form;
    }

    //validate existence output dir
    $node = menu_get_object();
    $wrapper = entity_metadata_wrapper('node', $node);

    $title = $node->title;
    $collection = $wrapper->flat_parent_title->value();


    $export_dir = 'metadata://' . '/' . $owner . "/$collection/$title";
    if (!file_exists($export_dir)) {
        drupal_mkdir($export_dir, NULL, TRUE);
    }

    if (!file_exists($export_dir)) {
        form_set_error('error', 'Cannot create directory to temporarily store cmdi files');
        return $form;
    }


    // set cmdi filename
    $profile = $form_state['values']['select_profile_name'];
    $recordCmdi = $export_dir . '/' . $profile . '_' . uniqid() . '.cmdi';
    $form_state['values']['recordCmdi'] = $recordCmdi;

    // Get cmdi
    $import =  FALSE;

    module_load_include('php','flat_deposit','Helpers/CMDI/CmdiHandler');

    $msg = get_cmdi($form_state['values'], $recordCmdi, $import);


    if ($msg !== TRUE){
        form_set_error('error', $msg);
        return $form;
    }


/*

    form_set_error('Debug','Debug2');
    return $form;

*/

}



function flat_deposit_cmdi_form_submit($form, &$form_state){


    $node = menu_get_object();
    $wrapper = entity_metadata_wrapper('node', $node);

    $new_file = file_save((object)array(
        'filename' => 'record.cmdi',
        'uri' => $form_state['values']['recordCmdi'],
        'status' => FILE_STATUS_PERMANENT,
        'filemime' => file_get_mimetype($form_state['values']['recordCmdi']),
        'display' => '1',
        'description' =>'',
    ));



    // for some unknown reason flat_location and flat_original_path are messed up by attaching the newly created cmdi file, so we need to restore it
    $flat_location_original = $wrapper->flat_location->value();
    $flat_original_path_original = $wrapper->flat_original_path->value();

    $wrapper->flat_cmdi_file->file->set($new_file);
    $wrapper->save();

    $node = menu_get_object();
    $wrapper = entity_metadata_wrapper('node', $node);
    $wrapper->flat_location->set($flat_location_original);
    $wrapper->flat_original_path->set($flat_original_path_original);
    $wrapper->save();

    $form_state['redirect'] = 'node/' .$node->nid;
    drupal_set_message(t('Metadata for bundle %title has been saved', array('%title' => $node->title)));


}