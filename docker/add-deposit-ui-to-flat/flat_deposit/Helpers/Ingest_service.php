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

$config = variable_get('flat_deposit_paths');
define('SWORD_TMP_DIR', $config['sword_tmp_dir']);



// Processing routines
$nid = $_POST['nid'];

try {
    // INITIALIZE
    module_load_include('inc','flat_deposit', '/Helpers/Ingestor');
    $ingest = new Ingestor($nid);

    $header  = "Ingest service log file - "  . $ingest->type . " on ".date("F j, Y, g:i a").PHP_EOL. "-------------------------";
    $ingest->AddEntryLogFile($header);


    $ingest->authenticateUser($_POST['sid']);

    $ingest->validateNodeStatus();
    $ingest->updateNodeStatus($ingest->type);


    // VALIDATION SPECIFIC ACTIONS
    if ($ingest->type == 'validating') {

        // set doorkeeper query parameter
        $ingest->doorkeeper_query = 'validate%20resources';

        // access rights and data freeze
        $ingest->validate_backend_directory();
        $ingest->moveData('freeze');


    }

    // GENERAL ACTIONS
    // CMDI completion: add resources and isPartOf property
    $ingest->adaptCMDIresources('ingest');
    $ingest->addIsPartOfToCMDI($ingest->parent_id);
#throw new IngestServiceException('debug');
    // create bag for sword
    $ingest->prepareBag();
    $ingest->zipBag();

    // CMDI completion: add resources and isPartOf property
    $ingest->adaptCMDIresources('rollback');
    $ingest->removeIsPartOfFromCMDI($ingest->parent_id);

    // execute sword
    $ingest->doSword();
    $ingest->checkStatusSword();

    // execute doorkeeper
    $ingest->triggerDoorkeeper($ingest->doorkeeper_query);
    $ingest->checkStatusDoorkeeper();


    // INGEST SPECIFIC ACTIONS
    if ($ingest->type == 'processing'){

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
    $ingest->wrapper->upload_exception->set(get_class($e));
    $ingest->wrapper->save();
    $ingest->create_blog_entry('failed', $e->getMessage());

    $ingest->rollback();

}





