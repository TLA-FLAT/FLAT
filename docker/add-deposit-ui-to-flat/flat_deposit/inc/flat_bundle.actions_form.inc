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
        '#description' => t('Enter metadata for this bundle (required)'),
        '#validate' => array('flat_bundle_action_form_describe_validate'),
        '#access' => FALSE,
    );

    $form['actions']['upload_data'] = array(
        '#type' => 'submit',
        '#value' => t('Upload data'),
        '#access' => FALSE,
    );


    $form['actions']['markup_1'] = array(
        '#markup' => '<div><br></div>'
    );


    $form['actions']['validate_bundle'] = array(
        '#type' => 'submit',
        '#value' => t('Validate bundle'),
        '#validate' => array('flat_bundle_action_form_validate_validate'),
        '#description' => t('Validate the bundle. Valid bundles cannot be altered, unless they are re-opened again.'),
        '#disabled' => TRUE,
    );

    $form['actions']['reopen_bundle'] = array(
        '#type' => 'submit',
        '#value' => t('Re-open bundle'),
        '#validate' => array('flat_bundle_action_form_reopen_validate'),
        '#description' => t('Re-open the bundle to allow modifications of its metadata or included files'),
        '#disabled' => TRUE,
    );

    $form['actions']['archive_bundle'] = array(
        '#type' => 'submit',
        '#value' => t('Archive bundle'),
	    '#description' => t('Submit the bundle to be stored in the archive.'),
        '#disabled' => TRUE,
    );

    $form['actions']['edit_bundle'] = array(
        '#type' => 'submit',
        '#value' => t('Edit bundle properties'),
        '#prefix' => '<div><br/></div>',
    );

    $form['actions']['delete_bundle'] = array(
        '#type' => 'submit',
        '#value' => t('Delete bundle'),
        '#suffix' => '<div><br/></div>',
    );

    $form['actions']['note'] = array(
	'#prefix' => '<div id="form-actions-note">',
	'#markup' => t('Note: Deleting a bundle will only delete it from your active bundles. In case you are modifying an existing bundle in the archive, clicking "Delete bundle" will leave the original in the archive untouched.'),
	'#suffix' => '</div><div><br/></div>',
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
            '#default_value' => 0,

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
    $enable_file_browser = in_array($source, array('local', 'cloud')) ? FALSE : TRUE;


    switch ($status) {
        case 'open': {

            $form['actions']['upload_data']['#access'] = $enable_file_browser;
            $form['actions']['bundle_image']['#disabled'] = !$enable_file_browser;
            if (empty($node->flat_cmdi_file)) {
                $form['actions']['validate_bundle']['#disabled'] = TRUE;
            }
            else {
                $form['actions']['validate_bundle']['#disabled'] = FALSE;
                $form['actions']['validate_bundle']['#attributes']['class'][] = 'btn-success';
            }
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
            $form['actions']['upload_data']['#access'] = $enable_file_browser;
            $form['actions']['bundle_image']['#disabled'] = !$enable_file_browser;
            $form['actions']['validate_bundle']['#disabled'] = FALSE;
            $form['actions']['edit_bundle']['#disabled'] = FALSE;
            $name = 'closed_bundle_failed';
            break;
        }

        case 'valid': {
            $form['actions']['archive_bundle']['#disabled'] = FALSE;
            $form['actions']['reopen_bundle']['#disabled'] = FALSE;
            $form['actions']['archive_bundle']['#attributes']['class'][] = 'btn-success';
            $name = 'closed_bundle';
            break;
        }
        case 'processing': {
            $name = 'process_bundle';

            break;
        }
    }

    $form['actions']['bundle_image']['#src'] = drupal_get_path('module', 'flat_deposit') . '/Images/' . $name . '.png';


    // Activate metadata creation button only if 1) cmdi option is 'new' 2) status is either open or failed and 3) no cmdi has been created
    // If metadata creation button is active (.e.g. if metadata file has been removed), validate button should be disabled.
    if ($wrapper->flat_cmdi_option->value() == 'new' AND ($status == 'open' OR $status == 'failed') AND (empty($node->flat_cmdi_file))) {

        $form['actions']['describe_bundle']['#access'] = TRUE;
        $form['actions']['describe_bundle']['#attributes']['class'][] = 'btn-success';
        $form['actions']['validate_bundle']['#disabled'] = TRUE;

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

        form_set_error('reopen', t('Unable to move bundle !what to unfreeze', array('!what'=>$node->nid)));

    }

}






function flat_bundle_action_form_validate_validate($form, &$form_state){

    $node = $form_state['values']['node'];
    $nid = $node->nid;
    $wrapper = entity_metadata_wrapper('node', $node);
    $path = $wrapper->flat_location->value();

    $parent_is_known = parent_is_known($nid);
    $has_cmdi = has_cmdi($nid);
    $valid_xml = is_valid_xml($nid, $has_cmdi);
    //$good_name = has_correct_filename($nid); filename here not relevant, only in SIP that is offered to doorkeeper
    $file_exists = bundle_file_exists($nid);  
    $user_has_permissions = user_has_permissions($nid);


    // validate that a collection has been selected
    if ($parent_is_known === false){
        form_set_error('error', "The bundle has not been assigned to a collection");
        return $form;
    }

    // In case no cmdi file exists
    if ($has_cmdi === false){
        form_set_error('error', "No metadata file has been specified");
        return $form;
    }

    // Quick and dirty Check cmdi valid xml
    if ($valid_xml === false){
        form_set_error('validate_bundle', t("The CMDI metadata file is not a valid xml file"));
        return $form;
    }

    /* In case of wrong naming
    if ($good_name === false){
        form_set_error('validate_bundle', t("Metadata file has wrong file name (record.cmdi expected)"));
        return $form;
    }
    */

    // Check existence external location
    if ($file_exists === false){

        form_set_error('validate_bundle',t('Location does not exist (:path) ' , array(':path' => $path)));
        return $form;

    }

    // Check user permissions
    if ($user_has_permissions === false){
        form_set_error('validate_bundle', t('You do not have the permission to perform this action. Please contact the archive manager.'));
        return $form;
    }

    // Validate path, number of files in bundle and whether there are deleted files

    if (!file_exists($path)){
        form_set_error('validate_bundle', t('The selected folder does not exist or is not accessible.'));
        return $form;
    }

    $nFiles = count(array_diff(scandir(drupal_realpath($path)), ['.','..']));
    $deletedFiles = isset($wrapper->flat_deleted_resources) ? $wrapper->flat_deleted_resources->value() : NULL;

    $max = variable_get('flat_deposit_ingest_service')['max_ingest_files'];

    if ($nFiles == 0){
        if (isset($deletedFiles)) {
            if ($deletedFiles == '') {
                form_set_error('validate_bundle', t('No new files added and no exising files selected for removal.'));
            }
        }
        else {
            form_set_error('validate_bundle', t('The selected folder does not contain any (accessible) files.'));
        }
        return $form;
    }

    if ($nFiles > $max){
        form_set_error('validate_bundle', t('The selected folder contains too many files, the maximum is !limit', array('!limit'=> $max)));
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

            global $user;
            $form_state['redirect'] = 'dashboard';
            drupal_set_message("Bundle is being $processed");

            break;


        case 'Edit bundle properties':
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
