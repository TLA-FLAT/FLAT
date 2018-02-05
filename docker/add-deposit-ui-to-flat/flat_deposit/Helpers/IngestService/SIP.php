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

    // configuration parameters necessary for successful processing of a bundle
    protected $info;


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

        $this->owner = $owner;

        $this->sipId = get_class($this) . '_'. $uuid;

        $this->logProcess = variable_get('flat_deposit_ingest_service')['log_errors'];

        $base_name = variable_get('flat_deposit_ingest_service')['error_log_file'];

        // put feedback on screen if logging is set and error_log dir is not accessible
        if ((!file_exists($base_name) OR !is_writable($base_name)) AND $this->logProcess){
            drupal_set_message('Unable to write log file to log directory','warning');
        };

        $this->logFile = $base_name . "/" . $this->sipId . '_' . $owner . '_' . date("Y-m-d\TH-i-s") . '.log';

        $this->logging(t("Starting SIP constructor with following parameters: \nowner: !owner\ncmdi file: !cmdiFileName\nparent Fid: !parentFid\ntest: !test\n", array(
            '!owner'=> $owner,
            '!cmdiFileName' => $cmdiFileName,
            '!parentFid' => $parentFid,
            '!test' => ($test ? 'TRUE' : 'FALSE'),
                    )
            )
        );

        $this->cmdiRecord =$cmdiFileName;

        $this->parentFid = $parentFid;

        $this->test = $test;

        $this->frozenSipDir = drupal_realpath('freeze://') . '/SIPS/' .  str_replace('@', '_at_', $this->owner) . '/' . $this->sipId . '/';

        $this->cmdiTarget = $this->frozenSipDir .  '/data/metadata/record.cmdi';

        $this->logging('Finishing SIP constructor');

    }

    /**
     * @return mixed
     */
    public function getFid()
    {
        return $this->fid;
    }

    /**
     * @return mixed
     */
    public function getSipId()
    {
        return (string)$this->sipId;
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
        $this->logging('Starting copyMetadata');

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

        $this->logging('Finishing copyMetadata');
        return TRUE;

    }


    function addIsPartOfProperty(){

        $this->logging('Starting addIsPartOfProperty');
        $parentFid = $this->parentFid;
        $file_name = $this->cmdiTarget;

        module_load_include('php','flat_deposit','/Helpers/CMDI/CmdiHandler');
        $cmdi = simplexml_load_file($file_name, 'CmdiHandler');
        if (is_string($cmdi)){
            throw new IngestServiceException($cmdi);
        }
        $cmdi->addIsPartOfProperty($parentFid);
        $check = $cmdi->asXML($file_name);
        if ($check !== TRUE){
            throw new IngestServiceException($check);
        }

        $this->logging('Finishing addIsPartOfProperty');
        return TRUE;
    }


    function generatePolicy()
    {
        $this->logging('Starting generatePolicy');
        $policy = $this->info['policy'];
        $fname = drupal_get_path('module','flat_deposit') . '/Helpers/IngestService/Policies/' . $policy . '.n3';

        $string = file_get_contents($fname);
        $new_string = preg_replace('/ACCOUNT_NAME/', $this->owner , $string);

        $cmdi_dir = dirname($this->cmdiTarget);
        $write = file_put_contents( $cmdi_dir . '/policy.n3', $new_string);

        if (!$write) {
            throw new IngestServiceException('Unable to write policy to target location (' . $cmdi_dir . ')');
        }
        $this->logging('Finishing generatePolicy');
        return TRUE;
    }


    function createBag()
    {
        $this->logging('Starting createBag');

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

        $this->logging('Finishing createBag');
        return TRUE;
    }

    function doSword()
    {
        $this->logging('Starting doSword');
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
            $this->logging('Finishing doSword');
            return TRUE;

        }

    }

    function doDoorkeeper()
    {
        $this->logging('Starting doDoorkeeper');

        $query = $this->test ? 'validate%20resources' : '';


        module_load_include('php', 'flat_deposit', '/Helpers/IngestService/Doorkeeper');
        $dk = new Doorkeeper();
        $dk->triggerServlet($this->sipId, $query);
        $fid = $dk->checkStatus($this->sipId, 120);

        $this->fid =$fid ;

        $this->logging('Finishing doDoorkeeper');

        return TRUE;
    }


    /**
     * @param array $message
     *
     * @return bool
     */
    function rollback($message)
    {
        $this->logging('Starting rollback');

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

        $this->logging('Finishing rollback');

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

        $basePath = variable_get('flat_deposit_ingest_service')['bag_dir'];
        $bagDir = $basePath . $this->sipId;
        module_load_include('inc', 'flat_deposit', 'inc/class.FlatBundle');
        FlatBundle::recursiveRmDir($bagDir);
        rmdir($bagDir);

    }

    
    public function logging($message){

        if ($this->logProcess){

            error_log ( date(DATE_ATOM) . "\t" . $message ."\n", $message_type = 3 , $this->logFile );

        }


    }

}