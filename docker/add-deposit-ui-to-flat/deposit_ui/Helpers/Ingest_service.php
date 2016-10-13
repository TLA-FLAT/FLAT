<?php
/**
 * Ingest service
 */
// include drupal functionality
$drupal_path = $_POST['drupal_path'];
chdir($drupal_path);
define('DRUPAL_ROOT', getcwd()); //the most important line
require_once './includes/bootstrap.inc';
drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);

/*
 * load configuration, helper functions and dependent classes
 */
$module_path = drupal_realpath(drupal_get_path('module','flat_deposit_ui'));
require_once ($module_path . '/inc/config.inc');
require_once ($module_path . '/inc/php_functions.php');
require_once ($module_path . '/Helpers/Fedora_REST_API.inc');
require_once ($module_path . '/Helpers/Ingestor.inc');
require_once ($module_path . '/Helpers/CMDI_Handler.php');

/*
 *  Definition of all variables and constants
 */
//posted variables
define('DEPOSIT_UI_PATH', $module_path);
define('FREEZE_DIR', variable_get('flat_deposit_paths',array())['freeze']);
define('BAG_DIR', variable_get('flat_deposit_paths',array())['bag']);
define('APACHE_USER', variable_get('flat_deposit_names',array())['apache_user']);

// variables stored in config.inc
$configuration = get_configuration_ingest_service();
define('BAG_EXE', $configuration['bag_exe']);
define('LOG_ERRORS', $configuration['log_errors']);
define('ERROR_LOG_FILE', $configuration['error_log_file']);
define('SWORD_TMP_DIR', $configuration['sword_tmp_dir']);

// Ingest routine
$nid = $_POST['nid'];

try {
    $ingest = new Ingestor($nid);
    #throw new IngestServiceException("Debugging");

    $ingest->wrapper->upload_status->set('processing');
    $ingest->wrapper->save();

    $ingest->addResourcesToCMDI();
    $ingest->addIsPartOfToCMDI();

    $ingest->prepareBag();
    $ingest->zipBag();

    $ingest->doSword();
    $ingest->checkStatusSword();

    $ingest->triggerDoorkeeper();
    $ingest->checkStatusDoorkeeper();

    $ingest->getConstituentFIDs();

    $ingest->changeOwnerId();

    $ingest->cleanup();

    $ingest->create_blog_entry('succeeded');

    #node_delete_multiple(array($nid));
    $ingest->wrapper->upload_status->set('awaiting');
    $ingest->wrapper->save();

} catch (IngestServiceException $e) {
    $ingest->wrapper->upload_status->set('failed');
    $ingest->wrapper->upload_exception->set($e->getMessage());
    $ingest->wrapper->save();
    $ingest->create_blog_entry('failure');

    $ingest->rollback();

}





