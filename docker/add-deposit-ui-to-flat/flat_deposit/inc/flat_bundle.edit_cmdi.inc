<?php

function flat_deposit_bundle_edit_cmdi_form($form, &$form_state)
{

    drupal_set_title(t('Edit Bundle Metadata'));

    $nid = $form_state['build_info']['args'][0]->nid;

    $wrapper = entity_metadata_wrapper('node', $nid);

    ctools_add_js('ajax-responder');

    module_load_include('inc', 'flat_deposit', 'Helpers/CMDI/class.CmdiHandler');

    // exit form generation if no parent collection has been selected
    if (isset($form_state['parent_nid'])) {
        $parent_nid = $form_state['parent_nid'];
    } else {
        $parent_nid = $wrapper->flat_parent_nid->value();
        $form_state['parent_nid'] = $parent_nid;
    }

    if ($parent_nid === '0') {
        drupal_set_message('Cannot generate or edit form because collection is not specified', 'error');
        return $form;
    }

    $existing_cmdi = $wrapper->flat_cmdi_file->raw()['uri'];
    $inheritedData = CmdiHandler::simplexml_load_cmdi_file(drupal_realpath($existing_cmdi));
    if ($inheritedData) {
        $profile = $inheritedData->getNameById();
    } else {
        $profile= null;
    }

    $form['#prefix'] = "<div id='flat_bundle_edit_cmdi_form_wrapper'>";
    $form['#suffix'] = "</div>";

    global $user;
    $form['owner'] = array(
        '#title' => t('Owner of the collection'),
        '#type' => 'hidden',
        '#required' => TRUE,
        '#default_value' => $user->name,
    );

    $form['template_container'] = array(
        '#type' => 'container',
        '#tree' => TRUE,
        '#attributes' => array(
            'id' => array('template-form'),
        ),
    );

    $form['trigger']['select_profile_name'] = array(
        '#type' => 'value',
        '#value' => $profile,
    );

    if ($inheritedData) {
        $form['data']['#value']['handle'] = (string)$inheritedData->children('cmd', TRUE)->Header->MdSelfLink;
    }

    $form['Submit'] = array(
        '#type' => 'submit',
        '#value' => t('Submit'),
        '#ajax' => array(
            'callback' => 'flat_bundle_edit_cmdi_form_ajax_handler',
            'wrapper' => 'flat_bundle_edit_cmdi_form_wrapper',
            'effect' => 'fade',
        ),
        '#validate' => array('flat_deposit_bundle_edit_cmdi_form_final_validate'),
    );



    //********************************************************************
    // Generate profile specific form render array and attach to container
    //********************************************************************
    module_load_include('inc', 'flat_deposit', 'Helpers/CMDI/FormBuilder/class.FormBuilder');

    // load template if selected
    CmdiTemplateManager::load($form_state);

    // adding modal component to form
    $form['flat_modal'] = CmdiTemplateManager::modal();

    // adding save cmdi template feature
    $saved = CmdiTemplateManager::save($form_state);

    $availableFormTemplates = FormBuilder::getAvailableTemplates('flat_bundle');



    if (array_key_exists($profile, $availableFormTemplates)) {
        // Load form builder app
        module_load_include('inc', 'flat_deposit', 'Helpers/CMDI/FormBuilder/class.FormBuilder');
        $formBuilder = new FormBuilder($form_state['build_info']['args'][0]);

        // count button presses per field
        $formBuilder->aggregatePressedButtons($form_state);

        // get the node in nested array from which we can start iterating all button presses per field
        if (isset($form_state['pressedButtons']['template_container']['elements'])) {
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
        drupal_set_message('Online editing of the metadata of this bundle is not supported');
        $form['Submit']['#disabled'] = TRUE;
    }
    return $form;
}

function flat_bundle_edit_cmdi_form_ajax_handler(&$form, &$form_state)
{
    module_load_include('inc', 'flat_deposit', 'Helpers/CMDI/CmdiTemplate/class.CmdiValueSyncer');
    CmdiValueSyncer::sync($form, $form_state);

    return $form;
}

function flat_deposit_bundle_edit_cmdi_form_final_validate($form, &$form_state)
{
    // Form Validation setup
    $nid = $form_state['build_info']['args'][0]->nid;
    $node = node_load($nid);
    $wrapper = entity_metadata_wrapper('node', $node);


    $title = $node->title;

    $owner = $form_state['values']['owner'];
    $profile = $form_state['values']['select_profile_name'];

    $cmdiFile = $wrapper->flat_cmdi_file->raw()['uri'];

    $form_state['values']['cmdiFile'] = $cmdiFile;

    $parent_nid = $wrapper->flat_parent_nid->value();

    //****************************//
    // Perform general validation //
    //****************************//

    // stop validation immediately if errors have previously occurred
    if (form_get_errors()) {
        return $form;
    }

    // Validate if owner exists.
    if (!user_load_by_name($owner)) {
        form_set_error('owner', t('Specified owner is unknown'));
        return $form;
    };

    //*******************************************//
    // Perform validation specific chosen option //
    //*******************************************//


    //*******************//
    // Title Validations //
    //*******************//
    module_load_include('inc', 'flat_deposit', 'Helpers/CMDI/FormBuilder/class.FormBuilder');
    $templateName = $form_state['values']['select_profile_name'];

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



function flat_deposit_bundle_edit_cmdi_form_submit($form, &$form_state)
{
    ctools_include('ajax');
    ctools_add_js('ajax-responder');

    $nid = $form_state['build_info']['args'][0]->nid;
    $node = node_load($nid);

    drupal_set_message(t('Metadata for bundle %title has been saved', array('%title' => $node->title)));

    $commands[] = ctools_ajax_command_redirect('node/' .$nid);
    print ajax_render($commands);
    exit;

}
