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
     * @return bool
     *
     * @throws IngestServiceException
     */
    public function init($info){

        $required = array(
            'policy',
        );

        $diff = array_diff($required,array_keys($info));
        if($diff){

            throw new IngestServiceException('Not all required variables are defined. Following variables are missing: ' . implode(', ', $diff));

        };

        $this->info = $info;

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
     return TRUE;
    }



    function finish()
    {
        $this->removeFrozenZipDir();
        $this->removeSipZip();
        #$this->removeSwordBag();
        /*


                */
        return TRUE;

    }



    function customRollback($message){}
}