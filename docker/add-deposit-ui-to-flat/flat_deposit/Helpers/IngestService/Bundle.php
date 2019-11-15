<?php

include_once('SIP.php');

class Bundle extends SIP
{
    // the node containing most of the important information
    protected $node;

    // the drupal entity_metadata_wrapper of this node
    protected $wrapper;

    /**
     * Set up function for bundle ingest; also validates provided input.
     *
     * @param array $info  array of configuration parameters necessary for successful processing of a bundle
     *
     * The array requires following parameters:
     *
     * 'loggedin_user': the user ID of the user doing the ingest
     * 'nid' : the node id of the bundle to ingest
     *
     * @return bool
     *
     * @throws IngestServiceException
     */
    public function init($info){
        $this->logging('Starting init');

        if (!$info['nid']){
            throw new IngestServiceException("Node id is not specified");
        }

        $this->node = node_load($info['nid']);
        $this->wrapper = entity_metadata_wrapper('node',$this->node);

        $info['cmdi_handling'] = $this->wrapper->flat_cmdi_option->value();
        $info['policy'] = $this->wrapper->flat_policies->value();
        $info['visibility'] = $this->wrapper->flat_metadata_visibility->value();

        $required = array(
            'loggedin_user',
            'nid',
            'policy',
            'cmdi_handling',
            'visibility'
        );
        $diff = array_diff($required,array_keys($info));
        if($diff){

            throw new IngestServiceException('Not all required variables are defined. Following variables are missing: ' . implode(', ', $diff));

        };

        $this->info = $info;




        // set status of bundle
        $status = $this->test ? 'validating' : 'processing';
        $this->wrapper->flat_bundle_status->set($status);
        $this->wrapper->save();

        $this->logging('Finishing init');
        return TRUE;
    }

    /**
     * Bundle permissions are handled by drupal. In case of missing permissions the whole ingest form including
     * submission button is absent. Accordingly has permission been granted in case user click on submit.
     *
     * @return bool
     *\
     * @throws IngestServiceException
     */
    public function authenticateUser()
    {
        $this->logging('Starting authentication');

        $query = new EntityFieldQuery();
        $query->entityCondition('entity_type', 'user')
            ->propertyCondition('uid', $this->info['loggedin_user'])
        ;

        $results = $query->execute();


        if (empty($results)){

            $id_loggedin_user = 0;

        } else {

            $id_loggedin_user = $this->info['loggedin_user'];
        }

        $uid_bundle = $this->node->uid;

        // only bundle owner, editors and admins might validate the bundle
        if ($this->test){

            if($id_loggedin_user === $uid_bundle OR user_access('validate bundles', user_load($id_loggedin_user))) {

                $this->logging('Finishing authentication');
                return TRUE;

            } else {

                throw new IngestServiceException('User has not enough privileges to perform requested action');

            }

        } else {
            // only certified users and corpmanager might ingest the bundle

            if (($id_loggedin_user === $uid_bundle AND user_access('certified user', user_load($id_loggedin_user))) OR user_access('ingest bundles', user_load($id_loggedin_user))) {

                $this->logging('Finishing authentication');
                return TRUE;

            } else {

                throw new IngestServiceException('User has not enough privileges to perform requested action');

            }
        }



    }

    /**
     * Either freeze data (validate) or do nothing (ingest)
     *
     * @return bool
     *
     * @throws IngestServiceException
     */
    function prepareSipData()
    {
        $this->logging('Starting prepareSipData');

        // Validated bundles do not need to be prepared
        if (!$this->test){
            $this->logging('Finishing prepareSipData');
            return TRUE;

        }

        module_load_include('inc', 'flat_deposit', 'inc/class.FlatBundle');

        $move = FlatBundle::moveBundleData($this->node, 'data', 'freeze');

        if (!$move){

            throw new IngestServiceException('Unable to move bundle data to freeze');

        }

        if (!is_null($this->wrapper->flat_cmdi_file->value())){

            $move = FlatBundle::moveBundleData($this->node, 'metadata', 'freeze');

            if (!$move){

                throw new IngestServiceException('Unable to move bundle metadata to freeze');

            }

            // update local variables
            $this->node = node_load($this->node->nid);
            $this->wrapper = entity_metadata_wrapper('node', $this->node);

            $cmdi_info = $this->wrapper->flat_cmdi_file->value();;
            $file_name = $cmdi_info['uri'];

            $this->cmdiRecord = $file_name;

        };

        $this->logging('Finishing prepareSipData; Data has been moved');
        return TRUE;

    }


    function validateResources(){
        $this->logging('Starting validateResources');
        $path = $this->wrapper->flat_location->value();

        $fileNames = file_scan_directory($path, '/.*/', array('min_depth' => 0));

        $deletedFiles = $this->wrapper->flat_deleted_resources ? $this->wrapper->flat_deleted_resources->value() : NULL;

        if (!isset($deletedFiles) OR ($deletedFiles == '')) {

            if(empty($fileNames)){
                throw new IngestServiceException('There are no (accessible) files in the chosen folder.');

            }

        }

        $pattern = '/^[\da-zA-Z][\da-zA-Z\._\-]+\.[\da-zA-Z]{1,9}$/';
        $violators = [];

        foreach ($fileNames as $uri => $file_array){

            $fileName = $file_array->filename;
            if (preg_match($pattern, $fileName) == false){
             $violators[] = $fileName;
            }

        }

        if (!empty($violators)){
            $message = 'Bundle contains files with names violating our file naming policy. ' .
            'Allowed are names starting with an alphanumeric characters (a-z,A-Z,0-9) followed by more alphanumeric characters '.
            'or these special characters (.-_). The name of the file needs to have an extension marked by a dot (".") '.
            'followed by 1 to 9 characters. ';

            $message .= 'Following file(s) have triggered this message: ';
            $message .= implode(', ', $violators);

            throw new IngestServiceException($message);
        }

        $this->logging('Finishing validateResources');
        return TRUE;
    }

    function addResourcesToCmdi(){

        $this->logging('Starting addResourcesToCmdi');

        module_load_include('inc','flat_deposit','/Helpers/CMDI/class.CmdiHandler');

        $file_name = $this->cmdiTarget;

        $cmdi = CmdiHandler::simplexml_load_cmdi_file($file_name);


        if (!$cmdi OR !$cmdi->canBeValidated()){
            throw new IngestServiceException('Unable to load record.cmdi file');
        }

        $directory = $this->wrapper->flat_location->value();

        try{

            $fid = isset($this->wrapper->flat_fid) ? $this->wrapper->flat_fid->value() : null;
            $flat_type = isset($this->wrapper->flat_type) ? $this->wrapper->flat_type->value() : NULL;
            $md_type = isset($this->wrapper->flat_cmdi_option) ? $this->wrapper->flat_cmdi_option->value() : NULL;
            if ($flat_type == 'update') {
                $md_type = 'existing';
            }

            switch ($md_type) {
                case 'new':
                    $cmdi->cleanMdSelfLink();
                    break;
                case 'import':
                case 'template':
                case 'existing':
                    if ($flat_type !== 'update') {
                        $cmdi->removeMdSelfLink();
                    }
                    else {
                        $cmdi->cleanMdSelfLink();
                    }
                    break;
            }

            $cmdi->addResources($md_type, $directory, $fid);

        } catch (CmdiHandlerException $exception){

            throw new IngestServiceException($exception->getMessage());

        }

        $check = $cmdi->asXML($file_name);

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

        $this->createBlogEntry(TRUE);

        if ($this->test){

            $this->wrapper->flat_bundle_status->set('valid');
            $this->wrapper->save();

        } else {

            // TODO remove comment when working

            $dir = drupal_realpath($this->wrapper->flat_location->value());
            if ($dir AND is_readable($dir) AND count(scandir($dir)) == 2){
                unlink ($dir);
            };

            node_delete_multiple(array($this->info['nid']));



        }


        $this->logging('Stop finish');

        return TRUE;

    }



    /**
     *
     * @param bool $succeeded outcome of the processing procedure.
     *
     * @param null|string $additonal_message possible error messages generated during processing
     */
    protected function  createBlogEntry ($succeeded, $additonal_message = NULL){

        $this->logging('Starting createBlogEntry');

        $host = variable_get('flat_deposit_ingest_service')['host_name'];
        $scheme = variable_get('flat_deposit_ingest_service')['host_scheme'];
        if (!$this->test AND $succeeded){

            $url_link = '/islandora/object/' . $this->fid ;

        } else {

            $url_link = '/node/' . (string)$this->node->nid;

        }

        $outcome = $succeeded ? 'succeeded' : 'failed' ;
        $action = $this->test ? 'Validation' : 'Archiving';

        $bundle = $this->node->title;
        $collection = $this->wrapper->flat_parent_title->value();

        $summary = sprintf("<p>%s of %s %s</p>",$action, $bundle, $outcome);
        $body = sprintf("<p>%s %s</p><p>%s of %s belonging to %s %s. Check bundle ". l(t('here'), $url_link, array('html' => TRUE, 'external' => TRUE, 'absolute' => TRUE, 'base_url' => $scheme . '://' . $host)) . '</p>', $bundle, $collection, $action, $bundle, $collection, $outcome);

        if ($additonal_message){ $body .=  '</p>Exception message:</p>' . $additonal_message ;};

        $new_node = new stdClass();
        $new_node->type = 'blog';
        $new_node->language = 'und';
        $new_node->title = sprintf("Result of processing bundle %s",$bundle);
        $new_node->uid = $this->node->uid;
        $new_node->status = 1;
        $new_node->sticky = 0;
        $new_node->promote = 0;
        $new_node->format = 3;
        $new_node->revision = 0;
        $new_node->body['und'][0]['format'] = 'full_html';
        $new_node->body['und'][0]['summary'] = $summary;
        $new_node->body['und'][0]['value'] = $body;
        node_save($new_node);

        $this->logging('Finishing createBlogEntry; Blog entry created');
    }



    function customRollback($message){

        $this->logging('Starting customRollback');

        // bundles need to unfreeze (if frozen) during rollback
        module_load_include('inc', 'flat_deposit', 'inc/class.FlatBundle');

        $move = FlatBundle::moveBundleData($this->node, 'data', 'unfreeze');
        $move = FlatBundle::moveBundleData($this->node, 'metadata', 'unfreeze');


        // create blog entry
        $this->createBlogEntry(FALSE, $message);

        //set status of bundle
        $this->wrapper->flat_bundle_status->set('failed');
        $this->wrapper->save();

        $this->logging('Finishing customRollback');
        return;


    }


}
