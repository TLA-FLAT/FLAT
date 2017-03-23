<?php

include_once('IngestFactoryCreator.php');
include_once('SIP.php');


class IngestFactory extends IngestFactoryCreator
{

    protected $factorySIP;


    protected function DoSipIngest(SIP $SIP)
    {

        try{

            $this->factorySIP = $SIP;
            $this->processLog['authenticateUser'] = $this->factorySIP->authenticateUser();
            $this->processLog['setupIngest'] = $this->factorySIP->setupIngest();
            $this->processLog['prepareMetaData'] = $this->factorySIP->prepareMetaData();
            $this->processLog['prepareSip'] = $this->factorySIP->prepareSipData();
            $this->processLog['createBag'] = $this->factorySIP->createBag();
            $this->processLog['doSword'] = $this->factorySIP->doSword();
            $this->processLog['doDoorkeeper'] = $this->factorySIP->doDoorkeeper();
            $this->processLog['doFedora'] = $this->factorySIP->doFedora();
            $this->processLog['finish'] = $this->factorySIP->finish();
/*

*/
            return $this->factorySIP->getFid();

        } catch (IngestServiceException $exception){

            $this->factorySIP->rollback($this->processLog);

            return ($exception->getMessage());
        }

    }


    public function RequestSipIngest($SIP){

        return $this->DoSipIngest($SIP);
    }

}
