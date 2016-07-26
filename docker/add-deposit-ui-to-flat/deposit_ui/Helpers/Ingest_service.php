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

    public $db_connection;

    public function __construct($data)
    {

        $this->entry = $data;
        $this->backend_bundle_dir = FREEZE_DIR . '/' . $this->entry['user_id'] . '/' . $this->entry['bundle'];
        $this->pid = array();
        $config = get_metadata_configuration();
        $this->md_prefix = $config['prefix'];

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
    public function chownDirectory($dir){
        $user = APACHE_USER . ":" . APACHE_USER;
        $command = sprintf("sudo chown -R %s %s",  $user , $dir);
        exec($command, $output_chmod, $return);
        if ($return) {
            $message = 'Unable to adapt rights of directory ' . $dir;
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

        $command = "java -Xmx4096m -jar " . LAT2FOX . " -d=" . CMD2DC . ' -h -z ' . BAG_DIR . '/' . $this->entry['bag_id'];
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
     * Batch_Ingest for whole bag /SIP.
     *
     * After running the command line batch ingest script provided by fedora, output of the command is checked. In case return value is != 0,
     * an exception is raised. Otherwise the number of ingested files is compared with the number of files as counted before ingest. In case of differences.
     * another exception is raised.
     *
     * @throws IngestServiceException
     */

    public function batchIngest(){
        $fedora_config = get_configuration_fedora();
        $command_b = FEDORA_HOME . "/client/bin/fedora-batch-ingest.sh " .
            BAG_DIR . "/" . $this->entry['bag_id'] . '/fox ' .
            BAG_DIR . "/" . $this->entry['bag_id'] . "/log xml info:fedora/fedora-system:FOXML-1.1 " .
            $fedora_config['host_name'] . ':' . $fedora_config['port'] . " "  .
            $fedora_config['user'] . " "  . $fedora_config['password'] . " "  . $fedora_config['scheme'] . " "  . $fedora_config['context'] ;
        exec($command_b, $output_ingest, $return);
        if ($return) {
            $message = 'Failed to ingest bundle ';
            if (LOG_ERRORS) error_log ( date(DATE_RSS) . ";\t" . $message . ";\t". implode(" ",$output_ingest) ."\n", $message_type = 3 , ERROR_LOG_FILE );
            throw new IngestServiceException ($message);
        } else {
            // extract pid from command output
            $Ingests = preg_grep("/ingest succeeded for:/", $output_ingest );
            foreach ($Ingests as $f){
                $pid = str_replace("ingest succeeded for: ", "", $f);
                $pid = str_replace(".xml", "", $pid);
                $pid = str_replace( $this->md_prefix . "_", $this->md_prefix . ":", $pid);
                if (substr($pid,-3) == "CMD") {
                    $pid = str_replace("_CMD", "", $pid);}

                $this->pid[] = $pid;}

            if (count($this->pid) != $this->entry['nfiles']) {
                $message = 'Ingest of bundle only partially succeeded. Check file naming in CMDI file or existing FOXML objects with same PID';
                if (LOG_ERRORS) error_log ( date(DATE_RSS) . ";\t" . $message . ";\t". implode(" ",$this->pid) ."\n", $message_type = 3 , ERROR_LOG_FILE );
                throw new IngestServiceException ($message);
            } else {
                // extract the pid of bundle and write to database
                $this->entry['pid_bundle'] = $this->pid[0];
                $this->entry['bag_ingested'] = 1;

            }
        }

    }


    /**
     * Call to change ownerID of fedora objects using the Fedora REST api.
     *
     * @throws IngestServiceException
     */
    public function changeOwnerId(){

        // create object that can do ReST requests
        $accessFedora = get_configuration_fedora();
        $rest_fedora = new FedoraRESTAPI($accessFedora);


        // Change ownership of ingested files
        $errors_occurred=0;
        $data = array(
            'ownerId' => $this->entry['user_id']);

        foreach ($this->pid as $pid) {
            $result = $rest_fedora->modifyObject($pid, $data);

            if (!$result) {$errors_occurred++;}
        }

        // rollback
        if ($errors_occurred > 0){

            foreach ($this->pid as $pid){
                $rest_fedora->deleteObject($pid);
            }

            $message = 'Couldn\'t change ownership of files';
            if (LOG_ERRORS) error_log ( date(DATE_RSS) . ";\t" . $message . ";\n", $message_type = 3 , ERROR_LOG_FILE );
            throw new IngestServiceException ($message);

        } else {
            $pid_data = $rest_fedora->getObjectData($this->pid[0]);
            $this->entry['date_bundle_ingest'] = strtotime($pid_data['objCreateDate']);
            $this->entry['owner_added'] = 1;
            $this->entry['status'] = 'archived';
        }
    }

public function deleteOriginal(){
    if ($this->entry['status'] = 'archived'){
        # delete original
        recursiveRmDir($this->backend_bundle_dir);
        rmdir ($this->backend_bundle_dir);
        if (!file_exists($this->backend_bundle_dir)) $this->entry['data_purged'] = 1;
    }
}
    /**
     * @param $db database connection handle
     */
    public function updateDatabase (){
        $conditions= array(
            "user_id" => $this->entry['user_id'],
            "bundle" => $this->entry['bundle'],
        );
        $res = pg_update($this->db_connection, 'flat_deposit_ui_upload', $this->entry, $conditions);
        if (!$res) {
            echo ( "Database table has not been updated\n");
        }
    }
}



/*
 * Script part run by every instance
 */

$conf = get_drupal_database_settings();
$conn_string = "host=" . $conf['host'] . " port=" . $conf['port'] ." dbname=" . $conf['dbname'] . " user=" . $conf['user'] .  " password=" . $conf['pw'] ;
$db = pg_connect($conn_string) or die('Could not connect: ' . pg_last_error());;


// Performing SQL query to pick up all information of one specific bundle
$query = sprintf('SELECT * FROM flat_deposit_ui_upload WHERE user_id = \'%s\' AND bundle = \'%s\' and collection = \'%s\'',$user_id, $bundle, $collection);
$results = pg_query($db, $query) or die('Query failed: ' . pg_last_error());


// Try to ingest one specified row (could be several but selection is now made in commit changes).
while ($row = pg_fetch_array($results, null, PGSQL_ASSOC)) {
    try {
        $ingest = new Ingestor($row);
        $ingest->db_connection = $db;
        $ingest->reset_entry();
        $ingest->entry['status'] = 'being processed';
        $ingest->updateDatabase();

        $ingest->prepareBag();
        $ingest->zipBag();

        $ingest->doSword();

        $ingest->chownDirectory(BAG_DIR . "/" . $ingest->entry['bag_id']);
        $ingest->createFOXML();
        $ingest->chownDirectory(BAG_DIR . "/" . $ingest->entry['bag_id']);
        $ingest->batchIngest();

        $ingest->changeOwnerId();
        $ingest->deleteOriginal();

    } catch (IngestServiceException $e) {
        $ingest->entry['status'] = 'failed';
        $ingest->entry['exceptions'] = $e->getMessage();
        print_r($ingest->entry['exceptions']);

    } finally {
        $ingest->updateDatabase();
        // Cleanup: bagit files, zip file and bag
        array_map('unlink', glob($ingest->backend_bundle_dir . "/*.txt"));
        if (file_exists(FREEZE_DIR . '/' . $ingest->entry['user_id'] . '/' . $ingest->entry['bundle'] . ".zip")) unlink(FREEZE_DIR . '/' . $ingest->entry['user_id'] . '/' . $ingest->entry['bundle'] . ".zip");
        if (file_exists(BAG_DIR . '/' . $ingest->entry['bag_id']) && $ingest->entry['bag_id']) {
            recursiveRmDir(BAG_DIR . '/' . $ingest->entry['bag_id']);
            rmdir(BAG_DIR . '/' . $ingest->entry['bag_id']);
        }

    }

}

// Free resultset
pg_free_result($results);


pg_close($db);






