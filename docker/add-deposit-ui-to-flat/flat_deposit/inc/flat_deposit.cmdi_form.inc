<?php


function flat_deposit_cmdi_form($form, &$form_state)
{

    // exit form generation if no parent collection has been selected
    if (isset($form_state['parent_nid'])) {

        $parent_nid = $form_state['parent_nid'];

    } else {

        $node = menu_get_object();
        $wrapper = entity_metadata_wrapper('node', $node);
        $parent_nid = $wrapper->flat_parent_nid->value();
        $form_state['parent_nid'] = $parent_nid;

    }

    if ($parent_nid === '0') {
        drupal_set_message('Cannot generate or edit form because collection is not specified', 'error');
        return $form;

    }

    if (!empty($node->flat_cmdi_file)) {
        drupal_set_message('Cannot generate CMDI because file is already attached', 'error');
        return $form;

    }


    // Set selected profile as this is updated on every AJAX request
    if (isset($form_state['values']['select_profile_name'])) {
        $form_state['selected'] = $form_state['values']['select_profile_name'];

    }

    // unset saved 'ajax_select' value if the ajax_select-button is unselected, the saved value of this button is empty and no button has been clicked
    if (!isset($form_state['values']['select_profile_name']) AND !empty($form_state['selected']) AND !isset($form_state['clicked_button'])) {

        $form_state['selected'] = '';
    }


    // get all available template xml
    module_load_include('inc', 'flat_deposit', 'Helpers/CMDI/FormBuilder/class.FormBuilder');
    $available_profiles = FormBuilder::getAvailableTemplates('flat_bundle');

    // $available_profiles ['Import'] = 'I want to upload a CMDI metadata file';

    global $user;
    $form['owner'] = array(
        '#title' => t('Owner of the collection'),
        '#type' => 'textfield',
        '#required' => TRUE,
        '#default_value' => $user->name,
    );

    if (!user_access('admin collection')) {
        $form['owner']['#disabled'] = TRUE;
    }


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

    if (isset($form_state['selected'])) {
        $form['trigger']['select_profile_name']['#default_value'] = $form_state['selected'];
    };

    $form['cmdi_file'] = array(
        '#type' => 'file',
        '#title' => t('Choose a file'),
        '#states' => array(
            'visible' => array(
                ':input[name="select_profile_name"]' => array('value' => 'Import'),
            ),
            'required' => array(
                ':input[name="select_profile_name"]' => array('value' => 'Import'),
            ),
        ),
    );

    $form['select_policy'] = array(
        '#title' => t('Which access policy do you want to apply?'),
        '#type' => 'select',
        '#required' => TRUE,
        '#options' => drupal_map_assoc(array('public','private')),
        '#default_value' => 'public',
    );

    $form['hide_metadata'] = array(
        '#title' => t('Hide metadata of this collection'),
        '#type' => 'checkbox',
        '#required' => FALSE,
    );



    $form['template_container'] = array(
        '#type' => 'container',
        '#tree' => TRUE,
        '#attributes' => array(
            'id' => array('template-form'),
        ),
    );


    // attach hidden data
    $form['data'] = array(
        '#type' => 'value',
        '#value' => array(
            'parent_nid' => $parent_nid,
        ),
    );

    $form['Submit'] = array(
        '#type' => 'submit',
        '#value' => t('Submit'),
        '#validate' => array('flat_deposit_cmdi_form_final_validate'),
    );

    //********************************************************************
    // Generate profile specific form render array and attach to container
    //********************************************************************

    if (isset($form_state['selected']) AND !empty($form_state['selected'])) {

        module_load_include('inc', 'flat_deposit', 'Helpers/CMDI/class.CmdiHandler');
        $check = FormBuilder::checkTemplateItems($form_state['selected']);
        if ($check !== TRUE){
            drupal_set_message($check, 'warning');
            return $form;
        }

        // Load inherited cmdi metadata from parent node
        $parent = node_load($parent_nid);
        $pwrapper = entity_metadata_wrapper('node', $parent);
        $pFid = $pwrapper->flat_fid->value();

        $ds = islandora_datastream_load('CMD', $pFid);

        if ($ds) {
            $inheritedData = simplexml_load_string($ds->content, 'CmdiHandler');
        } else {
            $inheritedData = null;
        }

        // Load form builder app
        $templateName = $form_state['selected'];
        $formBuilder = new FormBuilder();

        // count button presses per field
        $formBuilder->aggregatePressedButtons($form_state);

        // get the node in nested array from which we can start iterating all button presses per field
        if (isset($form_state['pressedButtons']['template_container']['elements'])){
            $pressedButtonsRoot = $form_state['pressedButtons']['template_container']['elements'];
        } else {
            $pressedButtonsRoot = null;
        }

        // Generate the form elements
        $formBuilder->setForm($templateName, $inheritedData, $pressedButtonsRoot);

        // Attach form elements to base form
        $elements = $formBuilder->getForm();
        $form['template_container']['elements'] = $elements;

        // check if everything worked as expected
        if (!is_array($form['template_container']['elements'])) {
            drupal_set_message('Unable to generate cmdi form based on profile', 'warning');
        }
    }

    return $form;

}



function flat_deposit_cmdi_form_final_validate($form, &$form_state)
{
    // Form Validation setup
    $node = menu_get_object();
    $wrapper = entity_metadata_wrapper('node', $node);

    $title = $node->title;
    $collection = $wrapper->flat_parent_title->value();

    $owner = $form_state['values']['owner'];
    $profile = $form_state['values']['select_profile_name'];

    $export_dir = 'metadata://' . '/' . str_replace('@', '_at_' , $owner) . "/$collection/$title";
    $cmdiFile = $export_dir . '/' . $profile . '_' . uniqid() . '.cmdi';

    $form_state['values']['cmdiFile'] = $cmdiFile;

    $parent_nid = $form_state['values']['data']['parent_nid'];


    //****************************//
    // Perform general validation //
    //****************************//

    // stop validation immediately if errors have previously occurred
    if(form_get_errors()){
        return $form;
    }

    // Validate that selected profile is not empty
    if ($form_state['values']['select_profile_name'] === '-- Select --') {
        form_set_error('select_profile_name', 'Please choose correct option');
        return $form;
    }

    // Validate if owner exists.
    if (!user_load_by_name($owner)) {
        form_set_error('owner', 'Specified owner is unknown to drupal');
        return $form;
    };

    // Validate that output directory for new cmdi exists or can be created
    if (!file_exists($export_dir)) {
        drupal_mkdir($export_dir, NULL, TRUE);
    }

    if (!file_exists($export_dir)) {
        form_set_error('error', 'Cannot create directory to temporarily store cmdi files');
        return $form;
    }


    //*******************************************//
    // Perform validation specific chosen option //
    //*******************************************//


    //*******************//
    // Title Validations //
    //*******************//
    $title = $form_state['values']['template_container']['elements']['field_01'][0];

    // 1. Validate that title does not contain special characters
    if (!preg_match('/^[a-zA-Z0-9\_\s]{2,40}+$/', $title)) {
        form_set_error('title', t('Allowed are all letters, numbers and underscores (max length 40)'));
    }

    // 2. Validate that no other collection at same collection with very similar name exists
    module_load_include('inc', 'flat_deposit', 'inc/class.FlatTuque');
    $parent_node = node_load($parent_nid);
    $parent_wrapper = entity_metadata_wrapper('node', $parent_node);

    $parent_fid = $parent_wrapper->flat_fid->value();
    $values = FlatTuque::getChildrenLabels($parent_fid);

    if ($values === false) {
        form_set_error('title', 'Unable to validate that collection name is unique at this location');
        return $form;
    }

    if (in_array(strtoupper($title), array_unique(array_map('strtoupper', $values)))) {
        form_set_error('title', 'Another collection or bundle with same name exists at this location. Please use other name');
        return $form;
    }



    //*******************//
    // Generate Cmdi file//
    //*******************//
    module_load_include('inc','flat_deposit','Helpers/CMDI/CmdiCreator/class.CmdiCreator');

    $templateName = $form_state['selected'];
    $form_data = $form_state['values']['template_container']['elements'];
    $owner = $form_state['values']['owner'];

    $creator = new CmdiCreator();

    try{
        $creator->setCmdi($templateName, $form_data, $owner);
        $cmdi = $creator->getCmdi();
    } catch (CmdiCreatorException $e){
        form_set_error('error',$e->getMessage());
        return $form;
    }

    $exported = $cmdi->asXML($cmdiFile);

    if (!$exported){
        form_set_error('error','Unable to save cmdi file');
    }


    //***************//
    // Validate cmdi //
    //***************//
    // Cmdi validation does not make sense as Cmdis will not contain resources at this stage


    //***********************//
    // Validate cmdi profile //
    //***********************//
    module_load_include('inc', 'flat_deposit', 'Helpers/CMDI/class.CmdiHandler');
    $cmdi = simplexml_load_file(drupal_realpath($cmdiFile), 'CmdiHandler');
    try {
        $cmdi->canBeValidated();
    } catch (CmdiHandlerException $cmdiHandlerException){
        form_set_error('cmdi_file',$cmdiHandlerException->getMessage());
        return $form;
    }


    return $form;

}



function flat_deposit_cmdi_form_submit($form, &$form_state){

    $node = menu_get_object();
    $wrapper = entity_metadata_wrapper('node', $node);

    $new_file = file_save((object)array(
        'filename' => 'record.cmdi',
        'uri' => $form_state['values']['cmdiFile'],
        'status' => FILE_STATUS_PERMANENT,
        'filemime' => file_get_mimetype($form_state['values']['cmdiFile']),
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

