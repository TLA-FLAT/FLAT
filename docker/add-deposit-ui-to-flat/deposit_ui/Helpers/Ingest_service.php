<?php
/**
 * Ingest service
 */

/*
 * load configuration, helper functions and dependent classes
 */
require_once ($_POST['module_path'] . '/inc/config.inc');
require_once ($_POST['module_path'] . '/inc/php_functions.php');
require_once ($_POST['module_path'] . '/Helpers/Fedora_REST_API.inc');
require_once ($_POST['module_path'] . '/Helpers/Ingestor.inc');


/*
 *  Definition of all variables and constants
 */

//posted variables
define('DEPOSIT_UI_PATH', $_POST['module_path']);
define('FREEZE_DIR', $_POST['freeze_dir']);
define('BAG_DIR', $_POST['bag_dir']);
define('APACHE_USER', $_POST['apache_user']);

// variables stored in config.inc
$configuration = ingest_service_configuration();
define('SWORD_SCRIPT', $configuration['sword_script']);
define('CMD2DC', $configuration['cmd2dc']);
define('LAT2FOX', $configuration['lat2fox']);
define('BAG_EXE', $configuration['bag_exe']);
define('FEDORA_HOME', $configuration['fedora_home']);
putenv ('FEDORA_HOME=' . FEDORA_HOME);
define('LOG_ERRORS', $configuration['log_errors']);
define('ERROR_LOG_FILE', $configuration['error_log_file']);


// Create Database connection
$conf = get_drupal_database_settings();
$conn_string = "host=" . $conf['host'] . " port=" . $conf['port'] ." dbname=" . $conf['dbname'] . " user=" . $conf['user'] .  " password=" . $conf['pw'] ;
$db = pg_connect($conn_string) or die('Could not connect: ' . pg_last_error());;


// Perform SQL query to get nodes from drupal database; Query may result in many nodes, but here I query 1 single node as defined by the nid
$query = sprintf('SELECT * FROM node WHERE nid = \'%s\'',$_POST['nid']);
$results = pg_query($db, $query) or die('Query failed: ' . pg_last_error());


// Ingest routine
while ($node = pg_fetch_array($results, null, PGSQL_ASSOC)) {
    try {
        $ingest = new Ingestor($node, $db);
        
        $ingest->update_field('status','processing');

        $ingest->prepareBag();
        $ingest->zipBag();

        $ingest->doSword();

        $ingest->chownDirectory(BAG_DIR . "/" . $ingest->bag['bag_id']);
        $ingest->createFOXML();
        $ingest->chownDirectory(BAG_DIR . "/" . $ingest->bag['bag_id']);
        $ingest->batchIngest();

        $ingest->changeOwnerId();
        $ingest->update_field('status','archived');
        $ingest->cleanup();

    } catch (IngestServiceException $e) {
        $ingest->update_field('status','failed');
        $ingest->update_field('exception',$e->getMessage());
        $ingest->rollback();

    }

}

// Free resultset
pg_free_result($results);


pg_close($db);






