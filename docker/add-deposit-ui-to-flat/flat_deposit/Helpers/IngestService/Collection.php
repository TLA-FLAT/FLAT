<?php

include_once('SIP.php');

/**
 * Collection is responsible to ingest new/updated MPI_collections and updated MPI_BUndles into the fedora commons repository.
 *
 */
class Collection extends SIP
{

    // the node containing most of the important information
    protected $node;

    // the drupal entity_metadata_wrapper of this node
    protected $wrapper;

    /**
     * Set up function for collection ingest; also validates provided input.
     *
     * @param array $info  array of configuration parameters necessary for successful processing of a collection
     *
     * The array requires following parameters:
     *
     * 'content_type': as this variant is also used for updating bundle metadata, we need to know the content type.
     *
     * 'policy': which policy (ACL) to generate; either
     *
     * 'fid': whether the ingest fid is already known
     *
     * @return bool
     *
     * @throws IngestServiceException
     */
    public function init($info){

        $this->logging('Starting init');

        $required = array(
            'content_type',
            'fid',
            'policy',
            'visibility',
        );

        if (!isset($info['fid'])){$info['fid'] ='';}


        $diff = array_diff($required,array_keys($info));
        if($diff){

            throw new IngestServiceException('Not all required variables are defined. Following variables are missing: ' . implode(', ', $diff));

        };

        $this->info = $info;

        $this->logging('Finishing init');
        return TRUE;



    }



    /**
     * Collection permissions are handled by Fedora. In case of missing permissions the whole ingest form including
     * submission button is absent. Accordingly has permission been granted in case user click on submit.
     *
     * @return bool
     */
    public function authenticateUser()
    {
        return TRUE;

    }

    /**
     * Do nothing
     *
     * @return mixed
     */
    function prepareSipData()
    {
        return TRUE;
    }

    /**
     * Do nothing
     *
     * @return true
     */
    function validateResources(){

        return TRUE;
    }

    /**
     *
     * @return mixed
     */
    function addResourcesToCmdi()
    {

        $this->logging('Starting addResourcesToCmdi');

        module_load_include('inc', 'flat_deposit', 'Helpers/CMDI/class.CmdiHandler');

        $file_name = $this->cmdiTarget;

        $cmdi = CmdiHandler::simplexml_load_cmdi_file($file_name);


        if (!$cmdi OR !$cmdi->canBeValidated()){
            throw new IngestServiceException('Unable to load record.cmdi file');
        }

        try{

            $md_type = isset($this->wrapper->flat_cmdi_option) ? $this->wrapper->flat_cmdi_option->value() : NULL;
            switch ($md_type) {
                case 'new':
                    $cmdi->cleanMdSelfLink();
                    break;
                case 'import':
                case 'template': 
                    $cmdi->removeMdSelfLink();
                    break;
            }

        } catch (CmdiHandlerException $exception){

            throw new IngestServiceException($exception->getMessage());

        }
        
        /*
        $file_name = $this->cmdiTarget;
        module_load_include('inc', 'flat_deposit', 'Helpers/CMDI/class.CmdiHandler');
        $cmdi = simplexml_load_file($file_name, 'CmdiHandler');


        if ($this->info['fid'] AND $this->info['content_type'] == 'flat_bundle') {

            try{
                $fid = $this->info['fid'];

                $cmdi->addResources(null, $fid);

            } catch (CmdiHandlerException $exception){

                throw new IngestServiceException($exception->getMessage());

            }
        }

        $check = $cmdi->asXML($file_name);
        if ($check !== TRUE){
            throw new IngestServiceException($check);
        }
        #$check = $xml->asXML('/lat/test.xml');

*/
        $this->logging('Finishing addResourcesToCmdi');
        return TRUE;
    }






    function finish()
    {
        $this->logging('Starting finish');
        $this->removeFrozenZipDir();
        $this->removeSipZip();
        #$this->removeSwordBag();
        /*


                */
        $this->logging('Stop finish');
        return TRUE;

    }



    function customRollback($message){}
    function generateFlatEncryptionMetadata() {}
}
