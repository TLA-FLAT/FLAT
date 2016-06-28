<?php
/**
 * Ingest service
 */


/*
 * load dependencies and settings
 */
require_once ($_POST['module_path'] . '/inc/config.inc');
require_once ($_POST['module_path'] . '/Helpers/Fedora_REST_API.inc');

/*
 * Variable and constant definitions
 */
$user_id=$_POST['user_id'];
$bundle=$_POST['bundle'];
$collection=$_POST['collection'];
define('DEPOSIT_UI_PATH', $_POST['module_path']);
define('SWORD_SCRIPT', $_POST['sword_script']);
define('CMD2DC', $_POST['cmd2dc']);
define('LAT2FOX', $_POST['lat2fox']);
define('FREEZE_DIR', $_POST['freeze_dir']);
define('BAG_DIR', $_POST['bag_dir']);
define('APACHE_USER', $_POST['apache_user']);
define('BAG_EXE', $_POST['bag_exe']);
define('FEDORA_HOME', $_POST['fedora_home']);
putenv ('FEDORA_HOME=' . FEDORA_HOME);
define('LOG_ERRORS', $_POST['log_errors']);
define('ERROR_LOG_FILE', $_POST['error_log_file']);

/*
 * function and object definitions
 */

/**
 * Recursively removes a directory
 *
 * @param string $dir the name of the directory which will be deleted recursively
 */
function recursiveRmDir($dir)
{
    $iterator = new RecursiveIteratorIterator(new \RecursiveDirectoryIterator($dir, \FilesystemIterator::SKIP_DOTS), \RecursiveIteratorIterator::CHILD_FIRST);
    foreach ($iterator as $filename => $fileInfo) {
        if ($fileInfo->isDir()) {
            rmdir($filename);
        } else {
            unlink($filename);
        }
    }
}

/**
 * Class IngestServiceException is an exception class
 */
class IngestServiceException extends Exception {}


/**
 * Class Ingestor 
 */
class Ingestor
{

    public function __construct($data)
    {
        $this->entry = $data;
        $this->backend_bundle_dir = FREEZE_DIR . '/' . $this->entry['user_id'] . '/' . $this->entry['bundle'];
        $this->ingests = array();
    }

    public function reset_entry(){
        $this->entry['status'] = 'awaiting';
        $this->entry['exceptions'] = NULL;
        $this->entry['baginplace'] = 0;
        $this->entry['zipped'] = 0;
        $this->entry['nfiles'] = NULL;
        $this->entry['bagged'] = 0;
        $this->entry['bag_id'] = NULL;
        $this->entry['foxml'] = 0;
        $this->entry['bag_ingested'] = 0;
        $this->entry['data_purged'] = 0;
        $this->entry['pid_bundle'] = NULL;
        $this->entry['owner_added'] = NULL;
        $this->entry['date_bundle_ingest'] = NULL;
    }

    /**
     * make bag with data at right location
     *
     */

     public function prepareBag()
    {
        $command = BAG_EXE . ' baginplace ' . $this->backend_bundle_dir;
        exec($command, $output, $return);
        if ($return) {
            $message = 'Error making bag';
            if (LOG_ERRORS) error_log ( date(DATE_RSS) . ";\t" . $message . ";\t". implode(" ",$output) ."\n", $message_type = 3 , ERROR_LOG_FILE );
            throw new IngestServiceException ($message);
        }
        $command = BAG_EXE . ' update ' . $this->backend_bundle_dir;
        exec($command, $output, $return);
        if ($return) {
            $message = 'Error updating bag info';
            if (LOG_ERRORS) error_log ( date(DATE_RSS) . ";\t" . $message . ";\t". implode(" ",$output) ."\n", $message_type = 3 , ERROR_LOG_FILE );
            throw new IngestServiceException ($message);
        } else {
            $this->entry['baginplace'] = 1;
        }

    }

    /**
     * Zips all unhidden files and make bag in bag directory
     *
     *
     */
    public function zipBag()
    {
        $command = DEPOSIT_UI_PATH . "/Helpers/scripts/zip_sip.sh " . $this->entry['user_id'] . " " . $this->entry["bundle"] . " " . BAG_DIR . " " . $this->backend_bundle_dir;
        exec($command, $output_prep, $return);
        if ($return) {
            $message = 'Error creating zip file';
            if (LOG_ERRORS) error_log ( date(DATE_RSS) . ";\t" . $message . ";\t". implode(" ",$output_prep) ."\n", $message_type = 3 , ERROR_LOG_FILE );
            throw new IngestServiceException ($message);
        } else {
            $this->entry['zipped'] = 1;
            $this->entry['nfiles'] = intval(str_split($output_prep['0'], 18)[1]);
        }
    }

    /**
     * Create Bag at correct bag location
     *
     * @throws IngestServiceException
     */

    public function doSword(){
        $command = SWORD_SCRIPT . ' ' . $this->backend_bundle_dir . '/../' . $this->entry['bundle'] . ".zip";
        exec($command, $output_bag, $return);

        $bag_id = str_split($output_bag['4'], strrpos($output_bag['4'], '/') + 1)[1];
        
        // sleep because sword is slow
        sleep (15);
        if (!$bag_id || !file_exists(BAG_DIR . '/' . $bag_id)) {
            $message = 'Error creating bag. Check bag log (deposit/sword/tmp/' . $bag_id . '/deposit.properties)';
            if (LOG_ERRORS) error_log ( date(DATE_RSS) . ";\t" . $message . ";\t". implode(" ",$output_bag) ."\n", $message_type = 3 , ERROR_LOG_FILE );
            throw new IngestServiceException ($message);
        } else {
            $this->entry['bagged'] = 1;
            $this->entry['bag_id'] = $bag_id;
        }
    }

    /**
     * Change rights of newly created bag directory
     *
     * @throws IngestServiceException
     */
    public function changeRightsBagDir(){
        $command = "sudo chmod -R 777 " . BAG_DIR . "/" . $this->entry['bag_id'];
        exec($command, $output_chmod, $return);
        if ($return) {
            $message = 'Unable to adapt rights of bag directory';
            if (LOG_ERRORS) error_log ( date(DATE_RSS) . ";\t" . $message . ";\t". implode(" ",$output_chmod) ."\n", $message_type = 3 , ERROR_LOG_FILE );
            throw new IngestServiceException ($message);
        }
    }

    /**
     *  generates FOXML files from CMDI
     *
     * @throws IngestServiceException
     */
    public function createFOXML(){

        $command = "java -Xmx4096m -jar " . LAT2FOX . " -d=" . CMD2DC . ' ' . BAG_DIR . '/' . $this->entry['bag_id'];
        exec($command, $output_fox, $return);
        if ($return) {
            $message = 'Error creating foxml files';
            if (LOG_ERRORS) error_log ( date(DATE_RSS) . ";\t" . $message . ";\t". implode(" ",$output_fox) ."\n", $message_type = 3 , ERROR_LOG_FILE );
            throw new IngestServiceException ($message);
        } else {
            $this->entry['foxml'] = 1;
        }
    }

    /**
     * Batch_Ingest for whole bag /SIP
     *
     * @throws IngestServiceException
     */

    public function batchIngest(){
        $command_b = FEDORA_HOME . "/client/bin/fedora-batch-ingest.sh " . BAG_DIR . "/" . $this->entry['bag_id'] . '/fox ' . BAG_DIR . "/" . $this->entry['bag_id'] . "/log xml info:fedora/fedora-system:FOXML-1.1 localhost:8443 fedoraAdmin fedora https fedora";
        exec($command_b, $output_ingest, $return);
        if ($return) {
            $message = 'Failed to ingest bundle ';
            if (LOG_ERRORS) error_log ( date(DATE_RSS) . ";\t" . $message . ";\t". implode(" ",$output_ingest) ."\n", $message_type = 3 , ERROR_LOG_FILE );
            throw new IngestServiceException ($message);
        } else {
            // compare number of ingested files with number of frozen files
            $Ingests = preg_grep("/ingest succeeded for:/", $output_ingest );
            if (count($Ingests) == $this->entry['nfiles']) {
                $pid_bundle =preg_grep("/CMD.xml/", $Ingests );
                $pid_bundle = str_replace("ingest succeeded for: ", "", $pid_bundle);
                $pid_bundle = str_replace(".xml", "", $pid_bundle);
                $pid_bundle = str_replace("lat_", "lat:", $pid_bundle);
                $this->entry['pid_bundle'] = $pid_bundle[0];
                $this->entry['bag_ingested'] = 1;
                $this->ingests = $Ingests;

                # delete original
                recursiveRmDir($this->backend_bundle_dir);
                rmdir ($this->backend_bundle_dir);
                if (!file_exists($this->backend_bundle_dir)) $this->entry['data_purged'] = 1;

            } else {
                $message = 'Ingest of bundle only partially succeeded. Check file naming in CMDI file or existing FOXML objects with same PID';
                if (LOG_ERRORS) error_log ( date(DATE_RSS) . ";\t" . $message . ";\t". implode(" ",$Ingests) ."\n", $message_type = 3 , ERROR_LOG_FILE );
                throw new IngestServiceException ($message);
            }
        }

    }


    /**
     *
     * @param array $Ingests
     * @throws IngestServiceException
     */
    public function changeOwnership($Ingests){

        // create object that can do ReST requests
        $accessFedora = get_configuration_fedora();
        $rest_fedora = new FedoraRESTAPI($accessFedora);

        // Change ownership of ingested files
        foreach ($Ingests as $f) {
            $pid = str_replace("ingest succeeded for: ", "", $f);
            $pid = str_replace(".xml", "", $pid);
            $pid = str_replace("lat_", "lat:", $pid);
            $data = array(
                'ownerId' => $this->entry['user_id']
            );

            $result = $rest_fedora->modifyObject($pid, $data);
            if (!$result) {
                $message = 'Couldn\'t change ownership of files';
                if (LOG_ERRORS) error_log ( date(DATE_RSS) . ";\t" . $message . ";\n", $message_type = 3 , ERROR_LOG_FILE );
                throw new IngestServiceException ($message);
            } else {
                $pid_data = $rest_fedora->getObjectData($pid);
                $this->entry['date_bundle_ingest'] = strtotime($pid_data['objCreateDate']);
            }

        }
        $this->entry['owner_added'] = 1;
        $this->entry['status'] = 'archived';
    }

    /**
     * @param $db database connection handle
     */
    public function updateDatabase ($db){
        $conditions= array(
            "user_id" => $this->entry['user_id'],
            "bundle" => $this->entry['bundle'],
        );
        $res = pg_update($db, 'flat_deposit_ui_upload_log', $this->entry, $conditions);
        if (!$res) {
            echo ( "Database table has not been updated\n");
        }
    }
}



/*
 * Script part run by every instance
 */

/*
 * Uncomment this to create file where time of script execution is logged
 */

#$file = "/app/flat/deposit/Ingest_service.log";
#file_get_contents($file);
#file_put_contents($file, 'Script 2 has slept enough and awakes at ' . date ('D, d M Y H:i:s') . "\n", FILE_APPEND | LOCK_EX);

$conf = get_drupal_database_settings();
$conn_string = "host=" . $conf['host'] . " port=" . $conf['port'] ." dbname=" . $conf['dbname'] . " user=" . $conf['user'] .  " password=" . $conf['pw'] ;
$db = pg_connect($conn_string) or die('Could not connect: ' . pg_last_error());;


// Performing SQL query to pick up all information of one specific bundle
$query = sprintf('SELECT * FROM flat_deposit_ui_upload_log WHERE user_id = \'%s\' AND bundle = \'%s\' and collection = \'%s\'',$user_id, $bundle, $collection);
$results = pg_query($db, $query) or die('Query failed: ' . pg_last_error());


// Try to ingest one specified row (could be several but selection is now made in commit changes).
while ($row = pg_fetch_array($results, null, PGSQL_ASSOC)) {
    try {
        $ingest = new Ingestor($row);
        $ingest->entry['status'] = 'being processed';
        $ingest->updateDatabase($db);

        $ingest->prepareBag();
        $ingest->zipBag();

        $ingest->doSword();

        $ingest->changeRightsBagDir();
        $ingest->createFOXML();
        $ingest->batchIngest();

        $ingest->changeOwnership($ingest->ingests);

    } catch(IngestServiceException $e){
        $ingest->reset_entry();
        $ingest->entry['status'] = 'failed';
        $ingest->entry['exceptions'] = $e->getMessage();
        print_r($ingest->entry['exceptions']);

    } finally {
        // Cleanup: bagit files, zip file and bag
        array_map('unlink', glob($ingest->backend_bundle_dir . "/*.txt"));
        if (file_exists(FREEZE_DIR . '/' . $ingest->entry['user_id'] . '/' . $ingest->entry['bundle'] . ".zip")) unlink(FREEZE_DIR . '/' . $ingest->entry['user_id'] . '/' . $ingest->entry['bundle'] . ".zip");
        if (file_exists(BAG_DIR . '/' . $ingest->entry['bag_id']) && $ingest->entry['bag_id']) {recursiveRmDir(BAG_DIR . '/' . $ingest->entry['bag_id']);
            rmdir(BAG_DIR . '/' . $ingest->entry['bag_id']);}
        // update entry on database
        $ingest->updateDatabase($db);
    }


    }


// Free resultset
pg_free_result($results);


pg_close($db);






