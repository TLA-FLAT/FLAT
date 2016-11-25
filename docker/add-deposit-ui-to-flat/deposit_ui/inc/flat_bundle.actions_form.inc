<?php
/**
 * Created by PhpStorm.
 * User: danrhe
 * Date: 15/11/16
 * Time: 12:18
 */


function flat_bundle_action_form($form, &$form_state, $variables){


    $form['actions']['bundle_image'] = array(
        '#type' => 'image_button',
        '#value' => t('Bundle image'),
        '#disabled'=> TRUE,
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
        '#description' => t('Bundles are obliged to be accompanied by a info file'),
        '#disabled'=> TRUE,
    );

    $form['actions']['validate_bundle'] = array(
        '#type' => 'submit',
        '#value' => t('Validate bundle'),
        #'#src' => drupal_get_path('module','flat_deposit').'/Images/arrow_right.png',
        '#description' => t('Validated bundles will be transferred to different location and can not be altered'),
        '#disabled'=> TRUE,
    );

    $form['actions']['archive_bundle'] = array(
        '#type' => 'submit',
        '#value' => t('Archive bundle'),
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

    $node = menu_get_object();

    $form['values']['node'] = array(
        '#type' => 'value',
        '#value' => $node
    );

// Specify which handlers become active
    $status = $variables['status'];

    switch ($status){
        case 'open':{
            $form['actions']['bundle_image']['#disabled'] = FALSE;
            $form['actions']['validate_bundle']['#disabled'] = FALSE;
            $form['actions']['describe_bundle']['#disabled'] = FALSE;
            $name = 'open_bundle';
            break;
        }

        case 'validating':{
            $name = 'closed_bundle';
            break;
        }

        case 'failed':{
            $form['actions']['bundle_image']['#disabled'] = FALSE;
            $form['actions']['validate_bundle']['#disabled'] = FALSE;
            $name = 'closed_bundle_failed';
            break;
        }

        case 'valid':{
            $form['actions']['archive_bundle']['#disabled'] = FALSE;

            $name = 'closed_bundle';
            break;
        }
        case 'processing':{
            $name = 'process_bundle';

            break;
        }
    }

    $form['actions']['bundle_image']['#src'] = drupal_get_path('module','flat_deposit').'/Images/' . $name . '.jpg';

    return $form;

}



function flat_bundle_action_form_validate($form, &$form_state){

    $node = $form_state['values']['node'];
    $action = $form_state['clicked_button']['#value'];
    $wrapper = entity_metadata_wrapper('node',$node);

    switch ($action){
        case 'Validate bundle':{

            // check existence info file
            module_load_include('php','flat_deposit','Helpers/CMDI_Handler');
            $cmd = new CMDI_Handler($node);

            if(!$cmd->projectCmdiFileExists()) {
                form_set_error('validate_bundle', t("Expecting record.cmdi file in metadata folder"));
            }

            // check profile name is set correctly
            $profile = $wrapper->upload_cmdi->value();
            if (!$profile) {
                form_set_error('validate_bundle', t("Profile has not been set"));
            }

            // reset cmdi value in case
            if ($profile AND !$cmd->projectCmdiFileExists())  {
                $wrapper->upload_cmdi->set(NULL);
            }


            break;
        }
    }
}


function flat_bundle_action_form_submit($form, &$form_state){

    $node = $form_state['values']['node'];
    $action = $form_state['clicked_button']['#value'];

    switch ($action){
        case 'Bundle image':{
            global $user;
            $form_state['redirect'] = array('user/' . $user->uid  . '/imce');
            break;
        }

        case 'Create info file for bundle':{
            $url = array('node/' . $node->nid  . '/cmdi/select');
            $form_state['redirect'] = $url;
            break;
        }

        case 'Validate bundle':
        case 'Archive bundle':{

            // create array containing data needed by the ingest service
            $data_to_post = array();
            $data_to_post['drupal_path'] = DRUPAL_ROOT;
            $data_to_post['nid'] = $node->nid;

            // background service http request settings
            $options = array(
                'method' => 'POST',
                'data' => $data_to_post,
                'blocking' => FALSE,
                'headers' => array(
                    // Set the Host header to self.
                    'Host' => $_SERVER['HTTP_HOST'],),
            );

            $url = httprl_build_url_self(drupal_get_path('module', 'flat_deposit') . "/Helpers/Ingest_service.php");

            // Debug mode
            global $user;
            $wrapper = entity_metadata_wrapper('node', $node);
            $status = $wrapper->upload_status->value();

            if ($status == 'failed' AND $user->name == 'admin') {
                $url_debug = str_replace($_SERVER['SERVER_ADDR'], "localhost", $url);

                $ch = curl_init();
                curl_setopt_array($ch, array(

                    CURLOPT_URL => $url_debug,
                    CURLOPT_COOKIE => "XDEBUG_SESSION=PHPSTORM",
                    CURLOPT_POST => sizeof($data_to_post),
                    CURLOPT_POSTFIELDS => $data_to_post));

                $result = curl_exec($ch);

                if($errno = curl_errno($ch)) {
                    $error_message = curl_strerror($errno);
                    echo "cURL error ({$errno}):\n {$error_message}";
                }
                curl_close($ch);

            } else {

            // normal background mode
                httprl_request($url, $options);

                // Execute request.
                httprl_send_request();
            }



            $form_state['redirect'] = '<front>';
            drupal_set_message('Frozen bundle is being processed');


            break;


        }

        case 'Edit bundle':{
            $url = array('node/' . $node->nid  . '/edit');
            $form_state['redirect'] = $url;
            break;
        }

        case 'Delete bundle':{
            $url = array('node/' . $node->nid  . '/delete');
            $form_state['redirect'] = $url;
            break;
        }

    }


}