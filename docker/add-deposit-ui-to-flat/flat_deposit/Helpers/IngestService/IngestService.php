<?php
/**
 * Ingest service
 */

// include drupal functionality
$drupal_path = '/var/www/html/flat/'; #remove when done
#$drupal_path = $_POST['drupal_path'];
chdir($drupal_path);
define('DRUPAL_ROOT', getcwd()); //the most important line
require_once './includes/bootstrap.inc';
drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);


/*
 *  Definition of all variables and constants
 */
//posted variables
$module_path = drupal_realpath(drupal_get_path('module','flat_deposit'));
define('DEPOSIT_UI_PATH', $module_path);
define('FREEZE_DIR', variable_get('flat_deposit_paths')['freeze']);
define('BAG_DIR', variable_get('flat_deposit_paths')['bag']);
define('APACHE_USER', variable_get('flat_deposit_names')['apache_user']);

// variables stored in database
$config = variable_get('flat_deposit_ingest_service');
define('BAG_EXE', $config['bag_exe']);
define('LOG_ERRORS', $config['log_errors']);
define('ERROR_LOG_FILE', $config['error_log_file']);

$config = variable_get('flat_deposit_paths');
define('SWORD_TMP_DIR', $config['sword_tmp_dir']);



// Processing routines
#$nid = $_POST['nid'];
$nid = 8;
$node = node_load($nid);
$wrapper = entity_metadata_wrapper('node',$node);

$sipType = 'Bundle';

$user = user_load($node->uid);
$userName = $user->name;

$metadata_file_info =$wrapper->flat_cmdi_file->value();
$recordCmdi = drupal_realpath($metadata_file_info['uri']);



$collection_nid = $wrapper->flat_parent_nid->value();
$collection_node = node_load($collection_nid);
$collection_wrapper = entity_metadata_wrapper('node',$collection_node);
$collection_fid = $collection_wrapper->flat_fid->value();



$pathResources = '.collection';
$test = FALSE;

module_load_include('php','flat_deposit','Helpers/IngestService/IngestClient');
$ingest_client = new IngestClient($sipType, $userName, $recordCmdi, $pathResources, $collection_fid, $test);

$fid = $ingest_client->requestSipIngest();



echo 'done';




