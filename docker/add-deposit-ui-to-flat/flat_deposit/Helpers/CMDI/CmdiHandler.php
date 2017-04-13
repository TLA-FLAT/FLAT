<?php

module_load_include('inc','flat_deposit','Helpers/CMDI/Template2FormParser');
module_load_include('inc','flat_deposit','Helpers/CMDI/Drupal2CmdiParser');


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

        $parser = new Drupal2CmdiParser();
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

        // Add 'Resources'-child to Components->profile if not existing
        if (!isset($xml->Components->{$profile}->Resources)){
            $xml->Components->{$profile}->addChild('Resources');
        }

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

            $file_mime = mime_content_type(drupal_realpath($file_name)) ;
            $file_size = filesize(drupal_realpath($file_name));

            // Add Resource to Resources->ResourceProxyList
            $resourceProxy = $xml->Resources->ResourceProxyList->addChild('ResourceProxy');
            $resourceProxy->addAttribute('id', $fid);

            $resourceProxy->addChild('ResourceType', 'Resource');
            $resourceProxy->ResourceType->addAttribute('mimetype', $file_mime);

            $resourceProxy->addChild('ResourceRef');
            $resourceProxy->ResourceRef->addAttribute('lat:localURI', 'file:' . $file_name, "http://lat.mpi.nl/");

            // Add Resources to Components->Resources
            $refType = ($profile == 'lat-session') ? 'MediaFile' : 'Resource';
            $resource = $xml->Components->{$profile}->Resources->addChild($refType);
            $resource->addAttribute('ref', $fid);

            if ($profile == 'lat-session'){

                $resource->addChild('Type', 'document');

            }

            $resource->addChild('Format', $file_mime);
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



}