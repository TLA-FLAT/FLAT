<?php

function flat_deposit_cmdi_form($form, &$form_state)
{

    // exit form generation if no parent collection has been selected
    if (isset($form_state['parent_nid'])) {
        $parent_nid = $form_state['parent_nid'];
    } else {
        $node = node_load($form_state['build_info']['args'][0]->nid);
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

    $form['#prefix'] = "<div id='flat_deposit_cmdi_form_wrapper'>";
    $form['#suffix'] = "</div>";

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


/*
    $form['select'] = array(
        '#prefix' => '<div id="file-import-area">',
        '#suffix' => '</div>',
    );
    $form ['select']['file_trigger'] = array(
        '#title' => t('Use file to import metadata'),
        '#type' => 'checkbox',
        '#required' => FALSE,
        '#states' => array(
            'invisible' => array(
                ':input[name="select_profile_name"]' => array('value' => ''),
            ),
        ),
    );
*/
    /*remove when doen
    $form['im-container'] = array(
        '#prefix' => '<div id="im-area">',
        '#suffix' => '</div>',
    );

    $form['image_file'] = array(
        '#type' => 'file',
    );

    $form['upload'] = array(
        '#type' => 'submit',
        '#value' => 'upload',
        '#submit' => array('upload_image'),
        '#ajax' => array(
            'callback' => 'upload_image',
            'wrapper' => 'im-area',
            'method' => 'replace',
            'effect' => 'fade',
        ),
    );
*/

/*
    $form ['select']['cmdi_file'] = array(
        '#type' => 'file',
        '#title' => t('Choose a file'),
        '#states' => array(
            'visible' => array(
                ':input[name="file_trigger"]' => array('checked' => TRUE),
            ),
            'required' => array(
                ':input[name="file_trigger"]' => array('checked' => TRUE),
            ),
        ),
    );

    $form ['select']['populate_submit'] = array(
        '#type' => 'submit',
        '#value' => 'upload',
        '#submit' => array('upload_cmdi'),
        '#ajax' => array(
            'callback' => 'upload_cmdi',
            'wrapper' => 'file-import-area',
            'method' => 'replace',
            'effect' => 'fade',
        ),
        '#states' => array(
            'visible' => array(
                ':input[name="file_trigger"]' => array('checked' => TRUE),
            ),
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
            'parent_nid' => $parent_nid,
        ),
    );

    $form['Submit'] = array(
        '#type' => 'submit',
        '#value' => t('Submit'),
        // @TODO: (metadata template issue) ask paul for this
        '#ajax' => array(
            'callback' => 'flat_deposit_cmdi_form_ajax_handler',
            'wrapper' => 'flat_deposit_cmdi_form_wrapper',
            'effect' => 'fade',
        ),
        '#validate' => array('flat_deposit_cmdi_form_final_validate'),
    );

    //********************************************************************
    // Generate profile specific form render array and attach to container
    //********************************************************************

    // load template if selected
    CmdiTemplateManager::load($form_state);

    // adding modal component to form
    $form['flat_modal'] = CmdiTemplateManager::modal();

    // adding save cmdi template feature
    CmdiTemplateManager::save($form_state);

    if (isset($form_state['selected']) AND !empty($form_state['selected'])) {
        module_load_include('inc', 'flat_deposit', 'Helpers/CMDI/class.CmdiHandler');
        $check = FormBuilder::checkTemplateItems($form_state['selected']);
        if ($check !== TRUE) {
            drupal_set_message($check, 'warning');
            return $form;
        }

        // Load inherited cmdi metadata from parent node
        $parent = node_load($parent_nid);
        $pwrapper = entity_metadata_wrapper('node', $parent);
        $pFid = $pwrapper->flat_fid->value();

        $ds = islandora_datastream_load('CMD', $pFid);

        if ($ds) {
            $inheritedData = CmdiHandler::simplexml_load_cmdi_string($ds->content);
        } else {
            $inheritedData = null;
        }

        // Load form builder app
        $templateName = $form_state['selected'];
        $node = $form_state['build_info']['args'][0];

        $formBuilder = new FormBuilder($node);

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

        // Clear form state input for when switching between templates
        // -> TODO: need to figure out a different way to do this, as this will clear the form on all ajax callbacks
        //unset($form_state['input']['template_container']['elements']);

        // Attach form elements to base form
        $elements = $formBuilder->getForm();

        // assigning form
        $form['template_container']['elements'] = $elements;

        // check if everything worked as expected
        if (!is_array($form['template_container']['elements'])) {
            drupal_set_message('Unable to generate cmdi form based on profile', 'warning');
        }
    }

    return $form;
}

function flat_deposit_cmdi_form_ajax_handler(&$form, &$form_state) {

    module_load_include('inc', 'flat_deposit', 'Helpers/CMDI/CmdiTemplate/class.CmdiValueSyncer');
    CmdiValueSyncer::sync($form, $form_state);

    return $form;
}

/*
function upload_image($form, $form_state) {

    $file = file_save_upload('image_file', array('file_validate_extensions' => array('png gif jpg jpeg')), "public://", FILE_EXISTS_REPLACE);
    if ($file) {
        $file->status = FILE_STATUS_PERMANENT;
        file_save($file);
        $form['im-container'] = array(
                '#title' => t('Preview:'),
                '#prefix' => '<div id="im-area">',
                '#markup' => '<img src="sites/default/files/'.$file -> filename.
            '" height="250" width="250" />',
            '#suffix' => '</div>',
        );
    } else {
        drupal_set_message('No file uploaded.');
    }

    return $form['im-container'];
}
*/
/*
function upload_cmdi($form, $form_state) {

    global $user;
    $fileName = "$user->uid" . "_record.cmdi";
    $file = file_save_upload('cmdi_file', array('file_validate_extensions' => array('cmdi')), "temporary://$fileName", FILE_EXISTS_REPLACE);
    if ($file) {
        $file->status = FILE_STATUS_PERMANENT;
        file_save($file);

    } else {
        drupal_set_message('No file uploaded.');
    }

    return $form['file-import-area'];
}
*/
function flat_deposit_cmdi_form_final_validate($form, &$form_state)
{
    // Form Validation setup
    $nid = $form_state['build_info']['args'][0]->nid;
    $node = node_load($nid);
    $wrapper = entity_metadata_wrapper('node', $node);

    $title = $node->title;
    $collection = $wrapper->flat_parent_title->value();

    $owner = $form_state['values']['owner'];
    $profile = $form_state['values']['select_profile_name'];

    $export_dir = 'metadata://' . '/' . str_replace('@', '_at_', $owner) . "/$collection/$title";
    $cmdiFile = $export_dir . '/' . $profile . '_' . uniqid() . '.cmdi';

    $form_state['values']['cmdiFile'] = $cmdiFile;

    $parent_nid = $form_state['values']['data']['parent_nid'];


    //****************************//
    // Perform general validation //
    //****************************//

    // stop validation immediately if errors have previously occurred
    if (form_get_errors()) {
        return $form;
    }

    // Validate that selected profile is not empty
    if ($form_state['values']['select_profile_name'] === '-- Select --') {
        form_set_error('select_profile_name', t('Please choose an option'));
        return $form;
    }

    // Validate if owner exists.
    if (!user_load_by_name($owner)) {
        form_set_error('owner', t('Specified owner is unknown'));
        return $form;
    };

    // Validate that output directory for new cmdi exists or can be created
    if (!file_exists($export_dir)) {
        drupal_mkdir($export_dir, NULL, TRUE);
    }

    if (!file_exists($export_dir)) {
        form_set_error('error', t('Cannot create directory to temporarily store cmdi files'));
        return $form;
    }


    //*******************************************//
    // Perform validation specific chosen option //
    //*******************************************//


    //*******************//
    // Title Validations //
    //*******************//
    module_load_include('inc', 'flat_deposit', 'Helpers/CMDI/FormBuilder/class.FormBuilder');
    $templateName = $form_state['selected'];

    $title = $form_state['values']['template_container']['elements']['title_field'][0];

    if (!$title) {
        form_set_error('error', t('title is not set for this cmdi file'));
        return $form;
    }

    module_load_include('inc', 'flat_deposit', 'inc/class.FlatTuque');
    $parent_node = node_load($parent_nid);
    $parent_wrapper = entity_metadata_wrapper('node', $parent_node);

    $parent_fid = $parent_wrapper->flat_fid->value();
    $values = FlatTuque::getChildrenLabels($parent_fid);

    if ($values === false) {
        form_set_error('title', t('Unable to validate that collection name is unique at this location'));
        return $form;
    }

    if (in_array(strtoupper($title), array_unique(array_map('strtoupper', $values)))) {
        form_set_error('title', t('Another collection or bundle with same name exists at this location. Please use a different name'));
        return $form;
    }


    //*******************//
    // Generate Cmdi file//
    //*******************//
    module_load_include('inc', 'flat_deposit', 'Helpers/CMDI/CmdiCreator/class.CmdiCreator');
    module_load_include('inc', 'flat_deposit', 'Helpers/CMDI/CmdiTemplate/class.CmdiValueExtractor');

    $owner = $form_state['values']['owner'];
    $form_data = CmdiValueExtractor::extract($form_state);

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
        form_set_error('error', t('Unable to save cmdi file'));
    }


    //***************//
    // Validate cmdi //
    //***************//
    // Cmdi validation does not make sense as Cmdis will not contain resources at this stage


    //***********************//
    // Validate cmdi profile //
    //***********************//
    module_load_include('inc', 'flat_deposit', 'Helpers/CMDI/class.CmdiHandler');
    $cmdi = CmdiHandler::simplexml_load_cmdi_file(drupal_realpath($cmdiFile));
    try {
        $cmdi->canBeValidated();
    } catch (CmdiHandlerException $cmdiHandlerException) {
        form_set_error('cmdi_file', $cmdiHandlerException->getMessage());
        return $form;
    }
    return $form;
}



function flat_deposit_cmdi_form_submit($form, &$form_state)
{
    // $node = menu_get_object();
    // $wrapper = entity_metadata_wrapper('node', $node);
    ctools_include('ajax');
    ctools_add_js('ajax-responder');

    $nid = $form_state['build_info']['args'][0]->nid;
    $node = node_load($nid);
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

    $node = node_load($nid);
    $wrapper = entity_metadata_wrapper('node', $node);

    $wrapper->flat_location->set($flat_location_original);
    $wrapper->flat_original_path->set($flat_original_path_original);
    $wrapper->save();

    drupal_set_message(t('Metadata for bundle %title has been saved', array('%title' => $node->title)));

    $commands[] = ctools_ajax_command_redirect('node/' . $nid);

    print ajax_render($commands);
    exit;
}
