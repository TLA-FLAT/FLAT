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
        $required = array(
            'loggedin_user',
            'nid',
        );
        $diff = array_diff($required,array_keys($info));
        if($diff){

            throw new IngestServiceException('Not all required variables are defined. Following variables are missing: ' . implode(', ', $diff));

        };
        $this->node = node_load($info['nid']);

        $this->wrapper = entity_metadata_wrapper('node',$this->node);

        $this->info = $info;

        $this->info['policy'] = $this->wrapper->flat_policies->value();

        $this->info['cmdi_handling'] = $this->wrapper->flat_cmdi_option->value();


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


    function addResourcesToCmdi(){

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

        $id = $xml->Header->MdProfile;
        $profile_name = CmdiHandler::getNameById($id);


        $resource_handling = $this->info['cmdi_handling'];
        $directory = $this->wrapper->flat_location->value();



        switch ($resource_handling) {

            case 'template':
            case 'import':
                {
                    //todo 1 function strip lat:localURI
                CmdiHandler::striplocalURI($xml);
                break;
            }


        }
        try{

            CmdiHandler::addResources($xml, $profile_name, $directory);

        } catch (CmdiHandlerException $exception){

            throw new IngestServiceException($exception->getMessage());

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

        if ($this->test){

            $this->wrapper->flat_bundle_status->set('valid');
            $this->wrapper->save();

        } else {

            // TODO remove comment when working

            node_delete_multiple(array($this->info['nid']));



        }

        $this->createBlogEntry(TRUE);

        $this->logging('Stop finish');

        return TRUE;

    }



    /**
     *
     * @param bool $succeeded outcome of the processing procedure.
     *
     * @param null|string $additonal_message possible error messages generated during processing
     */
    protected function createBlogEntry ($succeeded, $additonal_message = NULL){

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
        $body = sprintf("<p>%s %s</p><p>%s of %s belonging to %s %s. Check bundle ". l(t('here'), $url_link, array('html' => TRUE, 'external' => FALSE, 'absolute' => TRUE, 'base_url' => $scheme . '://' . $host)) . '</p>',$bundle, $collection, $action, $bundle, $collection, $outcome);
        $body = preg_replace(array('/lat_/') , array('lat%3A'), $body);

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