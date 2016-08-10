<?php

/**
 * Created by PhpStorm.
 * User: danrhe
 * Date: 28/07/16
 * Time: 16:06
 */

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
    public $node;

    /*
     * Constructor function for Ingestor class. Needs array of node data
     *
     * @param $data row data from drupal node table
     * @param $db database connection
     */
    public function __construct($node_data, $db)
    {

        $this->entry = $node_data;
        $this->node["nid"] =  $node_data['nid'];
        $this->node["bundle"] =  $node_data['title'];
        $this->node["uid"] =  $node_data['uid'];

        // add exception and status to node
        $config = get_metadata_configuration();
        $this->md_prefix = $config['prefix'];

        $this->db_connection = $db;
        $this->node["collection"] =  $this->get_collection();
        $this->node["user"] =  $this->get_user($this->node['uid']);
        $this->backend_bundle_dir = FREEZE_DIR . "/" . $this->node['user']. "/" . $this->node['collection'] . "/" . $this->node['bundle'];

        $this->bag = array();

    }

    /**
     * Queries the collection data field table of the drupal database to receive the associated collection value
     * @return string $collection
     */
    public function get_collection(){

        $query = sprintf('SELECT upload_collection_value FROM field_data_upload_collection WHERE entity_id = \'%s\'',$this->node['nid']);
        $results = pg_query($this->db_connection, $query) or die('Query failed: ' . pg_last_error());

        $collection = pg_fetch_array($results, null, PGSQL_ASSOC)['upload_collection_value'];
        return $collection;

    }

    /**
     * Queries drupal "users"-table to receive the user name belonging to uid
     *
     * $param $string uid user ID
     *
     * @return string $collection
     */
    public function get_user($uid){

        $query = sprintf('SELECT name FROM users WHERE uid = \'%s\'',$uid);
        $results = pg_query($this->db_connection, $query) or die('Query failed: ' . pg_last_error());

        $name = pg_fetch_array($results, null, PGSQL_ASSOC)['name'];
        return $name;

    }

    /**
     * make bag with data at right location
     *
     */

    public function prepareBag()
    {
        $command = BAG_EXE . ' baginplace ' . "\"" . $this->backend_bundle_dir . "\"";
        exec($command, $output, $return);
        if ($return) {
            $message = 'Error making bag';
            if (LOG_ERRORS) error_log ( date(DATE_RSS) . ";\t" . $message . ";\t". implode(" ",$output) ."\n", $message_type = 3 , ERROR_LOG_FILE );
            throw new IngestServiceException ($message . implode(";\n",$output));
        }
        $command = BAG_EXE . ' update ' . "\"" . $this->backend_bundle_dir . "\"";
        exec($command, $output, $return);
        if ($return) {
            $message = 'Error updating bag info';
            if (LOG_ERRORS) error_log ( date(DATE_RSS) . ";\t" . $message . ";\t". implode(" ",$output) ."\n", $message_type = 3 , ERROR_LOG_FILE );
            throw new IngestServiceException ($message . implode(";\n",$output));
        }

    }

    /**
     * Zips all unhidden files and make bag in bag directory
     *
     *
     */
    public function zipBag()
    {
        $command = DEPOSIT_UI_PATH . "/Helpers/scripts/zip_sip.sh \"$this->backend_bundle_dir\" " . $this->node['bundle'];
        exec($command, $output_prep, $return);
        if ($return) {
            $message = 'Error creating zip file;';
            if (LOG_ERRORS) error_log ( date(DATE_RSS) . ";\t" . $message . ";\t". implode(" ",$output_prep) ."\n", $message_type = 3 , ERROR_LOG_FILE );
            throw new IngestServiceException ($message . implode(";\n", $output_prep). implode(";\n", $command));
        } else {
            $this->node['nfiles'] = intval(str_split($output_prep['0'], 18)[1]);
        }

    }

    /**
     * Create Bag at correct bag location
     *
     * @throws IngestServiceException
     */

    public function doSword(){
        $command = "cd \"" . $this->backend_bundle_dir . "\"/../;" . SWORD_SCRIPT  . " " . $this->node['bundle'] . ".zip";
        exec($command, $output_bag, $return);

        $bag_id = str_split($output_bag['4'], strrpos($output_bag['4'], '/') + 1)[1];

        // sleep because sword is slow
        sleep (15);
        if (!$bag_id || !file_exists(BAG_DIR . '/' . $bag_id)) {
            $message = 'Error creating bag. Check bag log (deposit/sword/tmp/' . $bag_id . '/deposit.properties)';
            if (LOG_ERRORS) error_log ( date(DATE_RSS) . ";\t" . $message . ";\t". implode(" ",$output_bag) ."\n", $message_type = 3 , ERROR_LOG_FILE );
            throw new IngestServiceException ($message . implode(";\n", $output_bag). implode(";\n", $command));
        } else {
            $this->bag['bag_id'] = $bag_id;
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

        $command = "java -Xmx4096m -jar " . LAT2FOX . " -d=" . CMD2DC . ' -h -z ' . BAG_DIR . '/' . $this->bag['bag_id'];
        exec($command, $output_fox, $return);
        if (!file_exists(BAG_DIR . '/' . $this->bag['bag_id']. '/fox')) {
            $message = 'Error creating foxml files';
            if (LOG_ERRORS) error_log ( date(DATE_RSS) . ";\t" . $message . ";\t". implode(" ",$output_fox) ."\n", $message_type = 3 , ERROR_LOG_FILE );
            throw new IngestServiceException ($message);
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
            BAG_DIR . "/" . $this->bag['bag_id'] . '/fox ' .
            BAG_DIR . "/" . $this->bag['bag_id'] . "/log xml info:fedora/fedora-system:FOXML-1.1 " .
            $fedora_config['host_name'] . ':' . $fedora_config['port'] . " "  .
            $fedora_config['user'] . " "  . $fedora_config['password'] . " "  . $fedora_config['scheme'] . " "  . $fedora_config['context'] ;
        exec($command_b, $output_ingest, $return);
        if ($return) {
            $message = 'Failed to ingest bundle ';
            if (LOG_ERRORS) error_log ( date(DATE_RSS) . ";\t" . $message . ";\t". implode(" ",$output_ingest) ."\n", $message_type = 3 , ERROR_LOG_FILE );
            throw new IngestServiceException ($message . implode(" ",$output_ingest) ."\n");
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

            if (count($this->pid) != $this->node['nfiles']) {
                $message = 'Ingest of bundle only partially succeeded. Check paths to resources in CMDI file or existing FOXML objects with same PID, or maybe new files have been added after cmdi creation';
                if (LOG_ERRORS) error_log ( date(DATE_RSS) . ";\t" . $message . ";\t". implode(" ",$this->pid) ."\n", $message_type = 3 , ERROR_LOG_FILE );
                throw new IngestServiceException ($message);


            }
        }

    }
    public function deleteFedoraObjects(){

        // create object that can do ReST requests
        $accessFedora = get_configuration_fedora();
        $rest_fedora = new FedoraRESTAPI($accessFedora);


        foreach ($this->pid as $pid){
            $rest_fedora->deleteObject($pid);
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
            'ownerId' => $this->node['user']);

        foreach ($this->pid as $pid) {
            $result = $rest_fedora->modifyObject($pid, $data);

            if (!$result) {$errors_occurred++;}
        }

        // rollback
        if ($errors_occurred > 0){
            $message = 'Couldn\'t change ownership of files';
            if (LOG_ERRORS) error_log ( date(DATE_RSS) . ";\t" . $message . ";\n", $message_type = 3 , ERROR_LOG_FILE );
            throw new IngestServiceException ($message);
        }
    }

    public function deleteOriginal(){
        # delete original
        recursiveRmDir($this->backend_bundle_dir);
        rmdir ($this->backend_bundle_dir);
    }



    /**
     * Updates a node field value; a new row is inserted if no row exists
     *
     * @param $db database connection handle
     */
     public function update_field ($field, $value){

        $conditions= array("entity_id" => $this->node['nid'],);
        $fields= array("upload_" . $field . "_value"  => $value);
        
        // update field
        $res = pg_update($this->db_connection, 'field_data_upload_' . $field, $fields, $conditions);
        if (!$res) {
            echo ( "field_data_upload_$field has not been updated\n");
        }

        // update revision
        $res = pg_update($this->db_connection, 'field_revision_upload_' . $field, $fields, $conditions);
        if (!$res) {
            echo ( "field_revision_upload_$field has not been updated\n");
        }

        // insert exception if it doesn't exist
         if ($field == 'exception'){
             $filtered_value = filter_var($value, FILTER_SANITIZE_STRING);
             $query = "INSERT INTO field_data_upload_$field (entity_type, bundle, deleted, entity_id, revision_id, language, delta, upload_exception_value, upload_exception_format) ";
             $query .= sprintf("SELECT 'node', 'flat_bundle', 0, %s , %s, 'und', 0, '$filtered_value', NULL  WHERE NOT EXISTS (SELECT 1 FROM field_data_upload_$field WHERE entity_id=%s)", $this->node['nid'], $this->node['nid'] , $this->node['nid']);

             $res = pg_exec($this->db_connection, $query);
             if (!$res) {
                 echo ( "Extra row has not been inserted into $field\n");
             }
         }
          // clear cached field
        $cache_row = array("cid"=>'field:node:'.$this->node['nid']);
        $res = pg_delete($this->db_connection, 'cache_field', $cache_row);
        if (!$res) {
            echo ( "Database table has not been updated\n");
        }

    }


    public function deleteBagMeta()
    {
        // delete bag metafiles
        $bagfiles = array('bag-info', 'bagit', 'manifest-md5', 'tagmanifest-md5');
        foreach ($bagfiles as $f) {
            $fname = $this->backend_bundle_dir . '/' . $f . ".txt";
            if (file_exists($fname)) {
                unlink($fname);
            }
        }
    }


    public function deleteZipfile()
    {
        // delete zip file
        $zip = $this->backend_bundle_dir . '/../' . $this->node['bundle'] . ".zip";
        if (file_exists($zip)) {
            unlink($zip);
        }
    }

    public function deleteBagDirectory()
    {
        // delete bag directory
        $bag_full_path = BAG_DIR . '/' . $this->bag['bag_id'];
        if (file_exists($bag_full_path && $this->bag['bag_id'])) {
            recursiveRmDir(BAG_DIR . '/' . $this->bag['bag_id']);
            rmdir(BAG_DIR . '/' . $this->bag['bag_id']);
        }
    }


    public function deleteFOs()
    {
        // delete already ingested Objects
        if ($this->pid) {
            $this->deleteFedoraObjects();
        }
    }


    public function create_blog_entry (){
        $bundle = $this->node['bundle'];
        $collection = $this->node['collection'];
        $new_node = new stdClass();
        $new_node->type = 'blog';
        $new_node->language = 'und';
        $new_node->title = sprintf("Archiving Report for bundle %s",$bundle);
        $new_node->uid = $this->node['uid'];
        $new_node->status = 1;
        $new_node->sticky = 0;
        $new_node->promote = 1;
        $new_node->format = 3;
        $new_node->revision = 0;
        $new_node->body['und'][0]['format'] = 'full_html';
        $new_node->body['und'][0]['summary'] = sprintf("<p>Archiving of %s completed</p>",$bundle, $collection,$bundle, $collection);;
        $new_node->body['und'][0]['value'] = sprintf("<p>%s %s</p><p>Archiving of %s belonging to %s succeeded.</p>",$bundle, $collection,$bundle, $collection);

        node_save($new_node);
    }


    public function rollback()
    {
        $this->deleteBagMeta();
        $this->deleteZipfile();
        $this->deleteBagDirectory();
        $this->deleteFOs();
    }

    public function cleanup(){
        $this->deleteBagMeta();
        $this->deleteZipfile();
        $this->deleteBagDirectory();
        $this->deleteOriginal();
    }

}