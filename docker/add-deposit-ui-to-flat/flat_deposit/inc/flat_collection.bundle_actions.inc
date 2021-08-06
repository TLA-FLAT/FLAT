<?php
function flat_collection_bundle_action_form($form, &$form_state, $var)
{
    form_load_include($form_state, 'inc', 'flat_deposit', 'inc/flat_bundle_action_helpers');

    $form['table_container'] = array
    (
        '#theme' => 'flat_bundle_action_table',
        // Need to set #tree to be able to differentiate
        // between the various delete buttons upon
        // submission.
        '#tree' => TRUE,
    );

// query already existing links to collections
    global $user;
    $parent_nid = $var->nid;
    $query = new EntityFieldQuery();
    $query->entityCondition('entity_type', 'node')
        ->entityCondition('bundle', 'flat_bundle')
        ->fieldCondition('flat_parent_nid', 'value', "$parent_nid", '=')
        ->propertyCondition('uid', $user->uid);

    $user_bundle_nodes = $query->execute();
    $existing = [];
    $count = 0;
    $data = [];


    if (!empty($user_bundle_nodes['node'])) {
        foreach ($user_bundle_nodes['node'] as $entity) {
            $nid = $entity->nid;
            $bundle =node_load($nid);
            $bundle_wrapper = entity_metadata_wrapper('node', $bundle);
            $metadata = !is_null($bundle_wrapper->flat_cmdi_file->value()) ? 'available' : 'not available';
            $status = $bundle_wrapper->flat_bundle_status->value();

            $source = $bundle_wrapper->flat_source->value();

            $disabled = ($metadata == 'available' AND ($status == 'open' OR $status == 'failed' OR $status == 'valid')) ? FALSE : TRUE;

            $form['table_container'][$count]['title'] = array(
                '#type' => 'link',
                '#title' => $bundle->title,
                '#href' => 'node/' . $bundle->nid,
            );

            $form['table_container'][$count]['location'] = array(
                '#markup' => t('!source', array('!source' => $source))
            );

            $form['table_container'][$count]['status'] = array(
                '#markup' => t('!status', array('!status' => $status))
            );
            $form['table_container'][$count]['metadata'] = array(
                '#markup' => t('!metadata', array('!metadata' => $metadata))
            );
            $form['table_container'][$count]['checked'] = array(
                '#type' => 'checkbox',
                '#disabled' => $disabled,
            );

            $data[$count] = array(
                'nid' => $nid,
                'title' => $bundle->title,
                'metadata' => $metadata,
                'status' => $status,
            );

            $count++;
        }
        $form['data'] = array(
            '#type' => 'value',
            '#value' => $data,
        );

    }
    $node = menu_get_object();

    $form['values']['node'] = array(
        '#type' => 'value',
        '#value' => $node
    );


    $form['actions']['container'] = array(
        '#type' => 'container',
        '#attributes' => array('class' => array('container-inline')),
    );

    $form['actions']['validate_bundle'] = array(
        '#type' => 'submit',
        '#value' => t('Validate bundle(s)'),
        '#validate' => array('flat_collection_bundle_action_form_validate_validate'),
        '#description' => t('Validate selected bundles. Successfully validated bundles cannot be altered, unless they are re-opened'),
    );

    $form['actions']['reopen_bundle'] = array(
        '#type' => 'submit',
        '#value' => t('Re-open bundle(s)'),
        '#validate' => array('flat_collection_bundle_action_form_reopen_validate'),
        '#description' => t('Re-open valid bundles to allow modifications'),
    );

    $form['actions']['archive_bundle'] = array(
        '#type' => 'submit',
        '#value' => t('Archive bundle(s)'),
        '#validate' => array('flat_collection_bundle_action_form_archive_validate'),
        '#description' => t('Submit valid bundles to be archived'),
    );

    return $form;

}

function flat_collection_bundle_action_form_reopen_validate($form, &$form_state){

    $nids = get_nids($form_state);
    foreach ($nids as $index => $nid) {

        $error = [];
        $complete = check_completeness('reopen', $nid, $index, $form_state);

        if($complete) {

            $error = move_node_data($nid);

        }

        foreach ($form_state['values']['data'] as $bundle) {
                if ($bundle['nid'] == $nid) {
                        $title = $bundle['title'];
                        }
                }

        if (!empty($error)) {

            $form_state['values']['reopen_error'][] = $nid;
            $form_state['values']['reopen_error_titles'][] = $title;

        } else {

            $form_state['values']['reopen_bundles'][] = $nid;
            $form_state['values']['reopen_bundle_titles'][] = $title;

        }

    }

}


function flat_collection_bundle_action_form_validate_validate($form, &$form_state)
{

    $nids = get_nids($form_state);
    foreach ($nids as $index => $nid) {
        $complete = check_completeness('validate', $nid, $index, $form_state);
        $parent_is_known = parent_is_known($nid);
        $has_cmdi = has_cmdi($nid);
        $valid_xml = is_valid_xml($nid, $has_cmdi);
        //$good_name = has_correct_filename($nid); filename here not relevant, only in SIP that is offered to doorkeeper
        $file_exists = bundle_file_exists($nid);
        $user_has_permissions = user_has_permissions($nid);
        $max_files_not_exceeded = !bundle_max_files_exceeded($nid);
        $max_file_size_not_exceeded = !bundle_max_file_size_exceeded($nid);
        $valid_file_names = !bundle_invalid_file_names($nid);
        $valid_file_extensions = !bundle_invalid_file_extensions($nid);
        $has_new_or_deleted_files = (bundle_new_files($nid) OR bundle_deleted_files($nid));
        //$tests = array($complete, $parent_is_known, $has_cmdi, $valid_xml, $good_name, $file_exists, $user_has_permissions);
        $tests = array('complete' => $complete, 'parent_is_known' => $parent_is_known, 'has_cmdi' => $has_cmdi, 'valid_xml' => $valid_xml, 'file_exists' => $file_exists, 'user_has_permissions' => $user_has_permissions, 'max_files_not_exceeded' => $max_files_not_exceeded,
        'max_file_size_not_exceeded' => $max_file_size_not_exceeded, 'valid_file_names' => $valid_file_names, 'valid_file_extensions' => $valid_file_extensions, 'has_new_or_deleted_files' => $has_new_or_deleted_files);
        foreach ($form_state['values']['data'] as $bundle) {
                if ($bundle['nid'] == $nid) {
                        $title = $bundle['title'];
                        }
                }

        $test_values = array();
        foreach ($tests as $test => $value) {
            $test_values[] = $value;
        }
        if (!in_array(false, $test_values)) {

            $form_state['values']['validate_bundles'][] = $nid;
            $form_state['values']['validate_bundle_titles'][] = $title;

        } else {

            $form_state['values']['reject_bundles'][] = $nid;
            $form_state['values']['reject_bundle_titles'][] = $title;
            foreach ($tests as $test => $value) {
                if ($value === false) {
                    $form_state['values']['reject_bundle_failed_tests'][$nid][] = $test;
                }

            }

        }

    }

}

function flat_collection_bundle_action_form_archive_validate($form, &$form_state){


    $nids = get_nids($form_state);
    foreach ($nids as $index => $nid) {
        $check = check_completeness('archive', $nid, $index, $form_state);
	foreach ($form_state['values']['data'] as $bundle) {
		if ($bundle['nid'] == $nid) {
			$title = $bundle['title'];
			}
		}

        if ($check) {

            $form_state['values']['archive_bundles'][] = $nid;
	    $form_state['values']['archive_bundle_titles'][] = $title;

        } else {

            $form_state['values']['archive_reject'][] = $nid;
	    $form_state['values']['archive_reject_titles'][] = $title;

        }

    }
}

function implode_comma_and($array) {
    //helper function to implode an array with commas except for the final value, there it will be 'and'.
    $last  = array_slice($array, -1);
    $first = implode(', ', array_slice($array, 0, -1));
    $both  = array_filter(array_merge(array($first), $last), 'strlen');
    return implode(' and ', $both);
}

function flat_collection_bundle_action_form_submit($form, &$form_state)
{

    $action = $form_state['clicked_button']['#value'];

    switch ($action) {
        case 'Re-open bundle(s)':

            $action = 'Re-open bundle';

            if (!empty($form_state['values']['reopen_bundles'])) {

                foreach ($form_state['values']['reopen_bundles'] as $bundle){
                    $node = node_load($bundle);
                    $wrapper=entity_metadata_wrapper('node',$node);
                    $wrapper->flat_bundle_status->set('open');
                    $wrapper->save();

                }
                drupal_set_message('Re-open succeeded for bundle(s): ' . implode_comma_and($form_state['values']['reopen_bundle_titles']));
                //$form_state['redirect'] = 'dashboard';
            }

            if (!empty($form_state['values']['reopen_error'])){

                drupal_set_message('Unable to re-open bundle(s): ' .  implode_comma_and($form_state['values']['reopen_error_titles']) . '. Only valid or failed bundles can be re-opened.', 'error');

            }
            break;

        case 'Validate bundle(s)':

            $action = 'Validate bundle';

            if (!empty($form_state['values']['validate_bundles'])) {

                foreach ($form_state['values']['validate_bundles'] as $bundle){


                    send_request($bundle, $action, false);
                }

                drupal_set_message('Validation submission succeeded for bundle(s) ' . implode_comma_and($form_state['values']['validate_bundle_titles']));
                //$form_state['redirect'] = 'dashboard';
            }

            if (!empty($form_state['values']['reject_bundles'])) {

                drupal_set_message('Validation failed for the following bundle(s): ' . implode_comma_and($form_state['values']['reject_bundle_titles']) . '.', 'error');

                $max_files = variable_get('flat_deposit_ingest_service')['max_ingest_files'];
                $max_file_size = variable_get('flat_deposit_ingest_service')['max_file_size'];

                foreach ($form_state['values']['reject_bundles'] as $index => $rejected_bundle) {

                    $errors = [];

                    foreach ($form_state['values']['reject_bundle_failed_tests'][$rejected_bundle] as $failed_test) {

                        $rejected_title =  $form_state['values']['reject_bundle_titles'][$index];

                        if ($failed_test == 'complete') {
                            $errors[] = t('Bundle ' . $rejected_title . ' is not complete.');
                        }
                        if ($failed_test == 'parent_is_known') {
                            $errors[] = t('The parent of bundle ' . $rejected_title . ' is not known.');
                        }
                        if ($failed_test == 'has_cmdi') {
                            $errors[] = t('Bundle ' . $rejected_title . ' does not have a CMDI metadata file.');
                        }
                        if ($failed_test == 'file_exists') {
                            $errors[] = t('The specified file location for Bundle ' . $rejected_title . ' does not exist.');
                        }
                        if ($failed_test == 'user_has_permissions') {
                            $errors[] = t('You do not have enough permissions for Bundle ' . $rejected_title . '.');
                        }
                        if ($failed_test == 'max_files_not_exceeded') {
                            $errors[] = t('Bundle ' . $rejected_title . ' contains more than the maximum of ' . $max_files . ' files.');
                        }
                        if ($failed_test == 'max_file_size_not_exceeded') {
                            $errors[] = t('Bundle ' . $rejected_title . ' contains files that are larger than the maximum allowed file size of ' . $max_file_size . 'GB.');
                        }
                        if ($failed_test == 'valid_file_names') {
                            $errors[] = t('Bundle ' . $rejected_title . ' contains files that have disallowed characters in their name.');
                        }
                        if ($failed_test == 'valid_file_extensions') {
                            $errors[] = t('Bundle ' . $rejected_title . ' contains files that have a disallowed file extension. See the deposit manual for allowed file types and extensions.');
                        }
                        if ($failed_test == 'has_new_or_deleted_files') {
                            $errors[] = t('Bundle ' . $rejected_title . ' has no new or deleted files.');
                        }
                    }

                    if (!empty($errors)) {
                        foreach ($errors as $error) {
                            drupal_set_message($error, 'error');
                        }
                    }

                }

            }
            break;

        case 'Archive bundle(s)':

            $action = 'Archive bundle';

            if (!empty($form_state['values']['archive_bundles'])) {

                foreach ($form_state['values']['archive_bundles'] as $bundle){

                    send_request($bundle, $action, false);

                }

                drupal_set_message('Bundle(s) submitted for archiving:  ' . implode_comma_and($form_state['values']['archive_bundle_titles']));
                //$form_state['redirect'] = 'dashboard';

            }

            if (!empty($form_state['values']['archive_reject'])) {

                drupal_set_message('Failed submitting bundle(s) for archiving: ' . implode_comma_and($form_state['values']['archive_reject_titles']) . '. Only valid bundles can be archived.', 'error');

            }

            break;

    }
}
