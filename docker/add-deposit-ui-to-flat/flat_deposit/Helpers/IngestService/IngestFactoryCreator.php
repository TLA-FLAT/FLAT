<?php
//Creator.php
abstract class IngestFactoryCreator
{

    abstract protected function DoSipIngest(SIP $SIP);

    protected $returnValue;
    protected $processLog;


}

