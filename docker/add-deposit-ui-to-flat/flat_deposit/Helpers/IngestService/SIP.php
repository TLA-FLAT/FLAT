<?php

class IngestServiceException extends Exception {}

/**
 * Interface SIP
 */
abstract class SIP
{
    // owner of the SIP
    protected $owner;

    // full filename of the cmdi record file
    protected $cmdiRecord;

    // original directory where resources can be found
    protected $resourceDirectory;

    // FID of the parent fedora Object
    protected $parentFid;

    // boolean indicating whether ingest should be completed or only simulated
    protected $test;

    // name or id for a ingest SIP
    protected $sipId;

    // frozen data directory to be bagged and ingested
    protected $frozenSipDir;

    // fedora id assigned to a ingested SIP
    protected $fid;

    abstract function authenticateUser();

    abstract function setupIngest();

    abstract function setFrozenSipDir();

    abstract function prepareSipData();

    abstract function doFedora();

    abstract function finish();

    abstract function rollback(Array $processes);


    function __construct($owner, $cmdiFileName, $resourceDirectory, $parentFid, $test)
    {
        $this->setOwner($owner);
        $this->setCmdiRecord($cmdiFileName);
        $this->setResourceDirectory($resourceDirectory);
        $this->setParentFid($parentFid);
        $this->setTest($test);

        $uuid = uniqid();
        $this->setSipId(get_class($this) . '_'. $uuid);
        $this->setFrozenSipDir();
    }



    /**
     * @return mixed
     */
    public function getOwner()
    {
        return $this->owner;
    }

    /**
     * @param mixed $owner
     */
    public function setOwner($owner)
    {
        $this->owner = $owner;
    }

    /**
     * @return mixed
     */
    public function getCmdiRecord()
    {
        return $this->cmdiRecord;
    }

    /**
     * @param mixed $cmdiRecord
     */
    public function setCmdiRecord($cmdiRecord)
    {
        $this->cmdiRecord = $cmdiRecord;
    }

    /**
     * @return mixed
     */
    public function getResourceDirectory()
    {
        return $this->resourceDirectory;
    }

    /**
     * @param mixed $resourceDirectory
     */
    public function setResourceDirectory($resourceDirectory)
    {
        $this->resourceDirectory = $resourceDirectory;
    }

    /**
     * @return mixed
     */
    public function getParentFid()
    {
        return $this->parentFid;
    }

    /**
     * @param mixed $parentFid
     */
    public function setParentFid($parentFid)
    {
        $this->parentFid = $parentFid;
    }



    /**
     * @return mixed
     */
    public function getTest()
    {
        return $this->test;
    }

    /**
     * @param mixed $test
     */
    public function setTest($test)
    {
        $this->test = $test;
    }




    /**
     * @return mixed
     */
    public function getFrozenSipDir()
    {
        return $this->frozenSipDir;
    }


    /**
     * @return mixed
     */
    public function getSipId()
    {
        return $this->sipId;
    }

    /**
     * @param mixed $sipId
     */
    public function setSipId($sipId)
    {
        $this->sipId = $sipId;
    }

    /**
     * @return mixed
     */
    public function getFid()
    {
        return $this->fid;
    }

    /**
     * @param mixed $fid
     */
    public function setFid($fid)
    {
        $this->fid = $fid;
    }



    function prepareMetaData(){

        $parentFid = $this->getParentFid();
        $file_name = $this->getCmdiRecord();

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



    function createBag()
    {

        $bagit_executable = variable_get('flat_deposit_ingest_service')['bag_exe'];

        $command = $bagit_executable . ' baginplace ' . '"' . $this->getFrozenSipDir() .  '"';

        exec($command, $output, $return);

        if ($return)
        {
            $message = 'Error making bag';
            throw new IngestServiceException ($message);
        }

        $command = $bagit_executable . ' update ' . '"' . $this->getFrozenSipDir() .  '"';

        exec($command, $output, $return);

        if ($return)
        {
            $message = 'Error updating bag info';
            throw new IngestServiceException ($message);
        }

        $command = DRUPAL_ROOT . '/'. drupal_get_path('module','flat_deposit') . '/Helpers/scripts/zip_sip.sh "' . $this->getFrozenSipDir() .'" "' . $this->getSipId() .'"';

        exec($command, $output_prep, $return);

        if ($return) {
            $message = 'Error creating zip file;';
            throw new IngestServiceException ($message);
        }

        return TRUE;
    }

    function doSword()
    {
        $zipName = $this->getSipId() . '.zip';
        $path = dirname($this->getFrozenSipDir());

        $sipId = $this->getSipId();

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
        $query = $this->getTest() ? 'validate%20resources' : '';


        module_load_include('php', 'flat_deposit', '/Helpers/IngestService/Doorkeeper');
        $dk = new Doorkeeper();
        $dk->triggerServlet($this->getSipId(), $query);
        $fid = $dk->checkStatus($this->getSipId(), 15);

        $this->setFid($fid);

        return TRUE;
    }

    public function validateDirectories(array $directories){

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





    protected function removeFrozenZipDir(){

        // remove directory with SIP data
        $sip_dir = $this->getFrozenSipDir();
        module_load_include('php','flat_deposit', '/inc/php_functions');
        if (file_exists($sip_dir)){
            recursiveRmDir($sip_dir);
            rmdir($sip_dir);
        }
    }

    protected function removeSipZip()
    {
        // remove zipped SIP directory
        drupal_unlink(dirname($this->getFrozenSipDir()) . '/' . $this->getSipId() . '.zip');

    }

}