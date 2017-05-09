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
     * 'session_id': the session id of the user doing the ingest
     * 'nid' : the node id of the bundle to ingest
     *
     * @return bool
     *
     * @throws IngestServiceException
     */
    public function init($info){
        $this->logging('Starting init');
        $required = array(
            'session_id',
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

        $result = db_select('sessions', 's')
            ->fields('s', array('uid'))
            ->condition('s.sid', $this->info['session_id'])
            ->execute();


        if ($result){

            $user_id = $result->fetchAssoc()['uid'];

        } else {

            $user_id = 0;
        }

        $bundle_id = $this->node->uid;

        // only bundle owner, editors and admins might validate the bundle
        if ($this->test){

            if($user_id === $bundle_id OR user_access('validate bundles', user_load($user_id))) {

                $this->logging('Finishing authentication');
                return TRUE;

            } else {

                throw new IngestServiceException('User has not enough privileges to perform requested action');

            }

        } else {
            // only certified users and corpmanager might ingest the bundle

            if (($user_id === $bundle_id AND user_access('certified user', user_load($user_id))) OR user_access('ingest bundles', user_load($user_id))) {

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

        if (!$this->test){
            $this->logging('Finishing prepareSipData');
            return TRUE;

        } else {

            $stream = 'freeze';
        }

        module_load_include('inc', 'flat_deposit', 'inc/class.FlatBundle');


        $move = FlatBundle::moveBundleData($this->node, 'data', $stream);
        if (!$move){
            throw new IngestServiceException('Unable to move bundle data to ' . $stream);
        }


        if (!is_null($this->wrapper->flat_cmdi_file->value())){
            $move = FlatBundle::moveBundleData($this->node, 'metadata', $stream);
            if (!$move){
                throw new IngestServiceException('Unable to move bundle metadata to ' . $stream);
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



        switch ($resource_handling){


            case 'import':{
                CmdiHandler::addFrozenDirBaseToResources($xml, $directory);
                break;
            }
            default :{
                CmdiHandler::removeMdSelfLink($xml);
                CmdiHandler::stripResources($xml, $profile_name);

                try{

                    CmdiHandler::addResources($xml, $profile_name, $directory);

                } catch (CmdiHandlerException $exception){

                    throw new IngestServiceException($exception->getMessage());

                }

                break;
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


            $url_link = $scheme . '://' . $host . '/flat/islandora/object/' . $this->fid ;

        } else {

            $url_link = 'node/' . $this->node->nid;

        }

        $outcome = $succeeded ? 'succeeded' : 'failed' ;
        $action = $this->test ? 'Validation' : 'Archiving';

        $bundle = $this->node->title;
        $collection = $this->wrapper->flat_parent_title->value();

        $summary = sprintf("<p>%s of %s %s</p>",$action, $bundle, $outcome);
        $body = sprintf("<p>%s %s</p><p>%s of %s belonging to %s %s. Check bundle ". l('here', $url_link) . '</p>',$bundle, $collection, $action, $bundle, $collection, $outcome);
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

        $move = FlatBundle::moveBundleData($this->node, 'data', 'private');
        $move = FlatBundle::moveBundleData($this->node, 'metadata', 'private');


        // create blog entry
        $this->createBlogEntry(FALSE, $message);

        //set status of bundle
        $this->wrapper->flat_bundle_status->set('failed');
        $this->wrapper->save();

        $this->logging('Finishing customRollback');
        return;


    }


}