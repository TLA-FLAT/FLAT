<?php


function get_nids($form_state){

    foreach ($form_state['values']['table_container'] as $index => $entry){
        if ($entry['checked']){
            $nids [$index] = $form_state['values']['data'][$index]['nid'];

        }
    }

    return $nids;
}


function parent_is_known($nid){

    $node = node_load ($nid);

    if($node !== false AND $node->type == 'flat_bundle'){

        $wrapper = entity_metadata_wrapper('node',$node);
        $parent_nid = (integer)$wrapper->flat_parent_nid->value() > 0 ? true : false;

    } else {

        $parent_nid = false;

    }


    return $parent_nid;
}


function has_cmdi($nid){


    $node = node_load ($nid);
    $wrapper = entity_metadata_wrapper('node',$node);

    $has_cmdi = !$wrapper->flat_cmdi_file->value() ? FALSE : TRUE;

    return $has_cmdi;
}



function is_valid_xml($nid, $has_cmdi){

    if ($has_cmdi){

        $node = node_load ($nid);
        $wrapper = entity_metadata_wrapper('node',$node);
        $file_field = $wrapper->flat_cmdi_file->value();
        $attached_file = file_load($file_field['fid']);
        $xml = simplexml_load_file(drupal_realpath($attached_file->uri));
        $is_valid = !$xml ? false : true;

    } else {

        $is_valid = false;
    }

    return $is_valid;

}


function has_correct_filename($nid){

    $node = node_load($nid);
    $wrapper = entity_metadata_wrapper('node', $node);
    $exploded_uri = explode('/', $wrapper->flat_cmdi_file->value()['uri']);
    $file_name = end($exploded_uri);

    $correct = $file_name == 'record.cmdi' ? TRUE : false;

    return $correct;
}

function bundle_file_exists($nid)
{
    $node = node_load($nid);
    $wrapper = entity_metadata_wrapper('node', $node);
    $location = $wrapper->flat_location->value();

    $exists = file_exists($location) ;

    return $exists;

}


function user_has_permissions($nid)
{

    global $user;

    $user_id = $user->uid;


    $node = node_load($nid);

    $bundle_id = $node->uid;

    if ($user_id === $bundle_id OR user_access('validate bundles')) {
        $has = true;

    } else {

        $has = false;
    }

    return $has;

}

function check_completeness($type, $nid, $index, $form_state)
{
    $complete = false;

    if ($form_state['values']['table_container'][$index]['checked']){

        $meta = $form_state['values']['data'][$index]['metadata'];
        $correct_meta = ($meta == 'available') ? true : false;

        $status = $form_state['values']['data'][$index]['status'];

        if ($type == 'validate'){

        $correct_status = ($status == 'open' OR $status == 'failed') ? true : false;

        } else {

            $correct_status = ($status == 'valid') ? true : false;

        }

        $complete = ($correct_status AND $correct_meta) ? true : false;

    }

    return $complete;
}



function check_permissions($nid){

    $node = node_load($nid);

    // Check user permissions
    global $user;
    $user_id = $user->uid;

    $bundle_id = $node->uid;

    if (($user_id === $bundle_id AND user_access('certified user')) OR user_access('ingest bundles')) {

        $permissions = true;

    } else {

        $permissions = false;
    }

    return $permissions;

}


function send_request($nid, $action, $debug=FALSE){

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
    $data_to_post['nid'] = $nid;
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

    if ($debug){

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


function move_node_data($nid){

    $error = [];

    module_load_include('inc', 'flat_deposit', 'inc/class.FlatBundle');
    $node = node_load($nid);
    $wrapper = entity_metadata_wrapper('node', $node);

    $move = FlatBundle::moveBundleData($node, 'data', 'unfreeze');

    if (!$move){
        $error ['nid'][] = 'data';
    }

    if (isset($wrapper->flat_cmdi_file)){
        $move = FlatBundle::moveBundleData($node, 'metadata', 'unfreeze');

        if (!$move){
            $error['nid'][] = 'metadata';
        }


    };

    return $error;
}



