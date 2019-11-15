<?php
module_load_include('php','flat_deposit','Helpers/IngestService/IngestFactory');
module_load_include('php','flat_deposit','Helpers/IngestService/SIP');
module_load_include('php','flat_deposit','Helpers/IngestService/Collection');
module_load_include('php','flat_deposit','Helpers/IngestService/Bundle');



class IngestClient
{
    private $IngestFactory;
    private $sipConcrete;


    /**
     * IngestClient constructor.
     *
     * @param String $sipClassName Name of SIP type class to be loaded (i.e. Collection or bundle)
     *
     * @param String $owner Owner of the SIP
     *
     * @param String $cmdiFileName fullname of the record.cmdi file
     *
     * @param String $parentFid Fedora ID of the parent to which SIP should be attached
     *
     * @param bool $test Should ingest be completed or only validated
     */
    public function __construct($sipClassName, $owner, $cmdiFileName, $parentFid, $test=FALSE, $namespace=NULL)
    {
        if (!$sipClassName OR !$owner OR !$cmdiFileName OR !$parentFid){
            throw new IngestServiceException('One or more required constructor parameters are not set.');
        }

        $this->IngestFactory = new IngestFactory();

        $this->sipConcrete = new $sipClassName($owner, $cmdiFileName, $parentFid, $test, $namespace);

    }

    /**
     * SIP ingest request to factory. For parallel (backend) processing a session id is needed for authentication
     *
     * @param $info array (optional) configuration settings for bundle ingest (see also {@link Bundle.php}).
     *
     * @return mixed
     */
    public function requestSipIngest($info = []){

        $retValue = $this->IngestFactory->RequestSipIngest($this->sipConcrete, $info);

        return $retValue;
    }

}




