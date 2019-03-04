<?php
/**
 * Ingest service
 */

// include drupal functionality
#$drupal_path = '/var/www/html/flat/'; #remove when done
$drupal_path = $_POST['drupal_path'];
chdir($drupal_path);
define('DRUPAL_ROOT', getcwd()); //the most important line
require_once './includes/bootstrap.inc';
drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);

// Info from Post request
$nid = $_POST['nid'];
$loggedin_user = $_POST['loggedin_user'];
$test = $_POST['test'];

#$nid = 16;
#$test = 'validate';
#$sid = 'IZBkDIRmXu9UWBJnxSMofMKXqMZm2QxJbnQzu-IMF7U';

// transform parameter test
$test = ($test == 'Validate bundle') ? TRUE : FALSE;

// going to be a node field
$posted_cmdi_handling = 'use existing';

// get bundle info from node
$node = node_load($nid);
$wrapper = entity_metadata_wrapper('node',$node);

// get owner name from node
$sipOwner = user_load($node->uid);
$sipOwnerName = $sipOwner->name;

// define SIP type
$sipType = 'Bundle';

// get full record cmdi file path from node file field
$metadata_file_info =$wrapper->flat_cmdi_file->value();
$recordCmdi = drupal_realpath($metadata_file_info['uri']);


// get fedora ID of parent by loading node with node-id 'flat_parent_nid'
$collection_nid = $wrapper->flat_parent_nid->value();
$collection_node = node_load($collection_nid);
$collection_wrapper = entity_metadata_wrapper('node',$collection_node);
$collection_fid = $collection_wrapper->flat_fid->value();


// instantiate client
module_load_include('php','flat_deposit','Helpers/IngestService/IngestClient');

try {
    $ingest_client = new IngestClient($sipType, $sipOwnerName, $recordCmdi, $collection_fid, $test);
} catch (IngestServiceException $exception){

}

// set ingest parameters
$info['loggedin_user'] = $loggedin_user;
$info['nid']= $nid;

$try = $ingest_client->requestSipIngest($info);



