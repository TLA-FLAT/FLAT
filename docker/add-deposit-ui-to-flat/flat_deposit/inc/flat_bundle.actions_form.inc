<?php
/**
 * Created by PhpStorm.
 * User: danrhe
 * Date: 15/11/16
 * Time: 12:18
 */


function flat_bundle_action_form($form, &$form_state)
{

    $form['actions']['bundle_image'] = array(
        '#type' => 'image_button',
        '#value' => t('Bundle image'),
        '#disabled' => TRUE,
        '#prefix' => '</p>'

    );

    $form['actions']['container'] = array(
        '#type' => 'container',
        '#attributes' => array('class' => array('container-inline')),
    );


    $form['actions']['describe_bundle'] = array(
        '#type' => 'submit',
        '#value' => t('Create info file for bundle'),
        #'#src' => drupal_get_path('module','flat_deposit').'/Images/arrow_right.png',
        '#validate' => array('flat_bundle_action_form_describe_validate'),
        '#description' => t('Bundles are obliged to be accompanied by a info file'),
        '#disabled' => TRUE,
    );

    $form['actions']['reopen_bundle'] = array(
        '#type' => 'submit',
        '#value' => t('Re-open bundle'),
        #'#src' => drupal_get_path('module','flat_deposit').'/Images/arrow_right.png',
        '#description' => t('Click to allow modifications of your bundle'),
        '#disabled' => TRUE,
    );

    $form['actions']['validate_bundle'] = array(
        '#type' => 'submit',
        '#value' => t('Validate bundle'),
        '#validate' => array('flat_bundle_action_form_validate_validate'),
        #'#src' => drupal_get_path('module','flat_deposit').'/Images/arrow_right.png',
        '#description' => t('Validated bundles will be transferred to different location and can not be altered'),
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
        '#prefix' => '</p></p>',
    );

    $form['actions']['delete_bundle'] = array(
        '#type' => 'submit',
        '#value' => t('Delete bundle'),
        #    '#src' => drupal_get_path('module','flat_deposit').'/Images/arrow_right.png',
        '#suffix' => '</p>',
    );

    global $user;
    if ($user->name == 'admin') {
        $form['options']['serial'] = array(
            '#type' => 'checkbox',
            '#title' => t('Serial processing (debug mode)'),
            '#prefix' => '</p>',
        );
    }

    $form['actions']['delete_bundle'] = array(
        '#type' => 'submit',
        '#value' => t('Delete bundle'),
        #    '#src' => drupal_get_path('module','flat_deposit').'/Images/arrow_right.png',
        '#suffix' => '</p>',
    );

    $node = menu_get_object();
    $wrapper = entity_metadata_wrapper('node', $node);

    $form['values']['node'] = array(
        '#type' => 'value',
        '#value' => $node
    );

// Specify which handlers to become active and which bundle_image to display

    $status = $wrapper->flat_bundle_status->value();

    switch ($status) {
        case 'open': {
            $form['actions']['bundle_image']['#disabled'] = FALSE;
            $form['actions']['validate_bundle']['#disabled'] = FALSE;
            $name = 'open_bundle';
            break;
        }

        case 'validating': {
            $name = 'process_bundle';
            break;
        }

        case 'failed': {
            $form['actions']['bundle_image']['#disabled'] = FALSE;
            $form['actions']['validate_bundle']['#disabled'] = FALSE;
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


    // Activate metadata creation button if field value has not been set
    if (!$wrapper->flat_cmdi_file->value()) {

        $form['actions']['describe_bundle']['#disabled'] = FALSE;
    }

    return $form;

}

function flat_bundle_action_form_describe_validate($form, &$form_state){


        form_set_error('describe_bundle', "This feature will be available in a future release");

    }
    // In case no cmdi file exists

function flat_bundle_action_form_validate_validate($form, &$form_state){

    $node = $form_state['values']['node'];
    $wrapper = entity_metadata_wrapper('node',$node);


    // In case no cmdi file exists
    if (!$wrapper->flat_cmdi_file->value()) {
        form_set_error('validate_bundle', t("No metadata file has been specified"));

        return $form;
    }

    // In case of wrong naming
    $exploded_uri = explode('/', $wrapper->flat_cmdi_file->value()['uri']);
    $file_name = end($exploded_uri);

    if ($file_name !== 'record.cmdi') {
        form_set_error('validate_bundle', t("Metadata file has wrong file name"));

    }

    // Check if target freeze directory exists
    global $user;
    if (!file_exists(drupal_realpath('freeze://' . $user->name .'/' . $wrapper->flat_parent_title->value() . '/' . $node->title . '/metadata'))){

        form_set_error('validate_bundle', "Target directory for metafile does not exist");
    }


    // Quick and dirty Check cmdi valid xml
    $file_field = $wrapper->flat_cmdi_file->value();
    $attached_file = file_load($file_field['fid']);
    $xml = simplexml_load_file(drupal_realpath($attached_file->uri));
    if (!$xml) {

        form_set_error('validate_bundle', t("Attached file is no valid xml file"));
        return $form;
    }

    // Check existence external location

    $external = $wrapper->flat_external->value();
    $location = $wrapper->flat_location->value();
    if ($external ){
        $config = variable_get('flat_deposit_paths');
        $root = $config['alternate_dir'];
        $sub = $config['alternate_subdir'];

        global $user;
        $full_path = $root . $user->name . $sub . "/$location";

        if (!file_exists($full_path)){

            form_set_error('validate_bundle',t('External location does not exist (:path) ' , array(':path' => $full_path)));

        }
    }

    // Check user permissions
    global $user;
    $user_id = $user->uid;

    $bundle_id = $node->uid;

    if($user_id === $bundle_id OR user_access('validate bundles')) {


    } else {

        form_set_error('validate_bundle', t('User has not enough privileges to perform requested action'));
    }

    return $form;
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

    }
    return $form;

}


function flat_bundle_action_form_submit($form, &$form_state){

    $node = $form_state['values']['node'];
    $action = $form_state['clicked_button']['#value'];

    switch ($action){
        case 'Bundle image':
            global $user;
            $form_state['redirect'] = array('user/' . $user->uid  . '/imce');
            break;


        case 'Create info file for bundle':
            $url = array('node/' . $node->nid  . '/cmdi/select');
            $form_state['redirect'] = $url;
            break;


        case 'Validate bundle':
        case 'Archive bundle':

            global $user;

            // background service http request settings
            $host = variable_get('flat_deposit_ingest_service')['host_name'];
            $scheme = $GLOBALS['_SERVER']['REQUEST_SCHEME'];
            $base = $GLOBALS['base_path'];

            $sid = $user->sid;
            $name = array_search($sid,$_COOKIE);

            // create array containing data needed by the ingest service
            $data_to_post = array();
            $data_to_post['drupal_path'] = DRUPAL_ROOT;
            $data_to_post['nid'] = $node->nid;
            $data_to_post['sid'] = $sid;



        $options = array(
                'method' => 'POST',
                'data' => $data_to_post,
                'blocking' => FALSE,
                'headers' => array(
                    // Set the Host header to self.
                    'Host' => $host,
                ),
            );


            $url = $scheme . '://' . $host . $base . drupal_get_path('module', 'flat_deposit') . '/Helpers/Ingest_service.php'; #?user='.$user->name. '&pass='.$user->pass


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
                    echo "cURL error ({$errno}):\n {$error_message}";
                }
                curl_close($ch);

            } else {

                // normal background mode
                $request = httprl_request($url, $options);

                // Execute request.
                $response = httprl_send_request();
            }
        }


        $form_state['redirect'] = '<front>';
        drupal_set_message('Bundle is being processed');


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
            module_load_include('inc','flat_deposit', '/Helpers/Ingestor');

            $ingest = new Ingestor($node->nid);

            $ingest->deleteTempBag();

            $ingest->deleteZipfile();

            $ingest->moveData('unfreeze');

            $ingest->updateNodeStatus('open');

            drupal_set_message('Bundle is open and can be modified');

            break;


    }


}