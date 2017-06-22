<?php


function flat_bundle_action_form($form, &$form_state)
{
    form_load_include($form_state, 'inc', 'flat_deposit', 'inc/flat_bundle_action_helpers');

    $form['actions']['bundle_image'] = array(
        '#type' => 'image_button',
        '#title' => 'test',
        '#value' => t('Bundle image'),
        '#disabled' => TRUE,
        '#prefix' => '<div><br></div>',

    );

    $form['actions']['container'] = array(
        '#type' => 'container',
        '#attributes' => array('class' => array('container-inline')),
    );

    $form['actions']['describe_bundle'] = array(
        '#type' => 'submit',
        '#value' => t('Fill in metadata for bundle'),
        #'#src' => drupal_get_path('module','flat_deposit').'/Images/arrow_right.png',
        '#description' => t('Bundles are obliged to be accompanied by metadata'),
        '#validate' => array('flat_bundle_action_form_describe_validate'),
        '#disabled' => TRUE,
    );

    $form['actions']['upload_data'] = array(
        '#type' => 'submit',
        '#value' => t('Upload data'),
        '#disabled' => TRUE,
    );


    $form['actions']['markup_1'] = array(
        '#markup' => '<div><br></div>'
    );


    $form['actions']['validate_bundle'] = array(
        '#type' => 'submit',
        '#value' => t('Validate bundle'),
        '#validate' => array('flat_bundle_action_form_validate_validate'),
        #'#src' => drupal_get_path('module','flat_deposit').'/Images/arrow_right.png',
        '#description' => t('Validated bundles will be transferred to different location and can not be altered'),
        '#disabled' => TRUE,
    );

    $form['actions']['reopen_bundle'] = array(
        '#type' => 'submit',
        '#value' => t('Re-open bundle'),
        '#validate' => array('flat_bundle_action_form_reopen_validate'),
        #'#src' => drupal_get_path('module','flat_deposit').'/Images/arrow_right.png',
        '#description' => t('Click to allow modifications of your bundle'),
        '#disabled' => TRUE,
    );

    $form['actions']['archive_bundle'] = array(
        '#type' => 'submit',
        '#value' => t('Archive bundle'),
        #'#validate' => array('flat_bundle_action_form_archive_validate'),
        #    '#src' => drupal_get_path('module','flat_deposit').'/Images/arrow_right.png',
        '#disabled' => TRUE,
    );

    $form['actions']['edit_bundle'] = array(
        '#type' => 'submit',
        '#value' => t('Edit bundle'),
        #    '#src' => drupal_get_path('module','flat_deposit').'/Images/arrow_right.png',
        #'#submit' => array('flat_bundle_action_form_bundle_edit_submit'),
        '#prefix' => '<div><br></div>',
    );

    $form['actions']['delete_bundle'] = array(
        '#type' => 'submit',
        '#value' => t('Delete bundle'),
        #    '#src' => drupal_get_path('module','flat_deposit').'/Images/arrow_right.png',
        '#suffix' => '<div><br></div>',
    );

    $node = menu_get_object();

    $form['values']['node'] = array(
        '#type' => 'value',
        '#value' => $node
    );

    $form['values']['origin_url'] = array(
        '#type' => 'value',
        '#value' => 'node/' . $node->nid);

    if (user_role_load_by_name('developer')) {
        $form['options']['serial'] = array(
            '#type' => 'checkbox',
            '#title' => t('Serial processing (debug mode)'),
            '#prefix' => '</p>',
            '#default_value' => 1,

        );
    }


    return $form;
}



function flat_deposit_form_flat_bundle_action_form_alter(&$form, &$form_state, $form_id){

    $node = menu_get_object();
    $wrapper = entity_metadata_wrapper('node', $node);

    // Specify which handlers to become active and which bundle_image to display
    $form['actions']['edit_bundle']['#disabled'] = TRUE;
    $status = $wrapper->flat_bundle_status->value();


    # deactivate link to file browser for local bundles
    $source = $wrapper->flat_source->value();
    $disable_file_browser = $source == 'local' ? TRUE : FALSE;


    switch ($status) {
        case 'open': {
            $form['actions']['upload_data']['#disabled'] = $disable_file_browser;
            $form['actions']['bundle_image']['#disabled'] = $disable_file_browser;
            $form['actions']['validate_bundle']['#disabled'] = FALSE;
            $form['actions']['edit_bundle']['#disabled'] = FALSE;
            $name = 'open_bundle';
            break;
        }

        case 'validating': {
            $name = 'process_bundle';
            $form['actions']['reopen_bundle']['#disabled'] = FALSE;
            break;
        }

        case 'failed': {
            $form['actions']['upload_data']['#disabled'] = $disable_file_browser;
            $form['actions']['bundle_image']['#disabled'] = $disable_file_browser;
            $form['actions']['validate_bundle']['#disabled'] = FALSE;
            $form['actions']['edit_bundle']['#disabled'] = FALSE;
            $name = 'closed_bundle_failed';
            break;
        }

        case 'valid': {
            $form['actions']['archive_bundle']['#disabled'] = FALSE;
            $form['actions']['reopen_bundle']['#disabled'] = FALSE;
            $name = 'closed_bundle';
            break;
        }
        case 'processing': {
            $name = 'process_bundle';

            break;
        }
    }

    $form['actions']['bundle_image']['#src'] = drupal_get_path('module', 'flat_deposit') . '/Images/' . $name . '.jpg';


    // Activate metadata creation button only if 1) cmdi option is 'new' 2) status is either open or failed and 3) no cmdi has been created
    if ($wrapper->flat_cmdi_option->value() == 'new' AND ($status == 'open' OR $status == 'failed') AND (empty($node->flat_cmdi_file))) {

        $form['actions']['describe_bundle']['#disabled'] = FALSE;
    }

}


/**
 * Implements hook_form_validate.
 *
 * Validation of opening the bundle data browser
 *
 */
function flat_bundle_action_form_describe_validate($form, &$form_state){

    $node = $form_state['values']['node'];
    $wrapper = entity_metadata_wrapper('node', $node);

    $parent_nid = $wrapper->flat_parent_nid->value();
    if ($parent_nid === '0'){
        form_set_error('describe_bundle', "To generate or edit a cmdi file, you need to select a collection first");
        return $form;

    }

}

/**
 * Implements hook_form_validate.
 *
 * Validation of opening the bundle data browser
 *
 */
function flat_bundle_action_form_reopen_validate($form, &$form_state){

    $node = $form_state['values']['node'];
    $error = move_node_data($node->nid);
    if (!empty($error)){

        form_set_error('reopen', t('Unable to move bundle !what to unfreeze', array('!what'=>implode(' AND ', $error))));

    }

}






function flat_bundle_action_form_validate_validate($form, &$form_state){

    $node = $form_state['values']['node'];
    $nid = $node->nid;

    $parent_is_known = parent_is_known($nid);
    $has_cmdi = has_cmdi($nid);
    $valid_xml = is_valid_xml($nid, $has_cmdi);
    $good_name = has_correct_filename($nid);
    $file_exists = bundle_file_exists($nid);
    $user_has_permissions = user_has_permissions($nid);


    // validate that a collection has been selected
    if ($parent_is_known === false){
        form_set_error('error', "Bundle has not been assigned to a collection");
        return $form;
    }

    // In case no cmdi file exists
    if ($has_cmdi === false){
        form_set_error('error', "No metadata file has been specified");
        return $form;
    }

    // Quick and dirty Check cmdi valid xml
    if ($valid_xml === false){
        form_set_error('validate_bundle', t("Attached file is no valid xml file"));
        return $form;
    }

    // In case of wrong naming
    if ($good_name === false){
        form_set_error('validate_bundle', t("Metadata file has wrong file name (record.cmdi expected)"));
        return $form;
    }

    // Check existence external location
    if ($file_exists === false){
        form_set_error('validate_bundle',t('File location does not exist (:path) ' , array(':path' => '/path/to/file')));
        return $form;

    }

    // Check user permissions
    if ($user_has_permissions === false){
        form_set_error('validate_bundle', t('User has not enough privileges to perform requested action'));
        return $form;
    }

}


function flat_bundle_action_form_submit($form, &$form_state)
{

    $node = $form_state['values']['node'];
    $wrapper = entity_metadata_wrapper('node', $node);

    $action = $form_state['clicked_button']['#value'];

    switch ($action) {
        case 'Bundle image':
        case 'Upload data':
            global $user;
            $form_state['redirect'] = array('user/' . $user->uid . '/imce');
            break;


        case 'Fill in metadata for bundle':
            $url = array('node/' . $node->nid . '/cmdi');
            $form_state['redirect'] = $url;
            break;


        case 'Validate bundle':
        case 'Archive bundle':

            $debug = isset($form_state['values']['serial']) ? $form_state['values']['serial'] : false;

            send_request($node->nid, $action, $debug);

            $processed = ($wrapper->flat_bundle_status->value() == 'valid') ? 'archived' : 'validated';

            $form_state['redirect'] = 'workspace';
            drupal_set_message("Bundle is being $processed");

            break;


        case 'Edit bundle':
            $url = array('node/' . $node->nid . '/edit');
            $form_state['redirect'] = $url;
            break;


        case 'Delete bundle':
            $url = array('node/' . $node->nid . '/delete');
            $form_state['redirect'] = $url;

            break;

        case 'Re-open bundle':

            $wrapper->flat_bundle_status->set('open');
            $wrapper->save();
            drupal_set_message('Bundle is open and can be modified again');

            break;


    }


}
