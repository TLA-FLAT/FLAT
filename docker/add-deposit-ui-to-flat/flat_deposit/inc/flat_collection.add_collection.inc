<?php

function flat_collection_add_form($form, &$form_state, $fedoraObject)
{
    drupal_set_title(t('Add Collection'));

    ctools_add_js('ajax-responder');

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
    // Field for entering namespace value should only be shown to admin and manager users
    if(count(array_intersect(array('manager', 'administrator'), $user->roles)) > 0) {
	    $show_namespace_field = TRUE;
    }
    else {
	    $show_namespace_field = FALSE;
    }
    $form['namespace_toggle'] = array(
        '#title' => t('Enter namespace value'),
        '#type' => 'checkbox',
        '#required' => FALSE,
        '#access' => $show_namespace_field,
    );

    $form['namespace'] = array(
        '#title' => t('Namespace for the collection (leave blank to use parent namespace)'),
        '#type' => 'textfield',
	'#required' => FALSE,
	'#access' => $show_namespace_field,
	  '#states' => array(
    	   // Only show this field when the 'toggle_me' checkbox is enabled.
    		'visible' => array(
      			':input[name="namespace_toggle"]' => array(
        			'checked' => TRUE,
      			),
    		),
  	),
    );

    if (!user_access('admin collection')) {
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
        '#description' => t('Select which access policy should be applied as the default for bundles within this collection. "Public" materials can be accessed by anyone without having to log in. "Authenticated Users" means any user with a valid account for the archive. "Academic Users" are users that logs in with an academic account or whose academic status has been verified. "Private" means that the materials are only accessible to the depositor. Access policies can be refined later.'),
        '#required' => TRUE,
        '#options' => array('public' => t('public'), 'authenticated' => t('authenticated users'), 'academic' => t('academic users'), 'private' => t('private')),
        '#default_value' => 'public',
    );

    $form['visibility'] = array(
        '#title' => t('Visibility'),
        '#type' => 'select',
        '#description' => t('Hidden collections are not visible to anyone but the depositor and the archive managers. This is to be used only in cases in which the name or other metadata fields reveal too much information about work in progresss. The visibility will be applied to this collection and will be the default value for any sub-collections or bundles within this collection. Only materials with a private access policy can be hidden.'),
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

    // load template if selected
    CmdiTemplateManager::load($form_state);

    // adding modal component to form
    $form['flat_modal'] = CmdiTemplateManager::modal();

    // adding save cmdi template feature
    $saved = CmdiTemplateManager::save($form_state);

    if (isset($form_state['selected']) and $form_state['selected'] != 'Import' AND $form_state['selected'] != '') {

        $inheritedData = null;
        #$inherit = $form_state['values']['inherit_from_collection'];

        // Load inherited cmdi metadata from node
        $cmdiDs = islandora_datastream_load('CMD', $fedoraObject);
        if ($cmdiDs) {
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
            drupal_set_message(t('Unable to generate CMDI form based on selected profile'), 'warning');
        }
    }
    return $form;
}

function flat_deposit_form_flat_collection_add_form_alter(&$form, &$form_state) {
    // CmdiTemplateManager::alter($form, $form_state);
}

function flat_collection_add_form_final_validate($form, &$form_state) {

    module_load_include('inc', 'flat_deposit', 'inc/class.FlatTuque');
    // Form Validation setup
    $owner = $form_state['values']['owner'];
    $namespace = $form_state['values']['namespace'];
    $profile = $form_state['values']['select_profile_name'];
    $allowed_collection_profiles = variable_get('flat_deposit_cmdi_profiles')['collection_profile_ids'];
    $allowed_collection_profile_ids = explode(',', $allowed_collection_profiles);
    $export_dir = 'metadata://' . str_replace('@', '_at_', $owner) . "/.collection/";
    $cmdiFile = $export_dir . '/' . $profile . '_' . uniqid() . '.cmdi';
    $form_state['values']['cmdiFile'] = $cmdiFile;

    $fid = $form_state['values']['data']['fid'];
    // stop validation if errors have previously occurred
    if (form_get_errors()) {
        return $form;
    }

    //****************************//
    // Perform general validation //
    //****************************//

    // Validate that selected profile is not empty
    if ($form_state['values']['select_profile_name'] === '-- Select --') {
        form_set_error('select_profile_name', t('Please choose an option from the list'));
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
        form_set_error('error', t('Cannot create a directory to temporarily store CMDI files'));
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
            $values = FlatTuque::getChildrenLabels($fid);

            if ($values === false) {
                form_set_error('title', t('Unable to validate that collection name is unique at this location'));
                return $form;
            }

            if (in_array(strtoupper($title), array_unique(array_map('strtoupper', $values)))) {
                form_set_error('title', t('Another collection or bundle with same name exists at this location. Please use a different name'));
                return $form;
            }

            // 2. Validate that output directory for new cmdi exists or can be created
            if (!file_exists($export_dir)) {
                drupal_mkdir($export_dir, NULL, TRUE);
            }

            if (!file_exists($export_dir)) {
                form_set_error('error', t('Cannot create directory to temporarily store cmdi files'));
                return $form;
            }


            //*******************//
            // Generate Cmdi file//
            //*******************//
            module_load_include('inc', 'flat_deposit', 'Helpers/CMDI/CmdiCreator/class.CmdiCreator');
            module_load_include('inc', 'flat_deposit', 'Helpers/CMDI/CmdiTemplate/class.CmdiValueExtractor');

            $templateName = $form_state['selected'];
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
                form_set_error('error', t('Unable to save CMDI file on the server'));
                return $form;
            }

            break;

        case 'Import':
            module_load_include('inc', 'flat_deposit', 'Helpers/CMDI/class.CmdiHandler');
            module_load_include('inc', 'flat_deposit', 'Helpers/CMDI/FormBuilder/class.FormBuilder');

            $file = file_save_upload('cmdi_file', array(
                // Validate file extensions
                'file_validate_extensions' => array('cmdi'),
            ));
            if (!$file) {
                // No file specified or has incorrect extension
                form_set_error('cmdi_file', t('No file was specified or it has an incorrect file extension (should be .cmdi)'));
                return $form;
            }
            $cmdi = CmdiHandler::simplexml_load_cmdi_file(drupal_realpath($file->uri));
            // Valid xml?
            if (!$cmdi) {
                form_set_error('cmdi_file', t('Your uploaded CMDI file is not a valid XML file'));
                return $form;
            }
            // Check whether CMDI file has allowed CMDI collection profile
            $type = $cmdi->getCmdiProfileType();
            if ($type !== 'collection') {
                form_set_error('cmdi_file', t('Your uploaded CMDI file has a profile that is not accepted as a Collection profile. See the deposit manual for more information about accepted CMDI profiles.'));
                return $form;
            }

            // Check that no other collection/bundle exists at this level with same or very similar name
            $profile_name = $cmdi->getNameById();
            $profile_filename = FormBuilder::FORM_TEMPLATES_PATH . $profile_name .".xml";
            $template_xml = simplexml_load_file($profile_filename);
            $template_name = (string)$template_xml->xpath('/profileToDrupal/header/template_name')[0];
            $title_field = (string)$template_xml->xpath('/profileToDrupal/items/item[@id="title_field"]/@name')[0];
            $values = FlatTuque::getChildrenLabels($fid);
            $title = (string)$cmdi->xpath("/cmd:CMD/cmd:Components/cmd:$template_name/cmd:$title_field")[0];
            if (!$title) {
                //let's try without namespace
                $title = (string)$cmdi->xpath("/CMD/Components/$template_name/$title_field")[0];
            }

            if (!$title) {
                form_set_error('cmdi_file', t('Unable to read the collection name from your uploaded CMDI file'));
                return $form;
            }

            if ($values === false) {
                form_set_error('cmdi_file', t('Unable to validate that collection name is unique at this location'));
                return $form;
            }

            if (in_array(strtoupper($title), array_unique(array_map('strtoupper', $values)))) {
                form_set_error('cmdi_file', t('Another collection or bundle with same name exists at this location. Please use a different name'));
                return $form;
            }

            // Remove MdSelfLink (new collections cannot have an existing MdSelfLink)
            $cmdi->removeMdSelfLink();

            // Remove Resources (new collections should not already link to resources)
            $cmdi->stripResourceProxyAndResources();

            $exported = $cmdi->asXML($cmdiFile);

            if (!$exported) {
                form_set_error('error', t('Unable to save CMDI file on the server'));
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

    module_load_include('php', 'flat_deposit', 'Helpers/IngestService/IngestClient');
    try {
        $ingest_client = new IngestClient($sipType, $owner, $cmdiFile, $fid, $test, $namespace);
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

    if (!$fObject) {
        form_set_error('error', t('Check of FID for new collection item did not reveal valid data. Error message:' . $fid));
        return $form;
    }

    $form_state['values']['data']['fid'] = (string)$fid;
    $form_state['values']['data']['label'] = $fObject->label;
    $form_state['values']['data']['owner'] = $fObject->owner;

    return $form;
}


/**
* Creates new flat_collection node and redirects to parent node.
 *
 */
function flat_collection_add_form_submit($form, &$form_state)
{

    $owner = user_load_by_name($form_state['values']['data']['owner']);
    $uid = $owner->uid;
    $label = $form_state['values']['data']['label'];
    $fid = $form_state['values']['data']['fid'];
    $target = 'islandora/object/' . $form_state['values']['data']['fid'];
    create_collection_node($label, $uid, $fid);

    drupal_set_message(t('New collection item has been created'));

    ctools_include('ajax');
    ctools_add_js('ajax-responder');
    $commands[] = ctools_ajax_command_redirect($target);
    print ajax_render($commands);
    exit;
}


function create_collection_node($label, $uid, $fid)
{
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

function flat_collection_add_form_ajax_handler(&$form, &$form_state)
{
    module_load_include('inc', 'flat_deposit', 'Helpers/CMDI/CmdiTemplate/class.CmdiValueSyncer');
    CmdiValueSyncer::sync($form, $form_state);

    return $form;
}
