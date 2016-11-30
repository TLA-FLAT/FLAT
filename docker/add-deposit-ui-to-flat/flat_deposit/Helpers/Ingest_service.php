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
define('SWORD_TMP_DIR', $config['sword_tmp_dir']);


/*
 * load configuration, helper functions and dependent classes
 */
require_once ($module_path . '/Helpers/Ingestor.inc');
require_once ($module_path . '/Helpers/Fedora_REST_API.inc');
require_once ($module_path . '/Helpers/CMDI_Handler.php');
require_once ($module_path . '/inc/php_functions.php');


// Processing routines
$nid = $_POST['nid'];

try {
    // INITIALIZE
    $ingest = new Ingestor($nid);

    $header  = "Ingest service log file - "  . $ingest->type . " on ".date("F j, Y, g:i a").PHP_EOL. "-------------------------";
    $ingest->AddEntryLogFile($header);

    $ingest->validateNodeStatus();
    $ingest->updateNodeStatus($ingest->type);


    // VALIDATION SPECIFIC ACTIONS
    if ($ingest->type == 'validating') {
        // access rights and data freeze
        $ingest->validate_backend_directory();
        $ingest->moveData('freeze');

        // CMDI completion: add resources and isPartOf property
        $ingest->addResourcesToCMDI();
        $ingest->addIsPartOfToCMDI();

    }

    // GENERAL ACTIONS
    // create bag for sword
    $ingest->prepareBag();
    $ingest->zipBag();

    #throw new IngestServiceException("Debugging");
    // execute sword
    $ingest->doSword();
    $ingest->checkStatusSword();


    // INGEST SPECIFIC ACTIONS
    if ($ingest->type == 'processing'){

        // execute doorkeeper
        $ingest->triggerDoorkeeper();
        $ingest->checkStatusDoorkeeper();

        $ingest->getConstituentFIDs();

        $ingest->changeOwnerId();


    }

    // FINISH
    $ingest->cleanup();
    $ingest->finalizeProcessing($nid);


} catch (IngestServiceException $e) {
    $ingest->AddEntryLogFile('Catching IngestServiceException');
    $ingest->updateNodeStatus('failed');
    $ingest->AddEntryLogFile('Error message: ' . $e->getMessage());
    $ingest->wrapper->upload_exception->set($e->getMessage());
    $ingest->wrapper->save();
    $ingest->create_blog_entry('failed');

    $ingest->rollback();

}





