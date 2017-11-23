<?php

include_once('SIP.php');

class Collection extends SIP
{


    /**
     * Set up function for collection ingest; also validates provided input.
     *
     * @param array $info  array of configuration parameters necessary for successful processing of a collection
     *
     * The array requires following parameters:
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
            'fid',
            'policy',
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
     * @return mixed
     */
    function addResourcesToCmdi()
    {

        $this->logging('Starting addResourcesToCmdi');

        module_load_include('php','flat_deposit','/Helpers/CMDI/CmdiHandler');

        $file_name = $this->cmdiTarget;
        $xml = CmdiHandler::loadXml($file_name);
        if (is_string($xml)){
            throw new IngestServiceException($xml);
        }

        if (!isset($xml->Header->MdProfile)){
            throw new IngestServiceException('Element MdProfile in Cmdi Header is not set');
        }


        $id = (string)$xml->Header->MdProfile;
        $profile = CmdiHandler::getNameById($id);

        if ($this->info['fid'] AND $profile == 'MPI_Bundle') {
            try {

                CmdiHandler::addResourcesFromDatastream($xml, $this->info['fid']);

            } catch (CmdiHandlerException $exception) {

                throw new IngestServiceException($exception->getMessage());

            }
        }

        $check = $xml->asXML($file_name);
        if ($check !== TRUE){
            throw new IngestServiceException($check);
        }
        #$check = $xml->asXML('/lat/test.xml');

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
}