<?php

class IngestServiceException extends Exception {}

/**
 * Interface SIP
 */
abstract class SIP
{
    // owner of the SIP
    protected $owner;

    // name or id for a ingest SIP
    protected $sipId;

    // Temp directory where SIP is being stored
    protected $sipDir;


    // full filename of the original cmdi record file
    protected $cmdiRecord;

    // FID of the parent fedora Object
    protected $parentFid;

    // boolean indicating whether ingest should be completed or only simulated
    protected $test;


    // frozen data directory to be bagged and ingested
    protected $frozenSipDir;

    // fedora id assigned to a ingested SIP
    protected $fid;

    // full file name of copied cmdi file
    protected $cmdiTarget;

    // Custom ingest set up of a SIP child (Collection, bundle)
    abstract function init($info);

    abstract function authenticateUser();

    abstract function prepareSipData();

    abstract function addResourcesToCmdi();

    abstract function finish();

    abstract function customRollback($message);

    function __construct($owner, $cmdiFileName, $parentFid, $test)
    {
        $uuid = uniqid();

        $this->owner =$owner;

        $this->sipId = get_class($this) . '_'. $uuid;

        $this->cmdiRecord =$cmdiFileName;

        $this->parentFid = $parentFid;

        $this->test = $test;

        $this->frozenSipDir = drupal_realpath('freeze://') . '/SIPS/' .  $this->owner . '/' . $this->sipId . '/';

        $this->cmdiTarget = $this->frozenSipDir .  '/data/metadata/record.cmdi';

    }

    /**
     * @return mixed
     */
    public function getFid()
    {
        return $this->fid;
    }



    /**
     * Copies the specified record.cmdi to a temporary SIP directory
     *
     * @return bool
     *
     * @throws IngestServiceException
     */
    function copyMetadata()
    {

        $cmdi_source = $this->cmdiRecord;


        // create (if necessary) backend directory
        if (!file_exists(dirname($this->cmdiTarget))) drupal_mkdir(dirname($this->cmdiTarget), NULL, TRUE);

        copy($cmdi_source, $this->cmdiTarget);

        if (!file_exists($this->cmdiTarget)) {

            throw new IngestServiceException('Could not copy cmdi file to target location');

        }

        $resourceDir = dirname($this->cmdiTarget) . "/../resources";
        drupal_mkdir($resourceDir, NULL, TRUE);

        if (!file_exists($resourceDir)) {

                throw new IngestServiceException('Could not create resource directory at target location');
        }

        return TRUE;

    }


    function addIsPartOfProperty(){

        $parentFid = $this->parentFid;
        $file_name = $this->cmdiTarget;

        module_load_include('php','flat_deposit','/Helpers/CMDI/CmdiHandler');
        $xml = CmdiHandler::loadXml($file_name);
        if (is_string($xml)){
            throw new IngestServiceException($xml);
        }
        CmdiHandler::addIsPartOfProperty($xml , $parentFid);
        $check = $xml->asXML($file_name);
        if ($check !== TRUE){
            throw new IngestServiceException($check);
        }

        return TRUE;
    }


    function generatePolicy()
    {
        $policy = $this->info['policy'];
        $fname = drupal_get_path('module','flat_deposit') . '/Helpers/IngestService/Policies/' . $policy . '.n3';

        $string = file_get_contents($fname);
        $new_string = preg_replace('/ACCOUNT_NAME/', $this->owner , $string);

        $cmdi_dir = dirname($this->cmdiTarget);
        $write = file_put_contents( $cmdi_dir . '/policy.n3', $new_string);

        if (!$write) {
            throw new IngestServiceException('Unable to write policy to target location (' . $cmdi_dir . ')');
        }

        return TRUE;
    }


    function createBag()
    {

        $bagit_executable = variable_get('flat_deposit_ingest_service')['bag_exe'];

        $command = $bagit_executable . ' baginplace ' . '"' . $this->frozenSipDir .  '"';

        exec($command, $output, $return);

        if ($return)
        {
            $message = 'Error making bag';
            throw new IngestServiceException ($message);
        }

        $command = $bagit_executable . ' update ' . '"' . $this->frozenSipDir .  '"';

        exec($command, $output, $return);

        if ($return)
        {
            $message = 'Error updating bag info';
            throw new IngestServiceException ($message);
        }

        $command = DRUPAL_ROOT . '/'. drupal_get_path('module','flat_deposit') . '/Helpers/scripts/zip_sip.sh "' . $this->frozenSipDir .'" "' . $this->sipId .'"';

        exec($command, $output_prep, $return);

        if ($return) {
            $message = 'Error creating zip file;';
            throw new IngestServiceException ($message);
        }

        return TRUE;
    }

    function doSword()
    {
        $zipName = $this->sipId . '.zip';
        $path = dirname($this->frozenSipDir);

        $sipId = $this->sipId;

        module_load_include('php', 'flat_deposit', '/Helpers/IngestService/Sword');

        $sword = new Sword();
        $upload = $sword->postSip($path, $zipName, $sipId);
        $check = $sword->checkStatus($sipId);

        if (!$upload OR !$check){

            $message = 'Error Doing sword';
            throw new IngestServiceException ($message);

        } else {

            return TRUE;

        }

    }


    function doDoorkeeper()
    {
        $query = $this->test ? 'validate%20resources' : '';


        module_load_include('php', 'flat_deposit', '/Helpers/IngestService/Doorkeeper');
        $dk = new Doorkeeper();
        $dk->triggerServlet($this->sipId, $query);
        $fid = $dk->checkStatus($this->sipId, 15);

        $this->fid =$fid ;

        return TRUE;
    }

    public function validateDirectories($directories){

        $value = TRUE;

        foreach ($directories as $directory) {
            if (!file_exists($directory)) {
                $value = FALSE;
                break;
            }
            if (!is_writable($directory)) {
                $value = FALSE;
                break;
            }
        }
        return $value;
    }



    /**
     * Call to change ownerID of fedora objects using the Fedora REST api.
     *
     * @throws IngestServiceException
     */
    function doFedora()
    {

        if ($this->test){

            return TRUE;

        }

        // create object that can do ReST requests
        module_load_include('inc','flat_deposit', '/Helpers/Fedora_REST_API');

        $accessFedora = variable_get('flat_deposit_fedora');
        $rest_fedora = new FedoraRESTAPI($accessFedora);


        // Change ownership of ingested files
        $data = array(
            'ownerId' => $this->owner,
        );

        $result = $rest_fedora->modifyObject($this->fid, $data);

        if (!$result) {
            $message = 'Couldn\'t change ownership of files';


            throw new IngestServiceException ($message);
        }

        return TRUE;

    }



    /**
     * @param array $message
     *
     * @return bool
     */
    function rollback($message)
    {
        if (file_exists($this->frozenSipDir)){
         #   $this->removeFrozenZipDir();
        }

        if (file_exists(dirname($this->frozenSipDir) . '/' . $this->sipId . '.zip')){
        #    $this->removeFrozenZipDir();
        }
        /*if ($processes['doSword']){
            $this->removeSwordBag();
        }
        */

        if($this->fid){
            #$this->removeIngestedObject();
        }

        $this->customRollback($message);

        return TRUE;
    }


    protected function removeFrozenZipDir(){

        // remove directory with SIP data
        $sip_dir = $this->frozenSipDir;
        module_load_include('inc', 'flat_deposit', 'inc/class.FlatBundle');

        if (file_exists($sip_dir)){
            FlatBundle::recursiveRmDir($sip_dir);
            rmdir($sip_dir);
        }
    }

    protected function removeSipZip()
    {
        // remove zipped SIP directory
        drupal_unlink(dirname($this->frozenSipDir) . '/' . $this->sipId . '.zip');

    }


    protected function removeSwordBag(){

        $basePath = variable_get('flat_deposit_paths')['bag'];
        $bagDir = $basePath . $this->sipId;
        recursiveRmDir($bagDir);
        rmdir($bagDir);

    }

    protected function removeIngestedObject(){

        module_load_include('inc','flat_deposit', '/Helpers/Fedora_REST_API');

        $accessFedora = variable_get('flat_deposit_fedora');
        $rest_fedora = new FedoraRESTAPI($accessFedora);

        $rest_fedora->deleteObject($this->fid);
    }

}