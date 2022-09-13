<?php

function flat_bundle_manage_resources_form($form, &$form_state) {

    // hide blocks for function calls that are not related to nodes
    $node = $form_state['build_info']['args'][0]['node'];

    if (!$node){
        return $form;
    }

    $node_wrapper = entity_metadata_wrapper('node', $node);

    $form['flat_bundle_manage_resources'] = [

        '#theme' => 'flat_bundle_manage_resources',
        // Need to set #tree to be able to differentiate
        // between the various delete buttons upon
        // submission.
        '#tree' => TRUE,
    ];

    $location = $node_wrapper->flat_location->value();

    $files = [];
    if (file_exists($location)) {

        // We ignore hidden files (starting with a dot). These will not be added
        // to the bundle later on either.
        $files = array_diff(preg_grep('/^([^.])/', scandir($location)), ['..', '.']);
    }

    // normalizing currently saved metadata, null, empty str will be marked as empty array
    $marked = $node_wrapper->flat_encrypted_resources->value();
    $marked = empty($marked) ? [] : explode(',', $marked);

    foreach ($files as $file) {

        // using md5 on the filename to differentiate the resources
        $id = md5($file);

        $form['flat_bundle_manage_resources'][$id]['encrypt'] = [

            '#type' => 'checkbox',
            '#default_value' => in_array($id, $marked),
        ];

        $form['flat_bundle_manage_resources'][$id]['filename'] = [
            '#markup' => t('!label', ['!label' => $file]),
        ];
    }

    $form['buttons']['save'] = [

        '#type' => 'submit',
        '#value' => t('Save'),
        '#submit' => ['flat_bundle_manage_resources_form_submit'],
        '#attributes' => [
            'class' => ['btn-success']
        ]
    ];

    return $form;
}

function flat_bundle_manage_resources_form_submit($form, &$form_state) {

    $node = menu_get_object();
    $node_wrapper = entity_metadata_wrapper('node', $node);
    $marked = [];

    foreach ($form_state['values']['flat_bundle_manage_resources'] as $filenameMd5 => $fields) {

        if (isset($fields['encrypt']) && $fields['encrypt'] === 1) {

            // file has been marked as encrypted
            $marked[] = $filenameMd5;
        }
    }

    $node_wrapper->flat_encrypted_resources->set(implode(',', $marked));

    $flat_auth_endpoint = url('flat_auth', ['absolute' => true]);
    // var_dump($flat_auth_endpoint);exit;

    $node_wrapper->flat_encrypted_auth_endpoint->set($flat_auth_endpoint);
    $node_wrapper->save();
    // var_dump($node_wrapper->flat_encrypted_auth_endpoint->value());exit;
}
