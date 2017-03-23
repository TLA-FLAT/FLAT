<?php

include_once('SIP.php');

class Bundle extends SIP
{

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
     * Nothing to setup here
     */

    function setupIngest()
    {

        return TRUE;

    }

    /**
     * set the target directory to be frozen
     */

    function setFrozenSipDir()
    {

        $freezeDir = drupal_realpath('freeze://') . '/' .  $this->getOwner() . '/collection/' . $this->getSipId() . '/';

        $this->frozenSipDir = $freezeDir;


        return TRUE;

    }


    /**
     * Copies the record.cmdi file from the drupal controlled user data directories to a newly created subdirectory at
     * the owners backend freeze location

     * @return bool
     * @throws IngestServiceException
     */
    function prepareSipData()
    {

        $cmdi_source = $this->getCmdiRecord();
        $cmdi_target = $this->getFrozenSipDir() .  '/data/metadata/record.cmdi';

        // create (if necessary) backend directory
        if (!file_exists(dirname($cmdi_target))) drupal_mkdir(dirname($cmdi_target), NULL, TRUE);

        copy($cmdi_source, $cmdi_target);

        if (!file_exists($cmdi_target)) {

            throw new IngestServiceException('Could not copy cmdi file to target location');

        } else {

            return TRUE;

        }
    }

    /**
     * Call to change ownerID of fedora objects using the Fedora REST api.
     *
     * @throws IngestServiceException
     */
    function doFedora()
    {

        // create object that can do ReST requests
        module_load_include('inc','flat_deposit', '/Helpers/Fedora_REST_API');

        $accessFedora = variable_get('flat_deposit_fedora');
        $rest_fedora = new FedoraRESTAPI($accessFedora);


        // Change ownership of ingested files
        $data = array(
            'ownerId' => $this->getOwner(),
        );

        $result = $rest_fedora->modifyObject($this->getFid(), $data);

        if (!$result) {
            $message = 'Couldn\'t change ownership of files';


            throw new IngestServiceException ($message);
        }

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

    function rollback(Array $processes)
    {
        if (file_exists($this->getFrozenSipDir())){
            $this->removeFrozenZipDir();
        }

        if (file_exists(dirname($this->getFrozenSipDir()) . '/' . $this->getSipId() . '.zip')){
            $this->removeFrozenZipDir();
        }
        /*if ($processes['doSword']){
            $this->removeSwordBag();
        }
        */

        if($this->getFid()){
            $this->removeIngestedObject();
        }
        return TRUE;
    }

    function removeSwordBag(){

        $basePath = variable_get('flat_deposit_paths')['bag'];
        $bagDir = $basePath . $this->getSipId();
        recursiveRmDir($bagDir);
        rmdir($bagDir);

    }

    function removeIngestedObject(){

        module_load_include('inc','flat_deposit', '/Helpers/Fedora_REST_API');

        $accessFedora = variable_get('flat_deposit_fedora');
        $rest_fedora = new FedoraRESTAPI($accessFedora);

        $rest_fedora->deleteObject($this->getFid());
    }


}