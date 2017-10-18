<?php


function flat_deposit_cmdi_form($form, &$form_state){

    // exit form generation if no parent collection has been selected
    if (isset($form_state['parent_nid'])){

        $parent_nid = $form_state['parent_nid'];

    } else {

        $node = menu_get_object();
        $wrapper = entity_metadata_wrapper('node', $node);
        $parent_nid = $wrapper->flat_parent_nid->value();
        $form_state['parent_nid'] = $parent_nid;

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
    if (isset($form_state['values']['select_profile_name'])) {
        $form_state['selected'] =  $form_state['values']['select_profile_name'];

    }

    // unset saved 'ajax_select' value if the ajax_select-button is unselected, the saved value of this button is empty and no button has been clicked
    if (!isset($form_state['values']['select_profile_name']) AND !empty($form_state['selected']) AND !isset($form_state['clicked_button'])) {

        $form_state['selected'] = '';
    }


    // get all available template xml
    module_load_include('php','flat_deposit','Helpers/CMDI/CmdiHandler');
    $available_profiles = CmdiHandler::getAvailableTemplates('flat_bundle');


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
        '#ajax' => array(
            'callback' => 'select_profile_name_ajax_callback',
            'wrapper' => 'template-form',
            'method' => 'replace',
            'effect' => 'fade',
        ),
    );
    if (isset($form_state['selected'])){$form['trigger']['select_profile_name']['#default_value'] = $form_state['selected'];};


    $form['template_container'] = array(
        '#type' => 'container',
        '#tree' => TRUE,
        '#attributes' => array(
            'id' => array('template-form'),
        ),
    );




    if (!user_access('admin collection')){$form['owner']['#disabled'] = TRUE; }

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
    if (isset($form_state['selected']) and $form_state['selected'] == 'MPI_Bundle'){

        // generate basis form tree
        $fields = CmdiHandler::generateDrupalForm($form_state['selected']);


        // Fill form field with inherited data and add form fields for fields with multivals
        $parent = node_load($parent_nid);
        $pwrapper = entity_metadata_wrapper('node', $parent);
        $pFid = $pwrapper->flat_fid->value();
        $parentCmdi = CmdiHandler::getCmdiFromDatastream($pFid);

        module_load_include('inc', 'flat_deposit', 'Helpers/CMDI/Cmdi2FormParser');
        $parser = new Cmdi2FormParser;
        try {
            $default_values = $parser->getDefaultValuesFromCmdi($form_state['selected'], $parentCmdi);

            CmdiHandler::createInheritedMultivalForm($fields, $default_values, $form_state);

            $formPlusDefaults = $parser->array_merge_recursive_new($fields, $default_values);
            $form['template_container']['elements'] = $formPlusDefaults;


        } catch (Cmdi2FormParserException $exception) {

            drupal_set_message($exception->getMessage(), 'warning');
        }


        // count button presses per field
        CmdiHandler::aggregateClickedButtons($form_state);

        //
        if(isset($form_state['count'])){

            $allFields = CmdiHandler::addMultivalElements($formPlusDefaults, $form_state['count'] );
            $form['template_container']['elements'] = $allFields;

        }

        // check if everything worked as expected
        if (!is_array($form['template_container']['elements'])){

            drupal_set_message('Unable to generate cmdi form based on profile','warning');

        }

    }
    $form['Submit'] = array(
        '#type' => 'submit',
        '#value' => t('Submit'),
        '#validate' => array('flat_deposit_cmdi_form_final_validate'),
    );



    return $form;


}



function flat_deposit_cmdi_form_final_validate($form, &$form_state)
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


    // stop validation if errors have occurred
    if(form_get_errors()){
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