<?php


function flat_bundle_action_form($form, &$form_state, $vars)
{


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
        '#validate' => array('flat_bundle_action_form_archive_validate'),
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

    global $user;
    if (user_role_load_by_name('developer')){
        $form['options']['serial'] = array(
            '#type' => 'checkbox',
            '#title' => t('Serial processing (debug mode)'),
            '#prefix' => '</p>',
            '#default_value' => 1,

        );
    }

    $node = menu_get_object();
    $wrapper = entity_metadata_wrapper('node', $node);

    $form['values']['node'] = array(
        '#type' => 'value',
        '#value' => $node
    );

    $form['values']['origin_url'] = array(
        '#type' => 'value',
        '#value' => 'node/' . $vars['nid']);




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


    return $form;

}

function flat_bundle_action_form_describe_validate($form, &$form_state){

    $node = $form_state['values']['node'];
    $wrapper = entity_metadata_wrapper('node', $node);

    $parent_nid = $wrapper->flat_parent_nid->value();
    if ($parent_nid === '0'){
        form_set_error('describe_bundle', "To generate or edit a cmdi file, you need to select a collection first");
        drupal_goto($form_state['values']['origin_url']);
        return;

    }

    return $form;

}

function flat_bundle_action_form_reopen_validate($form, &$form_state){

    module_load_include('inc', 'flat_deposit', 'inc/class.FlatBundle');

    $node = $form_state['values']['node'];
    $wrapper = entity_metadata_wrapper('node', $node);



    $move = FlatBundle::moveBundleData($node, 'data', 'unfreeze');
    if (!$move){
        form_set_error('reopen', 'Unable to move bundle data to unfreeze');
        drupal_goto($form_state['values']['origin_url']);
    }

    if (isset($wrapper->flat_cmdi_file)){
        $move = FlatBundle::moveBundleData($node, 'metadata', 'unfreeze');
        if (!$move){
            form_set_error('reopen', 'Unable to move bundle metadata to unfreeze');
            drupal_goto($form_state['values']['origin_url']);
        }


    };

    return $form;

}





function flat_bundle_action_form_validate_validate($form, &$form_state){


    $node = $form_state['values']['node'];
    $wrapper = entity_metadata_wrapper('node',$node);

    global $user;
    $user_id = $user->uid;

    // validate that a collection has been selected
    if ((integer)$wrapper->flat_parent_nid->value() === 0){
        form_set_error('error', "Bundle has not been assigned to a collection");
        drupal_goto($form_state['values']['origin_url']);
        return;

    }

    // In case no cmdi file exists
    if (!$wrapper->flat_cmdi_file->value()) {
        form_set_error('error', "No metadata file has been specified");
        drupal_goto($form_state['values']['origin_url']);
        return;
    }



 // In case of wrong naming
    $exploded_uri = explode('/', $wrapper->flat_cmdi_file->value()['uri']);
    $file_name = end($exploded_uri);

    if ($file_name !== 'record.cmdi') {
        form_set_error('validate_bundle', t("Metadata file has wrong file name"));
        drupal_goto($form_state['values']['origin_url']);
        return;
    }

    /*
    // Check if target freeze directory exists
    if (!file_exists(drupal_realpath('freeze://' . $user->name .'/' . $wrapper->flat_parent_title->value() . '/' . $node->title . '/metadata'))){

        form_set_error('validate_bundle', "Target directory for metafile does not exist");
        drupal_goto($form_state['values']['origin_url']);
        return;
    }
*/

    // Quick and dirty Check cmdi valid xml
    $file_field = $wrapper->flat_cmdi_file->value();
    $attached_file = file_load($file_field['fid']);
    $xml = simplexml_load_file(drupal_realpath($attached_file->uri));
    if (!$xml) {

        form_set_error('validate_bundle', t("Attached file is no valid xml file"));
        drupal_goto($form_state['values']['origin_url']);
        return;
    }

    // Check existence external location

    $external = $wrapper->flat_external->value();
    $location = $wrapper->flat_location->value();
    if ($external ){
        $config = variable_get('flat_deposit_paths');
        $root = $config['alternate_dir'];
        $sub = $config['alternate_subdir'];

        $full_path = $root . str_replace('@' ,'_at_' , $user->name) . $sub . "/$location";

        if (!file_exists($full_path)){

            form_set_error('validate_bundle',t('External location does not exist (:path) ' , array(':path' => $full_path)));
            drupal_goto($form_state['values']['origin_url']);
            return;
        }
    }

    // Check user permissions
    $bundle_id = $node->uid;

    if($user_id === $bundle_id OR user_access('validate bundles')) {

    } else {

        form_set_error('validate_bundle', t('User has not enough privileges to perform requested action'));
        drupal_goto($form_state['values']['origin_url']);
        return;
    }





}



function flat_bundle_action_form_archive_validate($form, &$form_state){

    $node = $form_state['values']['node'];

    // Check user permissions
    global $user;
    $user_id = $user->uid;

    $bundle_id = $node->uid;

    if (($user_id === $bundle_id AND user_access('certified user')) OR user_access('ingest bundles')) {

        return $form;

    } else {

        form_set_error('archive_bundle', t('User has not enough privileges to perform requested action'));
        drupal_goto($form_state['values']['origin_url']);
        return $form;
    }


}


function flat_bundle_action_form_submit($form, &$form_state){

    $node = $form_state['values']['node'];
    $action = $form_state['clicked_button']['#value'];

    switch ($action){
        case 'Bundle image':
        case 'Upload data':
            global $user;
            $form_state['redirect'] = array('user/' . $user->uid  . '/imce');
            break;


        case 'Fill in metadata for bundle':
            $url = array('node/' . $node->nid  . '/cmdi');
            $form_state['redirect'] = $url;
            break;


        case 'Validate bundle':
        case 'Archive bundle':

            global $user;

            // background service http request settings
            $host = variable_get('flat_deposit_ingest_service')['host_name'];
            $scheme = variable_get('flat_deposit_ingest_service')['host_scheme'];
            $base = $GLOBALS['base_path'];

            $sid = $user->sid;
            $name = array_search($sid,$_COOKIE);

            // create array containing data needed by the ingest service
            $data_to_post = array();
            $data_to_post['drupal_path'] = DRUPAL_ROOT;
            $data_to_post['nid'] = $node->nid;
            $data_to_post['sid'] = $sid;
        $data_to_post['test'] = $action;



        $options = array(
                'method' => 'POST',
                'data' => $data_to_post,
                'blocking' => FALSE,
                'headers' => array(
                    // Set the Host header to self.
                    'Host' => $host,
                ),
            );


            $url = $scheme . '://' . $host . $base . drupal_get_path('module', 'flat_deposit') . '/Helpers/IngestService/IngestService.php'; #?user='.$user->name. '&pass='.$user->pass


        // Debug mode
        $wrapper = entity_metadata_wrapper('node', $node);
        $status = $wrapper->flat_bundle_status->value();


        if (isset($form_state['values']['serial'])) {

            if ($form_state['values']['serial']) {


                $ch = curl_init();
                curl_setopt_array($ch, array(

                    CURLOPT_URL => $url,

                    CURLOPT_COOKIE => "XDEBUG_SESSION=PHPSTORM",
                    CURLOPT_POST => sizeof($data_to_post),
                    CURLOPT_POSTFIELDS => $data_to_post));

                $result = curl_exec($ch);

                if ($errno = curl_errno($ch)) {
                    $error_message = curl_strerror($errno);
                    drupal_set_message("cURL error ({$errno}):\n {$error_message}",'error');
                }
                curl_close($ch);

            } else {

                // normal background mode
                $request = httprl_request($url, $options);

                // Execute request.
                $response = httprl_send_request();
            }
        }


        $processed = ($status == 'valid') ? 'archived' : 'validated';

        $form_state['redirect'] = 'workspace';
        drupal_set_message("Bundle is being $processed");


        break;



        case 'Edit bundle':
            $url = array('node/' . $node->nid  . '/edit');
            $form_state['redirect'] = $url;
            break;


        case 'Delete bundle':
            $url = array('node/' . $node->nid  . '/delete');
            $form_state['redirect'] = $url;
            break;

        case 'Re-open bundle':


            $node = $form_state['values']['node'];
            $wrapper = entity_metadata_wrapper('node', $node);
            $wrapper->flat_bundle_status->set('open');
            $wrapper->save();
            drupal_set_message('Bundle is open and can be modified');
            $form_state['redirect'] = $form_state['values']['origin_url'];

            break;


    }


}