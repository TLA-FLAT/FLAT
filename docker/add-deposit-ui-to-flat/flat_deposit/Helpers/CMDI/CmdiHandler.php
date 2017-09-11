<?php

module_load_include('inc','flat_deposit','Helpers/CMDI/Template2FormParser');
module_load_include('inc','flat_deposit','Helpers/CMDI/Form2CmdiParser');


class CmdiHandlerException extends Exception {}
/**
 * CmdiHandler class. Processes cmdi profiles
 */
class CmdiHandler
{

    // Path containing
    // a) xml form templates files for drupal form generation and drupal form 2 cmdi transformation and
    // b) xsd schema files for cmdi profile validation
    const FORM_TEMPLATES_PATH = '/Helpers/CMDI/FormTemplates/';


    /**
     * Scans the cmdi2Drupal form path for profiles and returns profile names of a certain content type
     *
     * @param $content_type drupal content type of the profile (e.g. flat_collection or flat_bundle)
     *
     * @return array associative array with file names
     */
    static public function getAvailableTemplates($content_type)
    {
        $templates = [];
        foreach (glob(drupal_get_path('module', 'flat_deposit') . self::FORM_TEMPLATES_PATH . "*.xml") as $filename) {
            $xml = CmdiHandler::loadXml($filename);
            $ct = (string)$xml->header->content_type;

            if ($ct === $content_type) {
                $templates [] = (string)$xml->header->template_name;
            }

        }
        return drupal_map_assoc($templates);
    }


    /**
     * Loads specified file as SimpleXML object.
     *
     * @param $fileName
     *
     * @return mixed SimpleXMLElement or error message
     */
    static public function loadXml($fileName)
    {

        if (simplexml_load_file($fileName) === false) {

            $message = 'Error loading schema file. ';

            foreach (libxml_get_errors() as $error) {
                $message .= "Line: $error->line($error->column) $error->message <br>";
            }

            return $message;

        } else {
            $xml = simplexml_load_file($fileName);
            return $xml;
        }
    }

    /**
     * Uses tuque to return cmdi datastream of a fedora object
     *
     * @param $fid fedora object ID
     *
     * @return bool|SimpleXMLElement
     */
    static public function getCmdiFromDatastream($fid)
    {
        $ds = islandora_datastream_load('CMD', $fid);

        if ($ds) {

            return (simplexml_load_string($ds->content));

        }

        return false;
    }


    /**
     * Uses curl to return cmdi datastream of a fedora object
     *
     * @param $fid fedora object ID
     *
     * @return bool|SimpleXMLElement
     */
    static public function request_cmdi_from_fedora_object_datastream($fid)
    {


        $host = variable_get('flat_deposit_ingest_service')['host_name'];
        $scheme = variable_get('flat_deposit_ingest_service')['host_scheme'];
        $base = $GLOBALS['base_path'];


        $object_url = $scheme . '://' . $host . $base . "islandora/object/" . $fid . '/datastream/CMD/';

        $ch = curl_init();
        curl_setopt_array($ch, array(
                CURLOPT_URL => $object_url,
                CURLOPT_RETURNTRANSFER => 1,
                CURLOPT_CONNECTTIMEOUT => 5,
                CURLOPT_TIMEOUT => 5,
            )
        );


        $result = curl_exec($ch);
        $httpcode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
        curl_close($ch);

        return ($httpcode >= 200 && $httpcode < 300) ? $result : false;

    }


    /**
     * Performs the generation of a drupal form on basis of a specified profile
     *
     * @return mixed array containing renderable form array or false
     */
    static public function generateDrupalForm($profile)
    {

        $fName = drupal_get_path('module', 'flat_deposit') . CmdiHandler::FORM_TEMPLATES_PATH . $profile . '.xml';

        $template = CmdiHandler::loadXml($fName);

        if (is_string($template)) {
            return $template;
        }

        $parser = new Template2FormParser();
        $form = $parser->buildDrupalForm($template);


        return $form;

    }

    /**
     * Adds 'add' and 'remove' buttons to fields with multival property
     *
     * @param $form drupal renderable array with form fields
     *
     * @param $multi_fields array with form elements. Keys indicate id of the field.
     *
     * @return mixed
     */
    static public function addMultivalElements($fields, $multi_fields)
    {


        foreach ($multi_fields as $id => $value) {

            // link to field element depends on subNode property of the element, If set for field ID the element is nested in fieldset
            if (isset($fields['data']['#value']['subNode'][$id])) {

                $subNode = $fields['data']['#value']['subNode'][$id];
                $link_field =  &$fields[$subNode][$id];

            } else {

                $link_field =& $fields[$id];

            };

            $copy_form_element = $link_field[0];
            $copy_add_button = $link_field['add'];
            $copy_remove_button = $link_field['remove'];

            unset($link_field['add']);
            unset($link_field['remove']);

            if ($value >= 1) {

                $copy_remove_button ['#access'] = TRUE;

                for ($i = 1; $i <= $value; $i++) {
                    // if form element does not exist copy the first element and add to form
                    if (!isset($link_field[$i])){

                        $link_field[$i] = $copy_form_element;
                        unset($link_field[$i]['#default_value']);
                    }

                }
            } else {

                if (isset($link_field[1])){

                    unset($link_field[1]);

                }
                $copy_remove_button ['#access'] = FALSE;
            }


            // make remove button visible depending on amount of extra fields
            #krumo($value);
            $link_field['add'] = $copy_add_button;
            $link_field['remove'] = $copy_remove_button;


        }

        return $fields;

    }

    /**
     * Transforms form_state 'clicked_button' value in aggregated data (i.e. associative array with '#name' property
     * as ID and #value-property as switch which action (i.e. add or substract)) to perform. Ass array is saved in form_state
     *
     * @param $form_state
     *
     */
    static public function aggregateClickedButtons(&$form_state)
    {


        if (isset($form_state['clicked_button'])) {

            $id = $form_state['clicked_button']['#name'];

            if (isset($form_state['count'][$id])) {

                if ($form_state['clicked_button']['#value'] == 'Add') {

                    $form_state['count'][$id]++;
                } else {

                    if ($form_state['count'][$id] >= 1) $form_state['count'][$id]--;

                }

            } else {

                $form_state['count'][$id] = 1;

            }
        }


    }
    static public function addInheritedElements($form, &$form_state, $parent_nid){

        // Fill form field with loaded data
        $parent = node_load($parent_nid);
        $pwrapper = entity_metadata_wrapper('node', $parent);
        $pFid = $pwrapper->flat_fid->value();
        $parentCmdi = CmdiHandler::getCmdiFromDatastream($pFid);

        try {

            module_load_include('inc', 'flat_deposit', 'Helpers/CMDI/Cmdi2FormParser');
            $parser = new Cmdi2FormParser;

            $default_values = $parser->getDefaultValuesFromCmdi($form_state['selected'], $parentCmdi);

            $inheritedMultivalForm = self::createInheritedMultivalForm($form['template_container']['elements'], $default_values, $form_state);

        } catch (Cmdi2FormParserException $exception) {

            drupal_set_message($exception->getMessage(), 'warning');
        }


    }

    /**
     * Generates a CMDI simplexml object for a cmdi form template populated with data of a drupal form.
     *
     * @param $profile name of the form template
     *
     * @param $user_name
     *
     * @param $form_data form data of drupal form
     *
     * @return SimpleXMLElement|string error message
     */
    static public function generateCmdi($profile, $user_name, $form_data)
    {

        $fName = drupal_get_path('module', 'flat_deposit') . CmdiHandler::FORM_TEMPLATES_PATH . $profile . '.xml';

        $template = CmdiHandler::loadXml($fName);

        // return error message if loading of simplexml object hasn't worked
        if (is_string($template)) {
            return $template;
        }

        $parser = new Form2CmdiParser();
        $cmdi = $parser->buildCmdi($profile, $template, $user_name, $form_data);

        #return 'debug';
        return $cmdi;


    }
    /**
     * @param $xml SimpleXMLElement cmdi xml file
     * @param $parent_pid String fedora identifier of the parent
     */
    static public function addIsPartOfProperty(&$xml, $parent_pid)
    {

        // Add isPartOf property to xml
        if (!isset($xml->Resources->IsPartOfList)) {
            $xml->Resources->addChild('IsPartOfList');
        }
        $xml->Resources->IsPartOfList->addChild('IsPartOf', $parent_pid);


    }

    /**
     * Removes MdSelfLink child from xml
     *
     * @param $xml SimpleXMLElement cmdi xml file
     */
    static public function removeMdSelfLink(&$xml)
    {
        if (isset($xml->Header->MdSelfLink)) {

            unset($xml->Header->MdSelfLink);
        }
    }

    /**
     * Removes all resources from xml file
     *
     * @param $xml SimpleXMLElement cmdi xml file
     */
    static public function stripResources(&$xml, $profile)
    {
        // Removal existing resources from ResourceProxy child
        foreach ($xml->Resources->ResourceProxyList->ResourceProxy as $resource) {
            unset($resource[0]);
        }

        // Removal exitsing resources from Components->{profile}->Resources child
        foreach ($xml->Components->{$profile}->Resource as $resource){
            unset($resource[0]);
        }
    }

    /**
     * Removes a specified resource from xml file
     *
     * @param $xml stdClass a cmdi simplexml object
     *
     * @param $profile string a valid cmdi profile name (e.g. MPI_bundle)
     *
     * @param $resourceID string resource ID
     */
    static public function stripSingleResource(&$xml, $profile, $resourceID)
    {


        // Removal existing resources from ResourceProxy child
        $proxy_list = $xml->Resources->ResourceProxyList;
        if ($proxy_list){
            foreach ($proxy_list->ResourceProxy as $resource){

                if ($resource && $resource->attributes()) {
                    if ((string)$resource->attributes()->id == $resourceID) {
                        unset($resource[0]);
                    }
                }

            }

        }


        // Removal exitsing resources from Components->{profile}->Resources child

        foreach ($xml->Components->{$profile}->Resource as $resource) {

            if ((string)$resource->attributes()->ref == $resourceID) {
                unset($resource[0]);

            }
        }




    }








        /**
     * Maps name on clarin id. In case of unspecified case, a get request is done to the clarin catalogue.
     *
     * @param $id clarin id.
     *
     * @return bool|string Either name associated with ID or false.
     */
    static public function getNameById($id){
        switch ($id){
            case 'clarin.eu:cr1:p_14077457120358' :
                $name = 'lat-session';
                break;
            case 'clarin.eu:cr1:p_1475136016239' :
                $name = 'MPI_Collection';
                break;
            case 'clarin.eu:cr1:p_1475136016242' :
                $name = 'MPI_Bundle';
                break;
            default :

                $url ="https://catalog.clarin.eu/ds/ComponentRegistry/rest/registry/1.x/profiles/$id";

                $ch = curl_init();
                curl_setopt_array($ch, array(

                    CURLOPT_RETURNTRANSFER => 1,
                    CURLOPT_CONNECTTIMEOUT => 5,
                    CURLOPT_TIMEOUT => 5,
                    CURLOPT_URL => $url));

                $result = curl_exec($ch);
                $xml = simplexml_load_string($result);
                if (!isset($xml->Header->Name)){

                    trigger_error('Unable to retrieve name from provided profile id');
                    return false;
                }

                $name = (string)$xml->Header->Name;
        }


        return $name;
    }


    /**
     * Function for searching a specified directory, and add all found files as resources to xml.
     *
     * @param $xml
     *
     * @param $profile
     *
     * @param $directory
     */
    static public function addResources(&$xml, $profile, $directory){


        // scan all files of the bundle freeze directory and add theses as resources to the CMDI;
        if (!is_dir($directory)) return false;

        $rii = new RecursiveIteratorIterator(new RecursiveDirectoryIterator($directory,RecursiveDirectoryIterator::FOLLOW_SYMLINKS));

        $resources = array();
        $c=10000; # counter for resource ID (each resource within an CMDI-object needs an unique identifier)


        foreach ($rii as $file) {
            if ($file->isDir()){
                continue;}

            $c++;
            $fid = "d$c";
            $resources[$fid] = drupal_realpath($file->getPathname());

        }

        // Add resources to simplexml variable
        foreach ($resources as $fid => $file_name) {

            $file_mime = self::fits_mimetype_check(drupal_realpath($file_name)) ;
            if (!$file_mime){
                throw new CmdiHandlerException('Unable to get fits mime type for specified file');
            }

            $file_size = filesize(drupal_realpath($file_name));

            // Add Resource to Resources->ResourceProxyList
            $resourceProxy = $xml->Resources->ResourceProxyList->addChild('ResourceProxy');
            $resourceProxy->addAttribute('id', $fid);

            $resourceProxy->addChild('ResourceType', 'Resource');
            $resourceProxy->ResourceType->addAttribute('mimetype', $file_mime);

            $resourceProxy->addChild('ResourceRef');
            $resourceProxy->ResourceRef->addAttribute('lat:localURI', 'file:' . $file_name, "http://lat.mpi.nl/");

            // Add Resources to Components->Resources
            if ($profile == 'lat-session'){

                // Add 'Resources'-child to Components->profile if not existing
                if (!isset($xml->Components->{$profile}->Resources)){
                    $xml->Components->{$profile}->addChild('Resources');
                }


                $refType = 'MediaFile';
                $resource = $xml->Components->{$profile}->Resources->addChild($refType);

            } else{

                $refType = 'Resource';
                $resource = $xml->Components->{$profile}->addChild($refType);

            }

            $resource->addAttribute('ref', $fid);

            if ($profile == 'lat-session'){

                $resource->addChild('Type', 'document');
                $resource->addChild('Format', $file_mime);

            }

            $resource->addChild('Size', $file_size);

        }

    }


    /**
     * Adds freeze directory base path to resources in cmdi file.
     *
     * @param $xml
     *
     * @param $directory
     *
     * @return bool
     */
    static public function addFrozenDirBaseToResources($xml, $directory){

        // Removal existing resources from ResourceProxy child
        foreach ($xml->Resources->ResourceProxyList->ResourceProxy as $resource) {
            $attributes = $resource->ResourceRef->attributes('lat', TRUE);
            $oldName = (string)$attributes['localURI'];
            $resource->ResourceRef =  'file:' . drupal_realpath($directory) . '/' . $oldName  ;
        }

    }



    /**
     * Call to fits REST API allowing to determine the mime type of a specified file.
     * Returns false if file is not accessible, FITS service returns wrong response code or the format attribute within
     * the xml file returned by fits service is not set.
     *
     * @param string $filename name of the file to be checked
     *
     * @return bool|string
     *
     */
    static public function fits_mimetype_check($filename){

        $fName = str_replace("\\\\", "\\", $filename);
        if (!file_exists($fName) OR !is_readable($fName)){

            return false;

        }

        $config = variable_get('flat_deposit_fits');
        $url = $config['url'] . '/examine?file=';
        $query = rawurlencode($fName);
        $port = $config['port'];

        $ch = curl_init();
        curl_setopt_array($ch, array(
                CURLOPT_URL => $url . $query,
                CURLOPT_PORT => $port,
                CURLOPT_RETURNTRANSFER => 1,
                CURLOPT_CONNECTTIMEOUT => 5,
                CURLOPT_TIMEOUT => 5,
            )
        );

        $result = curl_exec($ch);
        $httpcode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
        curl_close($ch);

        if ($httpcode < 200 OR $httpcode >= 300){

            return false;

        }

        $xml = simplexml_load_string($result);

        if (!isset($xml->identification->identity['mimetype'])){

            return false;

        }

        return (string)$xml->identification->identity['mimetype'];

    }


    static public function createInheritedMultivalForm(&$field, $default_values, &$form_state){

        foreach ($default_values as $default_key => $default_value){

            // check if values are nested by looking for array keys that are not numeric
            $not_all_numeric = in_array(false, array_map('is_numeric',array_keys($default_value)));

            if ($not_all_numeric){
                // nested
                foreach ($default_value as $field_name => $field_array){

                    #$form_state['count'][$field_name] = 0;

                    foreach ($field_array as $key => $value){

                        if ($key == 0) {

                            $basis = $field[$default_key][$field_name][$key];

                        } else {

                            $field[$default_key][$field_name][(string)$key] = $basis;

                            if (!isset($form_state['addInheritedElements'])) {

                                // keep track of multifield number
                                if (isset($form_state['count'][$field_name])) {

                                    $form_state['count'][$field_name]++;

                                } else {

                                    $form_state['count'][$field_name] = 1;
                                }
                            }

                        }
                    }


                }

            } else{
                // not nested

                foreach ($default_value as $key => $value){

                    if ($key == 0) {

                        $basis = $field[$default_key][$key];

                    } else {

                        $field[$default_key][(string)$key] = $basis;


                        // keep track of multifield number
                        if (!isset($form_state['addInheritedElements'])) {

                            $form_state['count'][$default_key] = $key;

                        }
                    }
                }
            }
        }

        $form_state['addInheritedElements'] = TRUE;

    }

}



///////////////////////////////////
// functions outside class
///////////////////////////////////


function select_profile_name_ajax_callback ($form, &$form_state)
{

    return $form['template_container'];
}



/**
 * Recursively exchanges array keys with a numeric value with '#default_value'.
 * @param $array
 *
 * @return array|void
 *
 */
function exchange_numeric_key_with_default_value_property($array) {
    if (!is_array($array)) return;

    $helper = array();

    foreach ($array as $key => $value) {

        if (is_array($value)) {

            $helper[$key] = exchange_numeric_key_with_default_value_property($value);

        } else {
            if (is_numeric($key)){

                $helper['#default_value' ] = $value;

            } elseif(is_numeric(array_search($key,['month','day','year']))){
                $helper['#default_value'][$key] = $value;
            } else{
                $helper[$key] = $value;
            }
        }
    }
    return $helper;
}


/**
 * Helper function that manages a) the generation of a new cmdi file or b) the import of existing cmdi file
 *
 * @param $data array containing a) nothing (empty array) or b) name of cmdi profile, the drupal form data of the specified profile and the id of the owner
 *
 * @param $fName string specifies the name under which cmdi will be stored
 *
 * @param $import bool specifies whether cmdi is imported or generated
 *
 * @return bool|string true in case of success or otherwise error message
 */
function get_cmdi($data, $fName, $import)
{
    // Import file in case this option was selected
    if ($import) {

        $file = file_save_upload('cmdi_file', array(

            // Validate extensions.
            'file_validate_extensions' => array('cmdi'),
        ));

        // If the file did not passed validation:
        if (!$file) {
            $message = 'File was not specified or has not correct extension (.cmdi)';
            return $message;
        }
        // Validate valid xml file
        if (!@simplexml_load_file($file->uri)) {
            $message = 'File is not a valid xml file';
            return $message;
        }

        copy(drupal_realpath($file->uri), $fName);

        if (!file_exists($fName)) {

            $message = 'Unable to copy specified file to target location';
            return $message;
        }

        return TRUE;

    } else {

        $profile = $data['select_profile_name'];
        $form_data = $data['template_container']['elements'];
        $user_name = $data['owner'];

        // get new simplexml object
        $cmdi = CmdiHandler::generateCmdi($profile, $user_name, $form_data);


        if (is_string($cmdi)) {

            return $cmdi;

        }

        $export = $cmdi->asXML($fName);

        if (!$export) {
            return 'Unable to create cmdi record in users\' metadata directory';

        }

        return TRUE;

    }
}




function add_multival_to_cmdi_form_ajax($form, $form_state) {


    return $form['template_container'];
}

function remove_multival_from_cmdi_form_ajax($form, $form_state) {

    return $form['template_container'];
}


function ajax_submit_button_call($form, $form_state) {

    return $form['container'];

}

function ajax_select_button_call($form, $form_state) {
    return $form['container'];
}
