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
     * @param String $originalPathResources absolute path where resources can be found
     *
     * @param String $parentFid Fedora ID of the parent to which SIP should be attached
     *
     * @param bool $test Should ingest be completed or only validated
     */
    public function __construct(String $sipClassName, String $owner, String $cmdiFileName, String $originalPathResources, String $parentFid, bool $test=FALSE)
    {
        $this->IngestFactory = new IngestFactory();

        $this->sipConcrete = new $sipClassName($owner, $cmdiFileName, $originalPathResources, $parentFid, $test);

    }

    /**
     * SIP ingest request to factory
     * @return mixed
     */
    public function requestSipIngest(){

        $retValue = $this->IngestFactory->RequestSipIngest($this->sipConcrete);

        return $retValue;
    }

}




