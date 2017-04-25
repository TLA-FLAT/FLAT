<?php


function flat_deposit_cmdi_form($form, &$form_state){

    // exit form generation if no parent collection has been selected
    if (isset($form_state['values']['data']['parentFid'])){

        $parent_nid = $form_state['values']['data']['parentFid'];

    } else {

        $node = menu_get_object();
        $wrapper = entity_metadata_wrapper('node', $node);
        $parent_nid = $wrapper->flat_parent_nid->value();

    }

    if ($parent_nid === '0'){
        drupal_set_message('Cannot generate or edit form because collection is not specified', 'error');
        return $form;

    }

    // Set selected profile as this is updated on every AJAX request
    $selected_profile = !empty($form_state['values']['select_profile_name']) ? $form_state['values']['select_profile_name'] : "";

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
            'parentFid' => $parent_nid,
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
    $data =  json_decode(file_get_contents(drupal_realpath(drupal_get_path('module', 'flat_deposit') . '/Helpers/CMDI/FormTemplates/Example_MPI_Collection.json')), TRUE);
    unset($data['data']);
    if ($load_profile_specific_form){
        // review why it does not work if extra condition is chosen
        #if (isset ($form_state['values']['import_trigger']) AND $form_state['values']['import_trigger'] === 1) {

        $result = array_merge_recursive($form['template_container']['elements'], $data);
        $result2 = exchange_numeric_key_with_default_value_property($result);

        $form['template_container']['elements'] = $result2;
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

    form_set_error(NULL,'Debug');
    return $form;

    //validate existence output dir
    $export_dir = 'metadata://' . '/' . $owner . "/.collection/";
    if (!file_exists($export_dir)) {
        drupal_mkdir($export_dir, NULL, TRUE);
    }

    if (!file_exists($export_dir)) {
        form_set_error('error', 'Cannot create directory to temporarily store cmdi files');
        return $form;
    }


    // Validate chosen option
    if ($form_state['values']['select_profile_name'] === '-- Select --') {
        form_set_error('select_profile_name', 'Please choose correct option');
        return $form;
    }


    // set cmdi filename
    $profile = $form_state['values']['select_profile_name'];
    $recordCmdi = $export_dir . '/' . $profile . '_' . uniqid() . '.cmdi';

    // Get cmdi
    $import = $form_state['values']['select_profile_name'] === 'Import' ? TRUE : FALSE;
    $msg = get_cmdi($form_state, $recordCmdi, $import);


    if ($msg !== TRUE) {
        form_set_error('error', $msg);
        return $form;
    }

    /// do INGEST
    global $user;
    $userName = $user->name;
    $sipType = 'Collection';
    $parentFid = $form_state['values']['data']['parentFid'];
    $test = FALSE;

    module_load_include('php', 'flat_deposit', 'Helpers/IngestService/IngestClient');
    $ingest_client = new IngestClient($sipType, $userName, $recordCmdi, $parentFid, $test);

    $options = [];
    $options ['policy'] = $form_state['values']['select_policy'];
    $fid = $ingest_client->requestSipIngest($options);

    // check existence object
    module_load_include('inc', 'flat_deposit', '/Helpers/Fedora_REST_API');

    $accessFedora = variable_get('flat_deposit_fedora');
    $rest_fedora = new FedoraRESTAPI($accessFedora);

    $fObject = $rest_fedora->getObjectData($fid);
    if (!$fObject) {
        form_set_error('error', t('Check of FID for new collection item did not reveal valid data. Error message:' . $fid));
        return $form;
    }

    $form_state['values']['data']['fid'] = (string)$fid;
    $form_state['values']['data']['label'] = $fObject['objLabel'];
    $form_state['values']['data']['owner'] = $fObject['objOwnerId'];
    return $form;
}