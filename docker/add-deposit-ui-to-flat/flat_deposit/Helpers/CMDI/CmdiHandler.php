<?php

module_load_include('inc','flat_deposit','Helpers/CMDI/Template2FormParser');
module_load_include('inc','flat_deposit','Helpers/CMDI/Form2CmdiParser');


class CmdiHandlerException extends Exception {}
/**
 * Interface CmdiProfile
 */
class CmdiHandler
{

    const FORM_TEMPLATES_PATH = '/Helpers/CMDI/FormTemplates/';


    /**
     * Loads specified file as SimpleXML object.
     *
     * @param $fileName
     *
     * @return SimpleXMLElement
     *
     * @throws CmdiProfileException
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

    static public function getCmdiFromDatastream($fid)
    {
        $ds = islandora_datastream_load('CMD', $fid);

        if ($ds) {

            return(simplexml_load_string($ds->content));

        }

        return false;
    }


        static public function getAvailableTemplates($content_type)
    {
        $templates = [];
        foreach (glob(drupal_get_path('module', 'flat_deposit') . self::FORM_TEMPLATES_PATH . "*.xml") as $filename) {
            $xml = CmdiHandler::loadXml($filename);
            $ct = (string)$xml->header->content_type;

            if ($ct === $content_type){
                $templates [] = (string)$xml->header->template_name;
            }

        }
        return drupal_map_assoc($templates);
    }


    static public function request_cmdi_from_fedora_object_datastream($fid){


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
     * Generates the drupal input form for a specified profile
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
     * Generates a new cmdi file based on an xml parser file
     *
     * @param $user_name
     *
     * @param $form_data
     *
     * @return SimpleXMLElement|string
     */
    static public function generateCmdi($profile, $user_name, $form_data)
    {

        $fName = drupal_get_path('module', 'flat_deposit') . CmdiHandler::FORM_TEMPLATES_PATH . $profile . '.xml';

        $template = CmdiHandler::loadXml($fName);

        if (is_string($template)) {
            return $template;
        }

        $parser = new Form2CmdiParser();
        $cmdi = $parser->buildCmdi($profile, $template, $user_name, $form_data);

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
    static function removeMdSelfLink(&$xml)
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
        foreach ($xml->Components->{$profile}->Resources->children() as $child){
            unset($child[0]);
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
        $url = $config['url'] . '/examine?file=' .$fName;
        $port = $config['port'];

        $ch = curl_init();
        curl_setopt_array($ch, array(
                CURLOPT_URL => $url,
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

}

function select_profile_name_ajax_callback ($form, $form_state)
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
            $message  = 'File was not specified or has not correct extension (.cmdi)';
            return $message;
        }
        // Validate valid xml file
        if (!@simplexml_load_file($file->uri)){
            $message  = 'File is not a valid xml file';
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

        $cmdi = CmdiHandler::generateCmdi($profile, $user_name, $form_data);

        if (is_string($cmdi)){

            return $cmdi;

        }


        $export = $cmdi->asXML($fName);

        if (!$export) {
            return 'Unable to create cmdi record in users\' metadata directory';

        }

        return TRUE;



    }



}



