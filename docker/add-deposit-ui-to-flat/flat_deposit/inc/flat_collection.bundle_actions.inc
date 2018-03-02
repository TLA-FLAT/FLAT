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
    $query = new EntityFieldQuery();
    $query->entityCondition('entity_type', 'node')
        ->entityCondition('bundle', 'flat_bundle')
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

        if (!empty($error)) {

            $form_state['values']['reopen_error'][] = $nid;

        } else {

            $form_state['values']['reopen_bundles'][] = $nid;
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
        $good_name = has_correct_filename($nid);
        $file_exists = bundle_file_exists($nid);
        $user_has_permissions = user_has_permissions($nid);
        $tests = array($complete, $parent_is_known, $has_cmdi, $valid_xml, $good_name, $file_exists, $user_has_permissions);

        if (!in_array(false, $tests)) {

            $form_state['values']['validate_bundles'][] = $nid;

        } else {

            $form_state['values']['reject_bundles'][] = $nid;
        }

    }
}


function flat_collection_bundle_action_form_archive_validate($form, &$form_state){


    $nids = get_nids($form_state);
    foreach ($nids as $index => $nid) {
        $check = check_completeness('archive', $nid, $index, $form_state);


        if ($check) {

            $form_state['values']['archive_bundles'][] = $nid;

        } else {

            $form_state['values']['archive_reject'][] = $nid;
        }

    }
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
                drupal_set_message('Reopen submission succeeded for bundle(s) ' . implode(' and ', $form_state['values']['reopen_bundles']));
                $form_state['redirect'] = 'dashboard';
            }

            if (!empty($form_state['values']['reopen_error'])){

                    drupal_set_message(t('Unable to reopen bundle(s) !what', array('!what' => implode(' AND ', $form_state['values']['reopen_error']))));

            }
            break;

        case 'Validate bundle(s)':

            $action = 'Validate bundle';

            if (!empty($form_state['values']['validate_bundles'])) {

                foreach ($form_state['values']['validate_bundles'] as $bundle){


                    send_request($bundle, $action, false);
                }

                drupal_set_message('Validation submission succeeded for bundle(s) ' . implode(' and ', $form_state['values']['validate_bundles']));
                $form_state['redirect'] = 'dashboard';
            }

            if (!empty($form_state['values']['reject_bundles'])) {

                drupal_set_message('Unable to ' . $action . ' ' . implode(' and ', $form_state['values']['reject_bundles']));
            }
            break;

        case 'Archive bundle(s)':

            $action = 'Archive bundle';

            if (!empty($form_state['values']['archive_bundles'])) {

                foreach ($form_state['values']['archive_bundles'] as $bundle){

                    send_request($bundle, $action, false);

                }

                drupal_set_message('Submission succeeded ' . implode(' and ', $form_state['values']['archive_bundles']));
                $form_state['redirect'] = 'dashboard';

            }

            if (!empty($form_state['values']['archive_reject'])) {

                drupal_set_message('Archive submission failed for bundle(s) ' . implode(' and ', $form_state['values']['archive_reject']));

            }

            break;

    }
}
