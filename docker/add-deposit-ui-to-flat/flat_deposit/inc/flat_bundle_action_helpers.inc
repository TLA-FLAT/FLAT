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

function bundle_new_files($nid) {

    $node = node_load($nid);
    $wrapper = entity_metadata_wrapper('node', $node);
    $location = $wrapper->flat_location->value();

    $nFiles = count(file_scan_directory($location, '/.*/', array('min_depth' => 0)));

    if ($nFiles == 0) {

        return FALSE;

    }

    else {

        return TRUE;

    }

}

function bundle_deleted_files($nid) {

    $node = node_load($nid);
    $wrapper = entity_metadata_wrapper('node', $node);

    $deletedFiles = isset($wrapper->flat_deleted_resources) ? $wrapper->flat_deleted_resources->value() : NULL;

    if (!isset($deletedFiles) OR ($deletedFiles == '')) {

        return FALSE;

    }

    else {

        return TRUE;

    }

}

function bundle_max_files_exceeded($nid) {

    module_load_include('inc','flat_deposit','Helpers/CMDI/class.CmdiHandler');

    $node = node_load($nid);
    $wrapper = entity_metadata_wrapper('node', $node);
    $location = $wrapper->flat_location->value();

    $fedora_id = $wrapper->flat_fid->value();

    $cmdi_file = $wrapper->flat_cmdi_file->value();
    $cmdi_file_uri = $cmdi_file['uri'];

    if (file_exists(drupal_realpath($cmdi_file_uri))){
        $cmd = @CmdiHandler::simplexml_load_cmdi_file(drupal_realpath($cmdi_file_uri));
    }

    $new_files = [];
    if (file_exists($location)) {

        $new_files = array_diff(preg_grep('/^([^.])/', scandir($location)), array('..', '.'));

    }

    $new_files_count = count($new_files);

    $existing_files = [];

    if ($fedora_id && $cmd) {

        foreach ($cmd->children('cmd',true)->Resources->ResourceProxyList->ResourceProxy as $resource){

            $resource_type = (String)$resource->children('cmd',true)->ResourceType;
            if ($resource_type == 'LandingPage' || $resource_type == 'searchpage'){
                continue;
            }

            else {

                $resource_id = (string)$resource->attributes()->id;
                $resourceref_attributes = $resource->children('cmd',true)->ResourceRef->attributes('lat', TRUE);
                $fid = $resourceref_attributes['flatURI'];
        
                $fObj = islandora_object_load($fid);
                $label = $fObj->label;
                $existing_files[] = $label;

            }

        }

    }

    $existing_files_count = count($existing_files);

    $replaced_files = array_intersect($new_files, $existing_files);
    $replaced_files_count = count($replaced_files);

    $totalFiles = $new_files_count + $existing_files_count - $replaced_files_count;
    
    $max = variable_get('flat_deposit_ingest_service')['max_ingest_files'];

    if ($totalFiles > $max) {

        return TRUE;

    }

    else {

        return FALSE;

    }

}

function bundle_max_file_size_exceeded($nid) {

    $node = node_load($nid);
    $wrapper = entity_metadata_wrapper('node', $node);
    $location = $wrapper->flat_location->value();

    $max_size = variable_get('flat_deposit_ingest_service')['max_file_size'];
    $max_size_bytes = ($max_size * 1073741824);

    $files = file_scan_directory($location, '/.*/', array('min_depth' => 0));

    $max_file_size_exceeded = array();
    foreach ($files as $file) {
        $path = $file->uri;
        $name = basename($path);
        $size = filesize($path);
        if ($size > $max_size_bytes) {
            $max_file_size_exceeded[] = $name;
        }
    }
    if (!empty($max_file_size_exceeded)) {

        return $max_file_size_exceeded;

    }

    else {

        return FALSE;

    }

}

function bundle_invalid_file_names($nid) {

    $node = node_load($nid);
    $wrapper = entity_metadata_wrapper('node', $node);
    $location = $wrapper->flat_location->value();

    $filename_pattern = '/^[\da-zA-Z][\da-zA-Z\._\-]+$/';

    $files = file_scan_directory($location, '/.*/', array('min_depth' => 0));

    $invalid_file_names = array();
    foreach ($files as $file) {
        $path = $file->uri;
        $name = basename($path);
        if (preg_match($filename_pattern, $name) == FALSE) {
            $invalid_file_names[] = $name;
        }
    }
    if (!empty($invalid_file_names)) {

        return $invalid_file_names;

    }

    else {

        return FALSE;

    }

}

function bundle_invalid_file_extensions($nid) {

    $node = node_load($nid);
    $wrapper = entity_metadata_wrapper('node', $node);
    $location = $wrapper->flat_location->value();

    $allowed_extensions = explode(', ', variable_get('flat_deposit_ingest_service')['allowed_extensions']);

    $files = file_scan_directory($location, '/.*/', array('min_depth' => 0));

    $invalid_file_extensions = array();
    foreach ($files as $file) {
        $path = $file->uri;
        $name = basename($path);
        $extension = pathinfo($path, PATHINFO_EXTENSION);
        if (!in_array($extension, $allowed_extensions)) {
            $invalid_file_extensions[] = $name;
        }
    }
    if (!empty($invalid_file_extensions)) {

        return $invalid_file_extensions;

    }

    else {

        return FALSE;

    }

}

/**
 *
 * Function to check whether any resourceproxy has a flatUR attribute, which means it is referring to a file that is already archived
 *
 */
function has_flat_uri($nid) {
    module_load_include('inc','flat_deposit','Helpers/CMDI/class.CmdiHandler');

    $node = node_load($nid);
    $wrapper = entity_metadata_wrapper('node', $node);
    $location = $wrapper->flat_location->value();

    $fedora_id = $wrapper->flat_fid->value();

    $cmdi_file = $wrapper->flat_cmdi_file->value();
    $cmdi_file_uri = $cmdi_file['uri'];

    if (file_exists(drupal_realpath($cmdi_file_uri))){
        $cmd = @CmdiHandler::simplexml_load_cmdi_file(drupal_realpath($cmdi_file_uri));
    }

    if ($cmd) {

        foreach ($cmd->children('cmd',true)->Resources->ResourceProxyList->ResourceProxy as $resource){
            $resourceref_attributes = $resource->children('cmd',true)->ResourceRef->attributes('lat', TRUE);
            $fid = $resourceref_attributes['flatURI'];
                    if ($fid) {
                        return TRUE;
                    }
                }
            }

    return FALSE;

}

/**
 *
 * Function to check whether there is a matching file in the selected bundle directory for each of the resourceproxies in the CMDI
 *
 */
function bundle_files_mismatch($nid) {

    module_load_include('inc','flat_deposit','Helpers/CMDI/class.CmdiHandler');

    $node = node_load($nid);
    $wrapper = entity_metadata_wrapper('node', $node);
    $location = $wrapper->flat_location->value();

    $fedora_id = $wrapper->flat_fid->value();

    $cmdi_file = $wrapper->flat_cmdi_file->value();
    $cmdi_file_uri = $cmdi_file['uri'];

    if (file_exists(drupal_realpath($cmdi_file_uri))){
        $cmd = @CmdiHandler::simplexml_load_cmdi_file(drupal_realpath($cmdi_file_uri));
    }

    $new_files = [];

    if (file_exists($location)) {

        $new_files = array_diff(preg_grep('/^([^.])/', scandir($location)), array('..', '.'));

    }

    $existing_files = [];

    if ($cmd) {

        foreach ($cmd->children('cmd',true)->Resources->ResourceProxyList->ResourceProxy as $resource){

            $resource_type = (String)$resource->children('cmd',true)->ResourceType;
            if ($resource_type == 'LandingPage' || $resource_type == 'searchpage'){
                continue;
            }

            else {

                $resource_id = (string)$resource->attributes()->id;
                $resourceref_attributes = $resource->children('cmd',true)->ResourceRef->attributes('lat', TRUE);
                $localURI = $resourceref_attributes['localURI'];
                if ($localURI) {
                    $label = basename($localURI);
                    $existing_files[] = $label;
                }
                else {
                    $fid = $resourceref_attributes['flatURI'];
                    if ($fid) {
                        $fObj = islandora_object_load($fid);
                        $label = $fObj->label;
                        $existing_files[] = $label;
                    }
                }

            }

        }

    }

    $files_mismatch = array_diff($existing_files, $new_files);

    if (!empty($files_mismatch)) {

        return $files_mismatch;

    }

    else {

        return FALSE;

    }

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

        switch ($type){
            case  'validate':
                $correct_status = ($status == 'open' OR $status == 'failed') ? true : false;
                break;
            case  'archive':
                $correct_status = ($status == 'valid') ? true : false;
                break;
            case  'reopen':
                $correct_status = ($status == 'valid') ? true : false;
                break;
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

    // create array containing data needed by the ingest service
    $data_to_post = array();
    $data_to_post['drupal_path'] = DRUPAL_ROOT;
    $data_to_post['nid'] = $nid;
    $data_to_post['loggedin_user'] = $user->uid;
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
            CURLOPT_SSL_VERIFYPEER => false,
            CURLOPT_SSL_VERIFYHOST =>false,
            CURLOPT_COOKIE => "XDEBUG_SESSION=PHPSTORM",
            CURLOPT_POST => sizeof($data_to_post),
            CURLOPT_POSTFIELDS => $data_to_post));

        $result = curl_exec($ch);
        $httpcode = curl_getinfo($ch, CURLINFO_HTTP_CODE);

        if ($httpcode  != 200) {
            drupal_set_message("Unable to send sequential http request using curl to Ingest service");
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



