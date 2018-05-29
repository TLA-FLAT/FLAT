<?php

function flat_collection_add_form($form, &$form_state, $fedoraObject){

    drupal_set_title(t('Collection Form'));

    // Set selected profile as this is updated on every AJAX request
    if (isset($form_state['values']['select_profile_name'])) {
        $form_state['selected'] =  $form_state['values']['select_profile_name'];

    }

    // unset saved 'ajax_select' value if the ajax_select-button is unselected, the saved value of this button is empty and no button has been clicked
    if (!isset($form_state['values']['select_profile_name']) AND !empty($form_state['selected']) AND !isset($form_state['clicked_button'])) {

        $form_state['selected'] = '';
    }

    // get all available form template files
    module_load_include('inc', 'flat_deposit', 'Helpers/CMDI/FormBuilder/class.FormBuilder');
    $available_profiles = FormBuilder::getAvailableTemplates('flat_collection');

    // Add option to import a external file
    $available_profiles ['Import'] = 'I want to upload a CMDI metadata file';

    $form['#prefix'] = "<div id='flat_collection_add_form_wrapper'>";
    $form['#suffix'] = "</div>";

    global $user;
    $form['owner'] = array(
        '#title' => t('Owner of the collection'),
        '#type' => 'textfield',
        '#required' => TRUE,
        '#default_value' => $user->name,
    );

    if (!user_access('admin collection')){
        $form['owner']['#disabled'] = TRUE;
    }

    $form['trigger']['select_profile_name'] = array(
        '#title' => t('Which metadata profile do you want to use?'),
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
/*
    $form['inherit_from_collection'] = array(
        '#title' => t('Inherit from collection'),
        '#type' => 'checkbox',
        '#required' => FALSE,
        '#default_value' => TRUE,
        '#states' => array(
            'invisible' => array(
                array(
                    array(':input[name="select_profile_name"]' => array('value' => '')),
                    'or',
                    array(':input[name="select_profile_name"]' => array('value' => 'Import')),
                ),
            ),
        ),
        '#ajax' => array(
            'callback' => 'select_profile_name_ajax_callback',
            'wrapper' => 'template-form',
            'method' => 'replace',
            'effect' => 'fade',
        ),
    );

*/

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
            'fid' => $fedoraObject->id,
        ),
    );

    $form['Submit'] = array(
        '#type' => 'submit',
        '#value' => t('Submit'),
        '#ajax' => array(
          'callback' => 'flat_collection_add_form_ajax_handler',
          'wrapper' => 'flat_collection_add_form_wrapper',
          'effect' => 'fade',
          ),
        '#validate' => array('flat_collection_add_form_final_validate'),
    );



    //********************************************************************
    // Generate profile specific form render array and attach to container
    //********************************************************************

    if (isset($form_state['selected']) and $form_state['selected'] != 'Import' AND $form_state['selected'] != '') {

        $inheritedData = null;
        #$inherit = $form_state['values']['inherit_from_collection'];

        // Load inherited cmdi metadata from node
        $cmdiDs = islandora_datastream_load('CMD',$fedoraObject);
        if($cmdiDs){
            module_load_include('inc', 'flat_deposit', 'Helpers/CMDI/class.CmdiHandler');
            $inheritedData = simplexml_load_string($cmdiDs->content, 'CmdiHandler');
        }


        // Load form builder app
        $templateName = $form_state['selected'];
        $formBuilder = new FormBuilder(null);

        // count button presses per field
        $formBuilder->aggregatePressedButtons($form_state);

        // get the node in nested array from which we can start iterating all button presses per field
        if (isset($form_state['pressedButtons']['template_container']['elements'])) {
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




function flat_collection_add_form_final_validate($form, &$form_state){

    // Form Validation setup
    $owner = $form_state['values']['owner'];
    $profile = $form_state['values']['select_profile_name'];
    $export_dir = 'metadata://' . str_replace('@', '_at_', $owner) . "/.collection/";
    $cmdiFile = $export_dir . '/' . $profile . '_' . uniqid() . '.cmdi';
    $form_state['values']['cmdiFile'] = $cmdiFile;

    $fid = $form_state['values']['data']['fid'];
    // stop validation if errors have previously occurred
    if(form_get_errors()){
        return $form;
    }

    //****************************//
    // Perform general validation //
    //****************************//

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

    $selected = $form_state['values']['select_profile_name'] ;
    switch (true) {
        // For all not imported cases
        case $selected != 'Import':

            //*******************//
            // Title Validations //
            //*******************//
            $title = $form_state['values']['template_container']['elements']['title_field'][0];

            // 1. Validate that no other collection at same collection with very similar name exists
            module_load_include('inc', 'flat_deposit', 'inc/class.FlatTuque');
            $values = FlatTuque::getChildrenLabels($fid);

            if ($values === false) {
                form_set_error('title', 'Unable to validate that collection name is unique at this location');
                return $form;
            }

            if (in_array(strtoupper($title), array_unique(array_map('strtoupper', $values)))) {
                form_set_error('title', 'Another collection or bundle with same name exists at this location. Please use other name');
                return $form;
            }

            // 2. Validate that output directory for new cmdi exists or can be created
            if (!file_exists($export_dir)) {
                drupal_mkdir($export_dir, NULL, TRUE);
            }

            if (!file_exists($export_dir)) {
                form_set_error('error', 'Cannot create directory to temporarily store cmdi files');
                return $form;
            }


            //*******************//
            // Generate Cmdi file//
            //*******************//
            module_load_include('inc', 'flat_deposit', 'Helpers/CMDI/CmdiCreator/class.CmdiCreator');

            $templateName = $form_state['selected'];
            $form_data = $form_state['values']['template_container']['elements'];
            $owner = $form_state['values']['owner'];

            $creator = new CmdiCreator();

            try {
                $creator->setCmdi($templateName, $form_data, $owner);
                $cmdi = $creator->getCmdi();
            } catch (CmdiCreatorException $e) {
                form_set_error('error', $e->getMessage());
                return $form;
            }

            $exported = $cmdi->asXML($cmdiFile);

            if (!$exported) {
                form_set_error('error', 'Unable to save cmdi file');
                return $form;
            }

            break;

        case 'Import':
            $file = file_save_upload('cmdi_file', array(
                // Validate extensions.
                'file_validate_extensions' => array('cmdi'),
            ));
            // If the file did not passed validation:
            if (!$file) {
                form_set_error('cmdi_file','File was not specified or has not correct extension (.cmdi)');
                return $form;
            }
            // Validate valid xml file
            if (!@simplexml_load_file($file->uri)) {
                form_set_error('cmdi_file','File is not a valid xml file');
                return $form;
            }

            copy(drupal_realpath($file->uri), $cmdiFile);
            if (!file_exists($cmdiFile)) {
                form_set_error('cmdi_file', 'Unable to copy specified file to target location');
                return $form;
            }

            break;

        default:
            break;
    }

    //***************//
    // Validate cmdi //
    //***************//
/*
    module_load_include('inc', 'flat_deposit', 'Helpers/CMDI/class.DOMValidator');
    $validator = new DomValidator;
    try{
        $validated = $validator->validateFeeds($cmdiFile, $templateName . '.xsd');
        echo "Feed successfully validated";

    } catch (Exception $e){

        form_set_error($validator->displayErrors());
        return $form;

    }
*/
    /*
    */
    //********************************//
    // Do ingest using the Doorkeeper //
    //********************************//
    $sipType = 'Collection';
    $fid = $form_state['values']['data']['fid'];
    $test = FALSE;

    module_load_include('php','flat_deposit','Helpers/IngestService/IngestClient');
    try{
        $ingest_client = new IngestClient($sipType, $owner, $cmdiFile, $fid, $test);
    } catch (IngestServiceException $exception){
        form_set_error('debug', $exception->getMessage());
        return $form;
    }


    $options = [];
    $options ['policy'] = $form_state['values']['select_policy'];
    $options ['content_type'] = 'flat_collection';
    $fid = $ingest_client->requestSipIngest($options);

    $fObject = islandora_object_load($fid);

    if (!$fObject){
        form_set_error('error', t('Check of FID for new collection item did not reveal valid data. Error message:' . $fid));
        return $form;
    }


    $form_state['values']['data']['fid'] = (string)$fid;
    $form_state['values']['data']['label']=  $fObject->label;
    $form_state['values']['data']['owner']=  $fObject->owner;

    return $form;

}


/**
* Creates new flat_collection node and redirects to parent node.
 *
 */
function flat_collection_add_form_submit($form, &$form_state){

    $owner = user_load_by_name($form_state['values']['data']['owner']);
    $uid = $owner->uid;
    $label = $form_state['values']['data']['label'];
    $fid = $form_state['values']['data']['fid'];
    $target = 'islandora/object/' . $form_state['values']['data'][ 'fid'];
    create_collection_node($label, $uid, $fid);

    drupal_set_message('New collection item has been created');

    ctools_include('ajax');
    ctools_add_js('ajax-responder');
    $commands[] = ctools_ajax_command_redirect($target);
    print ajax_render($commands);
    exit;

}


function create_collection_node($label, $uid, $fid){

    $new_node = new stdClass();
    $new_node->type = 'flat_collection';
    $new_node->language = 'und';
    $new_node->title = $label;
    $new_node->uid = $uid;
    $new_node->status = 1;
    $new_node->sticky = 0;
    $new_node->promote = 0;
    $new_node->format = 3;
    $new_node->revision = 0;
    $new_node->flat_fid['und'][0]['value'] = $fid;
    node_save($new_node);

}



function flat_collection_add_form_ajax_handler (&$form, &$form_state){
    return $form;
}
