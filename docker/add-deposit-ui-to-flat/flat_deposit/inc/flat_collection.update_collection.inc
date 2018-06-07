<?php

function flat_collection_update_form($form, &$form_state, $fedora_object){

    drupal_set_title(t('Update Collection Metadata'));

    module_load_include('inc', 'flat_deposit', 'Helpers/CMDI/class.CmdiHandler');
    $ds = islandora_datastream_load("CMD",$fedora_object->id);
    
    $inheritedData = CmdiHandler::simplexml_load_cmdi_string($ds->content);

    if ($inheritedData){
        $profile = $inheritedData->getNameById();
    } else {
        $profile= null;
    }


    $form['#prefix'] = "<div id='flat_collection_update_form_wrapper'>";
    $form['#suffix'] = "</div>";

    global $user;

    $form['owner'] = array(
        '#title' => t('Owner of the collection'),
        '#type' => 'textfield',
        '#required' => TRUE,
        '#default_value' => $user->name,
        '#disabled' => TRUE,
    );

    if (!user_access('admin collection')){$form['owner']['#disabled'] = TRUE; }

    $form['select_profile_name'] = array(
        '#type' => 'hidden',
        '#value' => $profile,
    );

    $form['select_policy'] = array(
        '#title' => t('Which access policy do you want to apply?'),
        '#type' => 'select',
        '#description' => t('Select which access policy should be applied as the default for bundles within this collection. "Public" materials can be accessed by anyone without having to log in. "Authenticated Users" means any user with a valid account for the archive. "Academic Users" are users that logs in with an academic account or whose academic status has been verified. "Private" means that the materials are only accessible to the depositor. Access policies can be refined later.'),
        '#required' => TRUE,
        '#options' => drupal_map_assoc(array('public' => t('public'), 'authenticated' => t('authenticated users'), 'academic' => t('academic users'), 'private' => t('private'))),
        '#default_value' => 'public',
    );

    $form['visibility'] = array(
        '#title' => t('Visibility'),
        '#type' => 'select',
        '#description' => t('Hidden collections are not visible to anyone but the depositor and the archive managers. This is to be used only in rare cases in which the name or other metadata fields reveal too much information about work in progresss. The visibility will be applied to this collection and will be the default value for any sub-collections or bundles within this collection. Only materials with a private access policy can be hidden.'),
        '#required' => TRUE,
        '#options'=> array(
                'show' => 'visible',
                'hide' => 'hidden',
                ),
        '#default_value' => 'visible',
        '#states' => array(
                'visible' => array(
                        ':input[name="select_policy"]' => array('value' => 'private'),
                ),
        ),
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
            'fid' => $fedora_object->id,
        ),
    );
    
    if ($inheritedData) {
        $form['data']['#value']['handle'] = (string)$inheritedData->Header->MdSelfLink;
    }

    $form['Submit'] = array(
        '#type' => 'submit',
        '#value' => t('Submit'),
        '#ajax' => array(
            'callback' => 'flat_collection_update_form_ajax_handler',
            'wrapper' => 'flat_collection_update_form_wrapper',
            'effect' => 'fade',
        ),
        '#validate' => array('flat_collection_update_form_final_validate'),
    );


    //********************************************************************
    // Generate profile specific form render array and attach to container
    //********************************************************************
    module_load_include('inc', 'flat_deposit', 'Helpers/CMDI/FormBuilder/class.FormBuilder');
    $availableFormTemplates = FormBuilder::getAvailableTemplates('flat_collection');

    if (in_array($profile, $availableFormTemplates)) {

        // Load form builder app
        $formBuilder = new FormBuilder(null);

        // count button presses per field
        $formBuilder->aggregatePressedButtons($form_state);

        // get the node in nested array from which we can start iterating all button presses per field
        if (isset($form_state['pressedButtons']['template_container']['elements'])){
            $pressedButtonsRoot = $form_state['pressedButtons']['template_container']['elements'];
        } else {
            $pressedButtonsRoot = null;
        }

        // Generate the form elements
        $formBuilder->setForm($profile, $inheritedData, $pressedButtonsRoot, $inheritAll = true);

        // Attach form elements to base form
        $elements = $formBuilder->getForm();
        $form['template_container']['elements'] = $elements;

        // check if everything worked as expected
        if (!is_array($form['template_container']['elements'])) {
            drupal_set_message('Unable to generate cmdi form based on profile', 'warning');
        }
    } else {

        drupal_set_message('Online editing of the metadata of this collection is not supported');
        $form['Submit']['#disabled'] = TRUE;
    }


    return $form;

}

function flat_collection_update_form_ajax_handler (&$form, &$form_state){
    return $form;
}

function flat_collection_update_form_final_validate($form, &$form_state){


    // stop validation if errors have previously occurred
    if(form_get_errors()){
        return $form;
    }

    // Validate that selected profile is not empty
    if ($form_state['values']['select_profile_name'] === '-- Select --') {
        form_set_error('select_profile_name', 'Please choose correct option');
        return $form;
    }

    // Validate if owner exists.
    $owner = $form_state['values']['owner'];
    if (!user_load_by_name($owner)) {

        form_set_error('owner', 'Specified owner is unknown to drupal');
        return $form;
    };

    // preparation
    module_load_include('inc','flat_deposit', 'inc/class.FlatTuque');

    $fid = $form_state['values']['data']['fid'];
    $fidParent = FlatTuque::getIsPartOfCollection($fid);

    //*******************//
    // Title Validations //
    //*******************//
    $new_title = $form_state['values']['template_container']['elements']['title_field'][0];


    // 1. Title uniqueness validation.
    // Gets fedoraObject->id of parent, uses this id to search all children (fObjects with isPartOfCollection attribute
    // set to parent), extract their fedoraObject->label(s) and checks whether one of these labels is the same as the title
    // for the current object.
    // Note. Only do this if title has actually changed.

    $old_title = $form['template_container']['elements']['title_field'][0]['#default_value'];

    if ($old_title !== $new_title){
        $values = FlatTuque::getChildrenLabels($fidParent);

        if ($values === false) {
            form_set_error('title', 'Unable to validate that collection name is unique at this location');
            return $form;
        }

        if (in_array(strtoupper($new_title), array_unique(array_map('strtoupper', $values)))) {
            form_set_error('title', 'Another collection or bundle with same name exists at this location. Please use other name');
            return $form;
        }
    }

    // 2. Validate that output directory for new cmdi exists or can be created
    $export_dir = 'metadata://' . str_replace('@', '_at_', $owner) . "/.collection/";
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

    $profile = $form_state['values']['select_profile_name'];
    $cmdiFile = $export_dir . '/' . $profile . '_' . uniqid() . '.cmdi';
    $form_state['values']['cmdiFile'] = $cmdiFile;

    module_load_include('inc', 'flat_deposit', 'Helpers/CMDI/CmdiCreator/class.CmdiCreator');

    $templateName = $form_state['values']['select_profile_name'];
    $form_data = $form_state['values']['template_container']['elements'];
    $owner = $form_state['values']['owner'];

    $creator = new CmdiCreator();

    try {
        $creator->setCmdi($templateName, $form_data, $owner);
        $generatedCmdi = $creator->getCmdi();
    } catch (CmdiCreatorException $e) {
        form_set_error('error', $e->getMessage());
        return $form;
    }


// get original cmdi by datastream
    $ds = islandora_datastream_load('CMD', $fid);
    $originalCmdi = CmdiHandler::simplexml_load_cmdi_string($ds->content);
    // add resource Section to updated components
    $newlyGeneratedCmdi = CmdiHandler::simplexml_load_cmdi_string($generatedCmdi->asXML());
    $newlyGeneratedCmdi->addResourceSectionToComponents($originalCmdi);

    // merge original cmdi and and edited components section
    $path = '//cmd:Components';

    $targetComponentNode = $originalCmdi->xpath($path)[0];
    $sourceNode = $newlyGeneratedCmdi->xpath($path)[0]; 

    unset($targetComponentNode->children('cmd',true)->{$profile});


    $domTarget = dom_import_simplexml($targetComponentNode);
    
    $domSource = dom_import_simplexml($sourceNode->children('cmd',true)->{$profile});

    $domSource = $domTarget->ownerDocument->importNode($domSource, TRUE);

    $domTarget->appendChild($domSource);

    //save result

    $exported = $originalCmdi->asXML($cmdiFile);

    if (!$exported) {
        form_set_error('error', 'Unable to save cmdi file');
        return $form;
    }

    //********************************//
    // Do ingest using the Doorkeeper //
    //********************************//
    $sipType = 'Collection';
    $test = FALSE;

    module_load_include('php','flat_deposit','Helpers/IngestService/IngestClient');

    try {
        $ingest_client = new IngestClient($sipType, $owner, $cmdiFile, $fidParent, $test);
    } catch (IngestServiceException $exception) {
        form_set_error('debug', $exception->getMessage());
        return $form;
    }

    $options = [];
    $options ['policy'] = $form_state['values']['select_policy'];
    $options ['content_type'] = 'flat_collection';
    $options ['visibility'] = $form_state['values']['visibility'];


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
* Updates flat_collection node and redirects to parent node.
 *
 */
function flat_collection_update_form_submit($form, &$form_state){

    $target = 'islandora/object/' . $form_state['extra_data'][ 'fid'];

    drupal_set_message('Collection item has been updated');

    ctools_include('ajax');
    ctools_add_js('ajax-responder');
    $commands[] = ctools_ajax_command_redirect($target);
    print ajax_render($commands);
    exit;


}
