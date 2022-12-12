<?php

include_once('SIP.php');

class IngestFactory
{

    /**
     * @var SIP a SIP class with basic ingest functionality
     */
    protected $factorySIP;

    /**
     * @var array booleans tracking success on each processing step
     */
    protected $processLog;

    /**
     * @param SIP $SIP an instantiation of the SIP class
     *
     * @param array $info
     *
     * @return mixed|string
     *
     */
    protected function DoSipIngest($SIP, $info)
    {

        try {
            $this->factorySIP = $SIP;
            $this->processLog = [];
            $this->processLog['ingestInitiated'] = $this->factorySIP->init($info);
            $this->processLog['authenticateUser'] = $this->factorySIP->authenticateUser();
            $this->processLog['prepareSipData'] = $this->factorySIP->prepareSipData();
            $this->processLog['validateResources'] = $this->factorySIP->validateResources();
            $this->processLog['copyMetadata'] = $this->factorySIP->copyMetadata();
            $this->processLog['addIsPartOfProperty'] = $this->factorySIP->addIsPartOfProperty();
            $this->processLog['addResourcesToCmdi'] = $this->factorySIP->addResourcesToCmdi();
            //throw new IngestServiceException('Debug');
            $this->processLog['generatePolicy'] = $this->factorySIP->generatePolicy();
            $this->processLog['generateFlatEncryptionMetadata'] = $this->factorySIP->generateFlatEncryptionMetadata();
            $this->processLog['createBag'] = $this->factorySIP->createBag();
            $this->processLog['doSword'] = $this->factorySIP->doSword();
            $this->processLog['doDoorkeeper'] = $this->factorySIP->doDoorkeeper();
            $this->processLog['finish'] = $this->factorySIP->finish();

            return $this->factorySIP->getFid();
        } catch (IngestServiceException $exception) {
            $this->factorySIP->logging('IngestServiceException for SIP ' . $SIP->getSipId() . ' : ' . $exception->getMessage());
            $this->factorySIP->checkSwordRejected();
            $this->factorySIP->rollback($exception->getMessage());

            return ($SIP->getSipId() . ' : ' . $exception->getMessage());
        }
    }

    /**
     * public request for factory to ingest a sip
     *
     * @param $SIP
     *
     * @param mixed $session_id (optional) session id of user
     *
     * @return mixed|string
     */
    public function RequestSipIngest($SIP, $info = [])
    {
        return $this->DoSipIngest($SIP, $info);
    }
}
