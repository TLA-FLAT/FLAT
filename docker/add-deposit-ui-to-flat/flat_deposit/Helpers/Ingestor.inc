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

    public $nResources;

    public $finished_actions;

    public $doorkeeper_query;

    public $parent_id;

    /*
     * Constructor function for Ingestor class. Needs array of node data
     *
     * @param $data row data from drupal node table
     * @param $db database connection
     */
    public function __construct($nid)
    {

        $this->node = node_load($nid);
        $this->wrapper = entity_metadata_wrapper('node', $this->node);

        $this->user = user_load($this->node->uid);

        $this->finished_actions = array();

        // Determine processing routine
        $status = $this->wrapper->upload_status->value();
        if ($status == 'failed' OR $status == 'open') {

            $this->type = 'validating';

        } else {

            $this->type = 'processing';

        }

        // Determine paths
        $config = variable_get('flat_deposit_paths');
        $this->dir = array(
            'freeze' => $config['freeze'],
            'freeze_bundle' => $config['freeze'] . "/" . $this->user->name . "/" . $this->wrapper->upload_collection->value() . "/" . $this->node->title,
         );

        // source path of the bundle data and meta data

        $this->dir['origin_meta'] = 'private://flat_deposit/metadata/' . $this->user->name . "/" . $this->wrapper->upload_collection->value() . '/' . $this->node->title;
        $this->dir['freeze_meta'] = 'freeze://' . "/" . $this->user->name . "/" . $this->wrapper->upload_collection->value() . '/' . $this->node->title;

        if ($this->wrapper->upload_external->value()) {

            $config = variable_get('flat_deposit_paths');
            $this->dir['origin_data'] = $config['alternate_dir'] . $this->user->name . $config['alternate_subdir'] . "/" . $this->wrapper->upload_external->value();

        } else {

            $this->dir['origin_data'] = drupal_realpath('private://flat_deposit/data/' . $this->user->name . "/" . $this->wrapper->upload_collection->value() . '/' . $this->node->title);

        }


        // settings of bag
        $this->bag = array();

        $this->bag ['bag_id'] = $this->user->name . "_" . $this->node->nid . "_" . $this->node->title . '_' . uniqid();
        $this->bag ['temp_extension'] = 'temp';
        $this->bag ['temp_folder'] = $this->dir['freeze_bundle'] . '/' . $this->bag ['temp_extension'];

        // log settings
        $config = variable_get('flat_deposit_ingest_service');
        $this->log = $config['log_all'];

        $dir = $config['log_file_dir'] ? $config['log_file_dir'] : './';
        $this->logfile = $dir . '/Ingest_service_log_' . date("j.n.Y_g.i") . '.txt';

        $this->parent_id = $this->getParentID()['collection_pid'];

    }


    /**
     * Method for getting the pid of the parent collection.
     *
     * @throws IngestServiceException
     */
    public function getParentID(){

        $collection = $this->wrapper->upload_collection->value();

        // retrieve pid of parent collection
        $collection_node = db_select('flat_collection','p')
            ->fields('p',array('collection_name','collection_pid'))
            ->condition('member', $this->user->name)
            ->condition('collection_name', $collection)
            ->execute()
            ->fetchAll(PDO::FETCH_ASSOC)[0];

        if (!$collection_node){
            throw new IngestServiceException ('No entry found for user in flat_collection table. Contact administrator.');
        } else {

            return $collection_node;
        }
    }


    /**
     * Helper Method for adding an additional string text to the logfile
     * @param $entry
     */
    public function AddEntryLogFile($entry) {

        if ($this->log) {

            file_put_contents($this->logfile, $entry . PHP_EOL, FILE_APPEND);
        }
    }

    /**
     * Function checking whether status of the flat_bundle with the nid is set to processing or validating
     * @throws IngestServiceException
     *
     */
    public function validateNodeStatus()
    {

        $status = $this->wrapper->upload_status->value();

        if ($status == 'processing' OR $status == 'validating'){
            throw new IngestServiceException(t('Invalid node status'));
        }
    }


    /**
     * Method for updating the node status
     * @param $status
     */
    public function updateNodeStatus($status)
    {

        $this->wrapper->upload_status->set($status);
        $this->wrapper->save();

        $this->AddEntryLogFile('set status to ' . $status);
    }



    public function validate_backend_directory()
    {

            $this->AddEntryLogFile(t('Validating backend (!freeze) and source directory (!source)', array(
                '!freeze' => $this->dir['freeze_bundle'],
                '!source'=>$this->dir['origin_data'])));

            // Check if data source is accessible
            if (!is_writable($this->dir['origin_data'])) {

                    throw new IngestServiceException(t('The data source directory is not accessible (!dir))',array(
                        '!dir' => $this->dir['origin_data'])));
            }

            // Check if freeze directory exists
            if (!file_exists($this->dir['freeze'])) {
                throw new IngestServiceException("No backend freeze directory exists. Please contact administator");
            }

            // Check if freeze directory is owned by apache_user and writable
            if (!is_writable($this->dir['freeze'])) {
                throw new IngestServiceException("Freeze directory is not writable by application. Please contact administator");
            }

            // Check if user backend directory already exists
            if (!file_exists($this->dir['freeze_bundle'])) {
                throw new IngestServiceException('No freeze directory with the name ' . $this->node->title . ' at target location.');
            }


        }


    /**
     * Data freeze method moving the data from private data and metadata directories to the freeze location
     * @param $type
     * @throws IngestServiceException
     */
    public function moveData($type)    {


        $collection = $this->wrapper->upload_collection->value();
        $collection_directory = $this->dir['freeze'] . $this->user->name .  "/$collection/";

        $metadata_file_info =$this->wrapper->upload_cmdi->value();

        // Determine source and target
        if ($type == 'freeze'){

            $source_data = $this->dir['origin_data'];
            $target_data = $this->dir['freeze_bundle'] . '/data';

            $target_meta = $this->dir['freeze_meta'] . '/metadata';


        } else {

            $source_data = $this->dir['freeze_bundle'] . '/data';
            $target_data = $this->dir['origin_data'];

            $target_meta = $this->dir['origin_meta'];

        }

        $this->AddEntryLogFile(t('Moving data from source from %source to !target', array ('%source' => $source_data, '!target' => $target_data)));

        // move resources and throw exception on error
        $data_is_moved = rename($source_data, $target_data);

        if (!$data_is_moved) {

            throw new IngestServiceException(t("Data could not be moved from %source to !target", array ( '%source'=> $source_data, '!target' => $target_data)));

        }

        //keep track that files have been moved
        $this->finished_actions[] = 'moved';


        //move metadata and throw exception on error
        $file= file_load($metadata_file_info['fid']);

        $new_file = file_move($file,$target_meta, FILE_EXISTS_REPLACE);

        if (!$new_file) {

            throw new IngestServiceException(t("Metadata could not be moved to !target", array ('!target' => $target_meta)));

        }


    }

    /**
     * Method for loading an existing meta data file, and adapting resources. In case of CMDIs created by flat,
     * all resources are removed and subsequently added as localURI-attribute to the resourceproxylist.
     * Externally created cmdis will undergo a removal of the resourceproxy ResourceRef-value.
     * The local-URI attribute will undergo a transformation from relative paths to absolute paths.
     *
     * @throws IngestServiceException
     */
    public function adaptCMDIresources($method){

        module_load_include('php','flat_deposit','Helpers/CMDI_Handler');
        $cmd = new CMDI_Handler($this->node);

        if ($method=='ingest'){

            if ($cmd->flat_created_CMDI){

                $this->AddEntryLogFile('Adding resources to bundle info file');
                $cmd->remove_all_resources();
                $cmd->add_all_resources();
                $cmd->save_updated_xml();

                //keep track that files have been moved
                $this->finished_actions[] = 'adaptCMDI';


            } else {

                $this->AddEntryLogFile('Transform relative paths to absolute paths');

                try {
                    $cmd->remove_MdSelfLink();
                    $cmd->remove_resourceRef_value();
                    $cmd->resourceRel2FreezePathTransform();
                    $cmd->save_updated_xml();
                    //keep track that files have been moved
                    $this->finished_actions[] = 'adaptCMDI';


                } catch (CMDIHandlerException $e){

                    throw new IngestServiceException($e->getMessage());

                }
            }

        } elseif ($method=='rollback' AND is_int(array_search('adaptCMDI', $this->finished_actions))){


            if ($cmd->flat_created_CMDI){

                $this->AddEntryLogFile('Cleaning all resources from file');
                $cmd->remove_all_resources();
                $cmd->save_updated_xml();

            } else {
                $this->AddEntryLogFile('Transform relative paths to absolute paths');
                $cmd->ResourceFreeze2RelPathTransform();
                $cmd->save_updated_xml();
            }

        }

    }



    /**
     * Method for loading an existing meta data file, and adding the parent relationship to the CMDI, which can be used
     * later to be translated into rel-ext info of the corresponding foxml object. In foxml, rel-ext will be extended by
     * isMemberofCollection attribute, which will point to the pid of the parent collection.
     *
     * @throws IngestServiceException
     */
    public function addIsPartOfToCMDI($parent_id){

        module_load_include('php','flat_deposit','Helpers/CMDI_Handler');
        $cmd = new CMDI_Handler($this->node);

        $cmd->addIsPartOf($parent_id);
        $cmd->save_updated_xml();
    }


    public function removeIsPartOfFromCMDI($parent_id=NULL){

        module_load_include('php','flat_deposit','Helpers/CMDI_Handler');
        $cmd = new CMDI_Handler($this->node);

        $cmd->removeIsPartOf($parent_id);
        $cmd->save_updated_xml();
    }


    /**
     * make bag with data at right location
     *
     */

    public function prepareBag()
    {
        $this->AddEntryLogFile('Creating bag for sword');

        drupal_mkdir($this->bag['temp_folder'] . '/data', NULL, TRUE);

        $command = 'cp -R "' . $this->dir['freeze_bundle'] .  '/metadata" "' . $this->bag['temp_folder'] .  '/data"';
        exec($command, $output, $return);
        if ($return) {
            $message = 'Could not copy metadata to temporary bag directory';
            if (LOG_ERRORS) error_log ( date(DATE_RSS) . ";\t" . $message . ";\t". implode(" ",$output) ."\n", $message_type = 3 , ERROR_LOG_FILE );
            throw new IngestServiceException ($message . implode(";\n",$output));
        }

        $command = BAG_EXE . ' baginplace ' . '"' . $this->bag['temp_folder'] .  '"';
        exec($command, $output, $return);
        if ($return) {
            $message = 'Error making bag';
            if (LOG_ERRORS) error_log ( date(DATE_RSS) . ";\t" . $message . ";\t". implode(" ",$output) ."\n", $message_type = 3 , ERROR_LOG_FILE );
            throw new IngestServiceException ($message . implode(";\n",$output));
        }

        $command = BAG_EXE . ' update ' . '"' . $this->bag['temp_folder'] .  '"';

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
        $this->AddEntryLogFile('Zipping bag');

        $command = DEPOSIT_UI_PATH . '/Helpers/scripts/zip_sip.sh "' . $this->bag['temp_folder'] .'" ' . $this->node->title;
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
     *
     * @throws IngestServiceException
     */
    public function doSword()
    {

        $this->AddEntryLogFile('Do sword upload of bag with metadata');

        $cwd = getcwd();
        $zipname = $this->node->title . '.zip';
        $path = $this->dir['freeze_bundle'];

        chdir($path);


        $data = file_get_contents($zipname);
        $md5 = md5_file($zipname);

        $config = variable_get('flat_deposit_sword');
        $ch = curl_init();
        curl_setopt($ch, CURLOPT_URL, $config['url'].'collection'); //
        curl_setopt($ch, CURLOPT_PORT, $config['port']);
        curl_setopt($ch, CURLOPT_USERPWD, sprintf("%s:%s",$config['user'],$config['password']));
        curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, false);
        curl_setopt($ch, CURLOPT_SSL_VERIFYHOST, false);

        curl_setopt($ch, CURLOPT_RETURNTRANSFER, TRUE); // -i
        curl_setopt($ch, CURLOPT_BINARYTRANSFER, TRUE); // --data-binary
        curl_setopt($ch, CURLOPT_POST, 1);
        curl_setopt($ch, CURLOPT_POSTFIELDS, $data);

        curl_setopt($ch, CURLOPT_HEADER, TRUE); // -i
        curl_setopt($ch, CURLOPT_HTTPHEADER, array(
            'Content-Type: application/zip',
            'Content-Disposition: attachment; filename='.$zipname,
            'Content-MD5: '.$md5,
            'Packaging: http://purl.org/net/sword/package/BagIt',
            'Slug: '.$this->bag['bag_id'],
            'In-Progress: false'));

        $content = curl_exec($ch);
        $httpcode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
        curl_close($ch);

        chdir($cwd);

        if ($httpcode != 200 && $httpcode != 202 && $httpcode != 201) {
            $message = sprintf("SWORD Server error (HTTP error code (%d) ;\n", $httpcode) .  $content;;
            if (LOG_ERRORS) error_log(date(DATE_RSS) . ";\t" . $message, $message_type = 3, ERROR_LOG_FILE);
            throw new IngestServiceException ($message);
        }

    }


    /**
     * HTTP GET Request for SWORD REST api
     *
     * @param bool $code_only if true method returns only HTTP response code
     * @return mixed
     */
    public function swordGetRequest($code_only=FALSE)
    {
        $config = variable_get('flat_deposit_sword');
        $ch = curl_init();

        curl_setopt($ch, CURLOPT_URL, $config['url'] . 'statement/' .$this->bag['bag_id'] ); //
        curl_setopt($ch, CURLOPT_PORT, $config['port']);
        curl_setopt($ch, CURLOPT_USERPWD, sprintf("%s:%s",$config['user'],$config['password']));
        curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, false);
        curl_setopt($ch, CURLOPT_SSL_VERIFYHOST, false);

        curl_setopt($ch, CURLOPT_RETURNTRANSFER, TRUE); // -i

        $val = curl_exec($ch);
        $httpcode = curl_getinfo($ch, CURLINFO_HTTP_CODE);

        curl_close($ch);

        return $code_only ? $httpcode : $val;
    }


    /**
     * Create Bag at correct bag location
     *
     *
     * @throws IngestServiceException
     */

    public function checkStatusSword()
    {

        $this->AddEntryLogFile('Check status of sword service');

        #initial check request
        $val = $this->swordGetRequest();
        $xml =simplexml_load_string($val) ;
        $status = (string)$xml->category['term'];

        // loop and wait until SWORD signals end of request
        while ($status == 'FINALIZING') {
            sleep(10);
            $val = $this->swordGetRequest();
            $xml =simplexml_load_string($val) ;
            $status = (string)$xml->category['term'];
        };

        // check outcome SWORD
        if ($status != 'SUBMITTED') {
            $message = "Error creating bag;\n" .  $val;
            if (LOG_ERRORS) error_log(date(DATE_RSS) . ";\t" . $message, $message_type = 3, ERROR_LOG_FILE);
            throw new IngestServiceException ($message);
        }
    }

    /**
     * This methods triggest the doorkeeper servlet to create FOXML files for the CMDI and all resources and ingest them.
     *
     * triggering is achieved by executing a curl 'put' response to the service IP
     *
     * @throws IngestServiceException
     */
    public function triggerDoorkeeper($to=FALSE){

        $this->AddEntryLogFile('Trigger doorkeeper');

        if($to){
           $query = '?to=' . $to ;
        } else {
            $query = '';
        }

        $config = variable_get('flat_deposit_doorkeeper');
        $ch = curl_init();
        curl_setopt($ch, CURLOPT_URL, $config['url'] . $this->bag['bag_id'] . $query);
        curl_setopt($ch, CURLOPT_PORT, $config['port']);
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
        curl_setopt($ch, CURLOPT_HTTPAUTH, CURLAUTH_ANY);
        curl_setopt($ch, CURLOPT_PUT, 1);

        $val = curl_exec($ch);

        $httpcode = curl_getinfo($ch, CURLINFO_HTTP_CODE);

        curl_close($ch);

        if ($httpcode != 200 && $httpcode != 202 ) {
            $message = 'Error triggering doorkeeper' . $val;
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
    public function doorkeeperGetRequest($code_only=FALSE)
    {

        $config = variable_get('flat_deposit_doorkeeper');
        $ch = curl_init();
        curl_setopt($ch, CURLOPT_URL, $config['url'] . $this->bag['bag_id']);
        curl_setopt($ch, CURLOPT_PORT, $config['port']);
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
        curl_setopt($ch, CURLOPT_HTTPAUTH, CURLAUTH_ANY);

        $val = curl_exec($ch);
        $httpcode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
        curl_close($ch);

        $return = $code_only ? $httpcode : $val;
        return $return;
    }


    /**
     * Method requesting every 10 seconds doorkeeper status
     *
     * toDO check return value of doorkeeper
     */
    public function checkStatusDoorkeeper(){

        $this->AddEntryLogFile('Check status of doorkeeper');

        #initial check request
        $httpcode = $this->doorkeeperGetRequest(TRUE);
        $max_wait = 60;
        $time = 0;

        // loop and wait until doorkeeper signals end of request
        while ($httpcode != 200 && $time <= $max_wait) {
            sleep(10);
            $time = $time + 10;
            $httpcode = $this->doorkeeperGetRequest(TRUE);
        };

        // Check time criterion
        if ($time >= $max_wait){
            $message = "Max time of $max_wait seconds exceeded; Doorkeeper does not finish";
            if (LOG_ERRORS) error_log ( date(DATE_RSS) . ";\t" . $message , $message_type = 3 , ERROR_LOG_FILE );
            throw new IngestServiceException ($message);
        }

        // check outcome doorkeeper
        $val = $this->doorkeeperGetRequest();
        $xml =simplexml_load_string($val) ;


        if ($xml['status'] != 'succeeded'){
            $message = 'Doorkeeper error' . $val;
            if (LOG_ERRORS) error_log(date(DATE_RSS) . ";\t" . $message . ";\n", $message_type = 3, ERROR_LOG_FILE);
            throw new IngestServiceException ($message);
        } else {
            $this->pid['bundle'] = $xml['fid'];

        }

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

        $this->AddEntryLogFile('Removing children of erronous ingested info file');

        // create object that can do ReST requests
        module_load_include('inc','flat_deposit', '/Helpers/Fedora_REST_API');

        $accessFedora = variable_get('flat_deposit_fedora');
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

        $this->AddEntryLogFile('Changing ownerID of ingested objects');

        // create object that can do ReST requests
        module_load_include('inc','flat_deposit', '/Helpers/Fedora_REST_API');

        $accessFedora = variable_get('flat_deposit_fedora');
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
        module_load_include('php','flat_deposit', '/inc/php_functions');
        recursiveRmDir($this->dir['freeze_bundle']);
        rmdir ($this->dir['freeze_bundle']);

        $collection = $this->wrapper->upload_collection->value();
        $collection_directory = $this->dir['freeze'] . $this->user->name .  "/$collection/";
        if (is_dir_empty($collection_directory)){

            rmdir($collection_directory);

        }

    }



    public function deleteBagMeta()
    {
        // delete bag metafiles
        $bagfiles = array('bag-info', 'bagit', 'manifest-md5', 'tagmanifest-md5');
        foreach ($bagfiles as $f) {
            $fname = $this->dir['freeze_bundle'] . '/' . $f . ".txt";
            if (file_exists($fname)) {
                unlink($fname);
            }
        }
    }

    public function deleteTempBag()
    {
        $dir = $this->bag['temp_folder'];
        if (file_exists($dir)){
            module_load_include('php','flat_deposit', '/inc/php_functions');
            recursiveRmDir($dir);
            rmdir ($dir);
        }
    }


    public function deleteZipfile()
    {
        // delete zip file

        $zip = $this->dir['freeze_bundle'] . '/' . $this->node->title . ".zip";
        if (file_exists($zip)) {
            unlink($zip);
        }
    }

    public function deleteBagDirectory()
    {
        $tmp_full_path = SWORD_TMP_DIR . '/' . $this->bag['bag_id'];
        $bag_full_path = BAG_DIR . '/' . $this->bag['bag_id'];
        $dirs = array($tmp_full_path, $bag_full_path);

        foreach ($dirs as $dir){
            shell_exec(sprintf('sudo chown -R %s:%s %s', APACHE_USER , APACHE_USER, $dir));
            if (file_exists($dir)) {
                module_load_include('php','flat_deposit', '/inc/php_functions');
                recursiveRmDir($dir);
                rmdir($dir);
            }
        }

    }


    public function deleteFOs()
    {
        // delete already ingested Objects
        if ($this->pid['obj']) {$this->deleteFedoraObjects();}
        if ($this->pid['bundle']) {$this->deleteFedoraObjects();}
    }


    public function create_blog_entry ($entry_type){

        $this->AddEntryLogFile('Creating blog entry');

        $host = variable_get('flat_deposit_ingest_service')['host_name'];
        if ($entry_type == 'succeeded'){

            $bdl = $this->pid['bundle'];
            $bdl = preg_replace('/[^a-zA-Z0-9_:]/u', '_', $bdl);

            $url_link = 'http://' . $host . '/flat/islandora/object/' . $bdl ;

        } elseif ($entry_type == 'failed'){

            $url_link = 'node/' . $this->node->nid;

        }

        $bundle = $this->node->title;
        $collection = $this->wrapper->upload_collection->value();
        $link_node = array(
            'theme' => 'link',
            'text' => 'here',
            'path' => $url_link,
            'options' => array(
                'attributes' => array(),
                'html' => FALSE
            ));

        $summary = sprintf("<p>Archiving of %s %s</p>",$bundle, $entry_type);
        $body = sprintf("<p>%s %s</p><p>Archiving of %s belonging to %s %s. Check ". theme_link($link_node) . '</p>',$bundle, $collection,$bundle, $collection, $entry_type);
        $body = preg_replace(array('/lat_/') , array('lat%3A'), $body);

        $new_node = new stdClass();
        $new_node->type = 'blog';
        $new_node->language = 'und';
        $new_node->title = sprintf("Result of archiving attempt for bundle %s",$bundle);
        $new_node->uid = $this->user->uid;
        $new_node->status = 1;
        $new_node->sticky = 0;
        $new_node->promote = 0;
        $new_node->format = 3;
        $new_node->revision = 0;
        $new_node->body['und'][0]['format'] = 'full_html';
        $new_node->body['und'][0]['summary'] = $summary;
        $new_node->body['und'][0]['value'] = $body;
        node_save($new_node);
    }


    public function rollback()
    {
        $this->AddEntryLogFile('Rollback');


        $this->cleanup();

        $this->deleteFOs();

        if (is_int(array_search('moved', $this->finished_actions))){
            $this->moveData('unfreeze');
        }

    }

    public function cleanup(){
        /*
        $this->deleteTempBag();
        $this->deleteZipfile();
        $this->deleteBagDirectory();
        #$this->deleteOriginal();
        */
    }


    public function finalizeProcessing($nid){
        if($this->type== 'validating'){

            $this->updateNodeStatus('valid');
            $this->wrapper->upload_exception->set(NULL);
            $this->wrapper->save();

        } else {

            $this->create_blog_entry('succeeded');

            node_delete_multiple(array($nid));

        }
    }

    #$ingest->wrapper->upload_status->set('awaiting');
    #$ingest->wrapper->save();


}