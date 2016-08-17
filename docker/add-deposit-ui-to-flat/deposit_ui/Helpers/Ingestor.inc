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

    public $node;

    public $user;

    public $bag;

    public $pid;
    /*
     * Constructor function for Ingestor class. Needs array of node data
     *
     * @param $data row data from drupal node table
     * @param $db database connection
     */
    public function __construct($nid)
    {

        $this->node = node_load($nid);

        $this->user = user_load($this->node->uid);

        $this->wrapper = entity_metadata_wrapper('node', $this->node);

        $this->backend_bundle_dir = FREEZE_DIR . "/" . $this->user->name. "/" . $this->wrapper->upload_collection->value() . "/" . $this->node->title;

        $this->bag = array(
            'bag_id'  => $this->user->uid . "_" . $this->node->nid);


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
        $command = DEPOSIT_UI_PATH . "/Helpers/scripts/zip_sip.sh \"$this->backend_bundle_dir\" " . $this->node->title;
        exec($command, $output_prep, $return);

        if ($return) {
            $message = 'Error creating zip file;';
            if (LOG_ERRORS) error_log ( date(DATE_RSS) . ";\t" . $message . ";\t". implode(" ",$output_prep) ."\n", $message_type = 3 , ERROR_LOG_FILE );
            throw new IngestServiceException ($message . implode(";\n", $output_prep). implode(";\n", $command));
        }
    }

    /**
     * Create Bag at correct bag location
     *
     * @throws IngestServiceException
     */

    public function doSword()
    {

        $command = "cd \"" . $this->backend_bundle_dir . "\"/../;" . SWORD_SCRIPT . " " . $this->node->title . ".zip " . $this->bag['bag_id'];
        exec($command, $output_bag, $return);

        // sleep because sword is slow
        sleep(15);

        if (!file_exists(BAG_DIR . '/' . $this->bag['bag_id'])) {
            $message = 'Error doing sword. Check bag log (deposit/sword/tmp/' . $this->bag['bag_id'] . '/deposit.properties)';
            if (LOG_ERRORS) error_log(date(DATE_RSS) . ";\t" . $message . ";\t" . implode(" ", $output_bag) . "\n", $message_type = 3, ERROR_LOG_FILE);
            throw new IngestServiceException ($message . implode(";\n", $output_bag) . implode(";\n", $command));
        }
    }

    /**
     * Create Bag at correct bag location
     *




     *
     * @throws IngestServiceException
     */

    public function doSword_php()
    {
        $path = '/app/flat/backend/admin/Dummy collection';
        chdir($path);
        $ch = curl_init();
        curl_setopt($ch, CURLOPT_URL, "https://localhost/easy-deposit/collection"); //
        curl_setopt($ch, CURLOPT_PORT, 8443);
        curl_setopt($ch, CURLOPT_USERPWD, "flat:sword"); //-u
        curl_setopt($ch, CURLOPT_HEADER, 1); // -i
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, true); // -i
        curl_setopt($ch, CURLOPT_BINARYTRANSFER, TRUE); // --data-binary

        curl_setopt($ch, CURLOPT_VERBOSE, 1); // -v
        curl_setopt($ch, CURLOPT_HTTPHEADER, ['Content-Type: application/zip']);
        curl_setopt($ch, CURLOPT_HTTPHEADER, ['Slug: 1_24']);
        curl_setopt($ch, CURLOPT_HTTPHEADER, ['Content-Disposition: attachment; filename=Test_29.zip']);
        curl_setopt($ch, CURLOPT_HTTPHEADER, ['Packaging: http://purl.org/net/sword/package/BagIt']);
        curl_setopt($ch, CURLOPT_HTTPHEADER, ['Content-MD5: f151cac3299be0e08a1be0bc1a4fd2ab']);
        curl_setopt($ch, CURLOPT_HTTPHEADER, ['In-Progress: false']);
        curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, false);

        #curl_setopt($ch, CURLOPT_POST, 1);
        #curl_setopt($ch, CURLOPT_POSTFIELDS, array('file' => '@' . 'Test_29.zip')) ;

        $val = curl_exec($ch);
        $httpcode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
        curl_close($ch);

        $return = $code_only ? $httpcode : $val;
        return $return;

            }


    /**
     * This methods triggest the doorkeeper servlet to create FOXML files for the CMDI and all resources and ingest them.
     *
     * triggering is achieved by executing a curl 'put' response to the service IP
     *
     * @throws IngestServiceException
     */
    public function triggerDoorkeeper(){

        $ch = curl_init();
        curl_setopt($ch, CURLOPT_URL, "http://localhost/flat/doorkeeper/" . $this->bag['bag_id']);
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
        curl_setopt($ch, CURLOPT_HTTPAUTH, CURLAUTH_ANY);
        curl_setopt($ch, CURLOPT_PORT, 8080);
        curl_setopt($ch, CURLOPT_PUT, 1);

        curl_exec($ch);
        $httpcode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
        curl_close($ch);

        if ($httpcode != 200 && $httpcode != 202 ) {
            $message = 'Error triggering doorleeper';
            if (LOG_ERRORS) error_log(date(DATE_RSS) . ";\t" . $message . ";\n", $message_type = 3, ERROR_LOG_FILE);
            throw new IngestServiceException ($message);
        }
    }


    /**
     * HTTP GET Request for doorkeeper REST api
     *
     * @param bool $code_only if true method returns only HTTP response code
     * @return mixed
     */
    public function getDoorkeeperRequest($code_only=FALSE)
    {
        $ch = curl_init();
        curl_setopt($ch, CURLOPT_URL, "http://localhost/flat/doorkeeper/" . $this->bag['bag_id']);
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
        curl_setopt($ch, CURLOPT_HTTPAUTH, CURLAUTH_ANY);
        curl_setopt($ch, CURLOPT_PORT, 8080);

        $val = curl_exec($ch);
        $httpcode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
        curl_close($ch);

        $return = $code_only ? $httpcode : $val;
        return $return;
    }


    /**
     * Method requesting every 10 seconds doorkeeper status
     */
    public function waitFinishDoorkeeper(){

        #initial check request
        $httpcode = $this->getDoorkeeperRequest(TRUE);
        $max_wait = 600;
        $time = 0;
        // loop and wait until doorkeeper signals end of request
        while ($httpcode != 200 && $time <= $max_wait) {
            sleep(10);
            $time = $time + 10;
            $httpcode = $this->getDoorkeeperRequest(TRUE);
        };

        if ($time >= $max_wait){
            $message = 'Doorkeeper does not finish';
            if (LOG_ERRORS) error_log ( date(DATE_RSS) . ";\t" . $message , $message_type = 3 , ERROR_LOG_FILE );
            throw new IngestServiceException ($message);
        }
    }


    /**
     * Method extracting the fedora ID (PID) from CMDI metadata file
     *
     *
     * @return bool|mixed in case CMDI file with MdSelfLink is found the fedora PID, otherwise FALSE
     */
    function getBundleFID(){

        $fname = BAG_DIR . $this->bag['bag_id'] . "/". $this->node->title . "/metadata/record.cmdi";

        if (file_exists($fname)) {
            $xml = simplexml_load_file($fname);
            $data_array = json_decode(json_encode((array)$xml));
            $pid_bundle = $data_array->Header->MdSelfLink ;
            $fid_bundle = preg_replace('/[^A-Za-z0-9]/', '_', $pid_bundle);
            $fid_bundle = str_replace(array('hdl_'),array('lat:'),$fid_bundle) ;

        }
        $this->pid['bundle'] = $fid_bundle ? $fid_bundle : [];
    }



    /**
     * Queries Fedoras Commons mulgara index for constituents of a bundles' fedora id (i.e fedora PID)
     *
     */
    public function getConstituentFIDs(){
        $FID = $this->pid['bundle'];
        $query = "PREFIX fm: <info:fedora/fedora-system:def/model#>
            PREFIX fv: <info:fedora/fedora-system:def/view#>
            PREFIX frel: <info:fedora/fedora-system:def/relations-external#>
            PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>
            
            SELECT ?pid ?label ?created
                FROM <#ri>
                WHERE {
        ?object <http://purl.org/dc/elements/1.1/identifier> ?pid;
                fm:state fm:Active;
                fm:label ?label;
                fm:createdDate ?created;
                fm:ownerId ?owner;
                frel:isConstituentOf <info:fedora/" . $FID . ">;
                }
            ORDER BY DESC(?created)";
        $tuque = islandora_get_tuque_connection();

        $results = $tuque->repository->ri->sparqlQuery($query);
        $constituents = !empty($results) ? array_column(array_column($results,'pid'),'value') : [];

        $this->pid['obj'] = $constituents;

    }


    public function deleteFedoraObjects(){

        // create object that can do ReST requests
        $accessFedora = get_configuration_fedora();
        $rest_fedora = new FedoraRESTAPI($accessFedora);

        if ($this->pid['bundle']){$rest_fedora->deleteObject($this->pid['bundle']);}
        foreach ($this->pid['obj'] as $pid){$rest_fedora->deleteObject($pid);}
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
            'ownerId' => $this->user->name);

        $result = $rest_fedora->modifyObject($this->pid['bundle'], $data);
        if (!$result) {$errors_occurred++;}

        foreach ($this->pid['obj'] as $pid) {
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
        $zip = $this->backend_bundle_dir . '/../' . $this->node->title . ".zip";
        if (file_exists($zip)) {
            unlink($zip);
        }
    }

    public function deleteBagDirectory()
    {
        // delete bag directory
        $bag_full_path = BAG_DIR . '/' . $this->bag['bag_id'];
        if (file_exists($bag_full_path) && $this->bag['bag_id']) {
            recursiveRmDir(BAG_DIR . '/' . $this->bag['bag_id']);
            rmdir(BAG_DIR . '/' . $this->bag['bag_id']);
        }
    }


    public function deleteFOs()
    {
        // delete already ingested Objects
        if ($this->pid['obj']) {$this->deleteFedoraObjects();}
        if ($this->pid['bundle']) {$this->deleteFedoraObjects();}
    }


    public function create_blog_entry (){
        $bundle = $this->node->title;
        $collection = $this->wrapper->upload_collection->value();
        $new_node = new stdClass();
        $new_node->type = 'blog';
        $new_node->language = 'und';
        $new_node->title = sprintf("Archiving Report for bundle %s",$bundle);
        $new_node->uid = $this->user->uid;
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