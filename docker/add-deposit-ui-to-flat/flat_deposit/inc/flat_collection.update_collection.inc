<?php

function flat_collection_update_form($form, &$form_state, $fedora_object){

    $profile = 'MPI_Collection';
    drupal_set_title(t('Update Collection Metadata'));

    // Set selected profile as this is updated on every AJAX request
    if (isset($form_state['values']['select_profile_name'])) {
        $form_state['selected'] =  $form_state['values']['select_profile_name'];

    }

    // unset saved 'ajax_select' value if the ajax_select-button is unselected, the saved value of this button is empty and no button has been clicked
    if (!isset($form_state['values']['select_profile_name']) AND !empty($form_state['selected']) AND !isset($form_state['clicked_button'])) {

        $form_state['selected'] = '';
    }



    // get all available template xml
    form_load_include($form_state, 'inc','flat_deposit','Helpers/CMDI/Cmdi2FormParser');
    form_load_include($form_state, 'php','flat_deposit','Helpers/CMDI/CmdiHandler');
    $available_profiles = CmdiHandler::getAvailableTemplates('flat_collection');

    // set '$load_profile_specific_form' which will trigger loading of container with form elements
    #$load_profile_specific_form = array_search($form_state['selected'], $available_profiles);


    global $user;
    $form['owner'] = array(
        '#title' => t('Owner of the collection'),
        '#type' => 'textfield',
        '#required' => TRUE,
        '#default_value' => $user->name,
        '#disabled' => TRUE,

    );

    if (!user_access('admin collection')){$form['owner']['#disabled'] = TRUE; }

    $form['trigger']['select_profile_name'] = array(
        '#title' => t('Which profile do you want to use?'),
        '#type' => 'select',
        '#empty_option' => '-- Select --',
        '#required' => TRUE,
        '#disabled' => TRUE,
        '#options' => $available_profiles,
        '#default_value' => $profile,
        '#ajax' => array(
            'callback' => 'select_profile_name_ajax_callback',
            'wrapper' => 'template-form',
            'method' => 'replace',
            'effect' => 'fade',
        ),
    );

    $form['select_policy'] = array(
        '#title' => t('Which accessibility policy do you want to apply'),
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

    //**************************************************
    // Get profile specific form and attach to container
    //**************************************************

    // load form element container if profile with available template is chosen

    $fields = CmdiHandler::generateDrupalForm($profile);


    // Fill form field with inherited data and add form fields for fields with multivals

    $fObj = $fedora_object;

    $cmdi = CmdiHandler::getCmdiFromDatastream($fObj->id);

    module_load_include('inc', 'flat_deposit', 'Helpers/CMDI/Cmdi2FormParser');
    $parser = new Cmdi2FormParser;
    try {
        $default_values = $parser->getDefaultValuesFromCmdi($profile, $cmdi, true);

        CmdiHandler::createInheritedMultivalForm($fields, $default_values, $form_state);

        $formPlusDefaults = $parser->array_merge_recursive_new($fields, $default_values);
        $form['template_container']['elements'] = $formPlusDefaults;


    } catch (Cmdi2FormParserException $exception) {

        drupal_set_message($exception->getMessage(), 'warning');
        return $form;
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




    // attach hidden data
    $form_state['extra_data'] =  array(
        'fid' => $fedora_object->id,
        'profile' => $profile,
    );

    $form['Submit'] = array(
        '#type' => 'submit',
        '#value' => t('Submit'),
        '#validate' => array('flat_collection_update_form_final_validate'),
    );



    return $form;


}





function flat_collection_update_form_final_validate($form, &$form_state){


    if (!empty(form_get_errors())) {

        form_set_error('debug','Other validations have failed');
        return $form;
    }

    module_load_include('php','flat_deposit','Helpers/CMDI/CmdiHandler');

    //Validate title
    // Special characters
    $new_title = $form_state['values']['template_container']['elements']['field_01'][0];
    if (!preg_match('/^[a-zA-Z0-9\_\s]{2,40}+$/', $new_title)) {
        form_set_error('title', t('Allowed are all letters, numbers and underscores (max length 40)'));
    }



    // Validate that there is just exactly 1 collection with the same label with isPartOfCollection attribute set to parent
    module_load_include('inc','flat_deposit', 'inc/class.FlatTuque');
    $new_title = $form_state['values']['template_container']['elements']['field_01'][0];




    $fid = $form_state['extra_data']['fid'];


    $parent_fid = FlatTuque::getIsPartOfCollection($fid);

    if ($parent_fid === false){
        form_set_error('title','Unable to validate collection naming');
        return $form;
    }


    $values = FlatTuque::getChildrenLabels($parent_fid);

    $array_up = array_map('strtoupper',$values);
    if (count(preg_grep('/^'.strtoupper($new_title).'/',$array_up)) > 1){

        form_set_error('title','Another collection or bundle with same name exists at this location. Please use other name');
        return $form;
    }




    // Validate owner.
    $owner =  $form_state['values']['owner'];
    if (!user_load_by_name($owner)) {

        form_set_error('owner','Specified owner is unknown to drupal');
        return $form;
    };


    //validate existence output dir
    $export_dir = 'metadata://' . '/' . $owner . "/.collection/" ;
    if (!file_exists($export_dir)){
        drupal_mkdir($export_dir, NULL, TRUE);
    }

    if (!file_exists($export_dir)){
        form_set_error('error','Cannot create directory to temporarily store cmdi files');
        return $form;
    }

    // set cmdi filename
    $profile = $form_state['extra_data']['profile'];
    $recordCmdi = $export_dir . '/' . $profile . '_' . uniqid() . '.cmdi';

    // Get cmdi
    $import = FALSE;
    $msg = get_cmdi($form_state['values'], $recordCmdi, $import);


    if ($msg !== TRUE){
        form_set_error('error', $msg);
        return $form;
    }

    // add SelfLink to cmdi
    $old_cmdi = CmdiHandler::getCmdiFromDatastream($fid);
    $handle = (string)$old_cmdi->Header->children('cmd',TRUE)->MdSelfLink;
    #$att = (string)$link->attributes('lat',TRUE)->flatURI;

    $new_cmdi = CmdiHandler::loadXml($recordCmdi);
    CmdiHandler::setMdSelfLink($new_cmdi, $fid, $handle);
    $new_cmdi->asXML($recordCmdi);

    /// do INGEST
    global $user;
    $userName = $user->name ;
    $sipType = 'Collection';

    $test = FALSE;


    module_load_include('php','flat_deposit','Helpers/IngestService/IngestClient');
    $ingest_client = new IngestClient($sipType, $owner, $recordCmdi, $parent_fid, $test);

    $options = [];
    $options ['policy'] = $form_state['values']['select_policy'];

    $fid = $ingest_client->requestSipIngest($options);

    $fObject = islandora_object_load($fid);

    if (!$fObject){
        form_set_error('error', t('Check of FID for new collection item did not reveal valid data. Error message:' . $fid));
        return $form;
    }


    #$form_state['values']['data']['fid'] = (string)$fid;
    #$form_state['values']['data']['label']=  $fObject->label;
    #$form_state['values']['data']['owner']=  $fObject->owner;

    return $form;

}


/**
* Creates new flat_collection node and redirects to parent node.
 *
 */
function flat_collection_update_form_submit($form, &$form_state){


    #$owner = user_load_by_name($form_state['values']['data']['owner']);
    #$uid = $owner->uid;
    #$label = $form_state['values']['data']['label'];

    // Change label of fedora object if Title in cmdi has changed.





    #module_load_include('inc','flat_deposit', 'inc/flat_collection.add_collection');
    #create_collection_node($label, $uid, $fid);

    drupal_set_message('Collection item has been updated');
    $form_state['redirect']= 'islandora/object/' . $form_state['extra_data'][ 'fid'];

}

