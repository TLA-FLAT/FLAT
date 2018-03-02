<?php

module_load_include('inc','flat_deposit','Helpers/CMDI/Template2FormParser');
module_load_include('inc','flat_deposit','Helpers/CMDI/Form2CmdiParser');


class CmdiHandlerException extends Exception {}
/**
 * CmdiHandler class. Processes cmdi profiles
 */
class CmdiHandler extends SimpleXMLElement
{

    // Path containing
    // a) xml form templates files for drupal form generation and drupal form 2 cmdi transformation and
    // b) xsd schema files for cmdi profile validation
    const FORM_TEMPLATES_PATH = '/Helpers/CMDI/FormTemplates/';




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

            return simplexml_load_string($ds->content,'CmdiHandler');

        }

        return false;
    }

    /**
     * Function that allows including processing instructions into exportable xml object.
     * @param $name
     * @param $value
     */
    public function addProcessingInstruction( $name, $value )
    {
        // Create a DomElement from this simpleXML object
        $dom_sxe = dom_import_simplexml($this);

        // Create a handle to the owner doc of this xml
        $dom_parent = $dom_sxe->ownerDocument;

        // Find the topmost element of the domDocument
        $xpath = new DOMXPath($dom_parent);
        $first_element = $xpath->evaluate('/*[1]')->item(0);

        // Add the processing instruction before the topmost element
        $pi = $dom_parent->createProcessingInstruction($name, $value);
        $dom_parent->insertBefore($pi, $first_element);
    }



    /**
     * Maps name on clarin id. In case of unspecified case, a get request is done to the clarin catalogue.
     *
     *
     * @return bool|string Either name associated with ID or false.
     */
    public function getNameById()
    {
        $node = $this->Header->MdProfile;
        if(!isset($node) OR empty((string)$node)){

            return false;
        }

        $id = (string)$node;

        switch ($id) {
            case 'clarin.eu:cr1:p_1475136016242' :
                $name = 'MPI_Bundle';
                break;

            case 'clarin.eu:cr1:p_1475136016239' :
                $name = 'MPI_Collection';
                break;

            case 'clarin.eu:cr1:p_14077457120358' :
                $name = 'lat-session';
                break;

            default :
                $url = "https://catalog.clarin.eu/ds/ComponentRegistry/rest/registry/1.x/profiles/$id";

                $ch = curl_init();
                curl_setopt_array($ch, array(

                    CURLOPT_RETURNTRANSFER => 1,
                    CURLOPT_CONNECTTIMEOUT => 5,
                    CURLOPT_TIMEOUT => 5,
                    CURLOPT_URL => $url));

                $result = curl_exec($ch);
                $xml = simplexml_load_string($result);
                if (!isset($xml->Header->Name)) {
                    return false;
                }

                $name = (string)$xml->Header->Name;
        }

        return $name;
    }



    //*************************************
    //Revise!!!!!!!!!!!
    //*************************************

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
     * Extracts CMDI profile name from datastream of fedora object
     *
     * @param $fid
     * @return bool
     *
     */
     static public function getCmdiProfileFromDatastream($fid){

        $cmdi = CmdiHandler::getCmdiFromDatastream($fid);

        if (!$cmdi AND isset ($cmdi->Header->MdProfile)) {
            return (string)$cmdi->Header->MdProfile;
        }

        return false;

    }

    // determine CMDI profile type as defined in general settings

    static public function getCmdiProfileType($fid){
      $profile_id = getCmdiProfileFromDatastream($fid);
      $collection_profiles = variable_get('flat_deposit_cmdi_profiles')['collection_profile_ids'];
      $collection_profile_values = explode(',',$collection_profiles);
      $bundle_profiles = variable_get('flat_deposit_cmdi_profiles')['bundle_profile_ids'];
      $bundle_profile_values = explode(',',$bundle_profiles);
      if (in_array($profile_id, $collection_profile_values)) {
        return "collection";
      }
      else if (in_array($profile_id, $bundle_profile_values)) {
        return "bundle";
      }
      else {
        return false;
      }
    }



    /**
     * Add Cmdi 'isPartOf' property to cmdi Resource
     *
     * @param $xml SimpleXMLElement cmdi xml file
     *
     * @param $parent_pid String fedora identifier of the parent
     */
    public function addIsPartOfProperty($parent_pid)
    {

        // Add isPartOf property to xml
        if (!isset($this->Resources->IsPartOfList)) {
            $this->Resources->addChild('IsPartOfList');
        }
        $this->Resources->IsPartOfList->addChild('IsPartOf', $parent_pid);


    }




    /**
     * Sets MdSelfLink in Cmdi header
     *
     * @param $fid String fedora id of MdSelfLink
     *
     * @param $handle String handle assigned to MdSelfLink
     */
    public function setMdSelfLink($fid, $handle)
    {
        $this->Header->MdSelfLink = $handle;
        $this->Header->MdSelfLink->addAttribute('lat:flatURI', $fid, 'http://lat.mpi.nl/');
    }


    /**
     * Removes MdSelfLink child from xml
     *
     */
    public function removeMdSelfLink()
    {
        if (isset($this->Header->MdSelfLink)) {

            unset($this->Header->MdSelfLink);
        }
    }

    /**
     * Removes all resources from xml file
     *
     */
    public function striplocalURI()
    {

        // Removal existing resources from ResourceProxy child
        foreach ($this->Resources->ResourceProxyList->ResourceProxy as $resource) {
            $value = $resource->ResourceRef;

            if (isset($value)) {
                $attributes = $resource->ResourceRef->attributes('lat', TRUE);

                if (isset($attributes->localURI)) {
                    unset ($attributes->localURI);
                }

            }
        }
    }

    /**
     * Removes a specified resource from xml file
     *
     * @param $resourceID string resource ID
     */
    public function stripSingleResource($resourceID)
    {
        // Removal existing resources from ResourceProxy child
        $proxy_list = $this->Resources->ResourceProxyList;
        if ($proxy_list) {
            foreach ($proxy_list->ResourceProxy as $resource) {

                if ($resource && $resource->attributes()) {

                    if ((string)$resource->attributes()->id == $resourceID) {
                        unset($resource[0]);
                    }
                }
            }
        }


        // Removal exitsing resources from Components->{profile}->Resources child
        $profile = $this->getNameById();
        foreach ($this->Components->{$profile}->Resource as $resource) {

            if ((string)$resource->attributes()->ref == $resourceID) {
                unset($resource[0]);

            }
        }


    }



    /**
     * Copies resources from an existing fedora object cmdi datastream to a the cmdi object.
     *
     * @param $fid String fedora ID of exsiting fedora object
     */
    public function addResourcesFromDatastream($fid)
    {

        $ds = islandora_datastream_load('CMD',$fid);

        $cmdi2 = simplexml_load_string($ds->content, 'CmdiHandler');

        $resourceProxyList = $cmdi2->Resources->ResourceProxyList;

        if (!empty($resourceProxyList)) {

            // Create new DOMElements from the two SimpleXMLElements
            $domxml = dom_import_simplexml($this->Resources->ResourceProxyList);
            foreach ($resourceProxyList->ResourceProxy as $resource) {

                $domDsResource  = dom_import_simplexml($resource);

                // Import the <Resource> into the xml document
                $domDsResource  = $domxml->ownerDocument->importNode($domDsResource, TRUE);

                // Append the <Resource> to <ResourceProxyList>
                $domxml->appendChild($domDsResource);

            }
        }

        $profile = $cmdi2->getNameById();
        $components = $cmdi2->Components->{$profile};

        if (!empty($components->Resource)) {

            // Create new DOMElements from the two SimpleXMLElements
            $domxml = dom_import_simplexml($this->Components->{$profile});

            foreach ($components->Resource as $resource) {

                $domDsResource = dom_import_simplexml($resource);

                // Import the <Resource> into the xml document
                $domDsResource = $domxml->ownerDocument->importNode($domDsResource, TRUE);

                // Append the <Resource> to <ResourceProxyList>
                $domxml->appendChild($domDsResource);

            }
        }

    }

    /**
     * Function for searching a specified directory, and add all found files as resources to xml.
     *
     * @param $directory
     */
     public function addResources($directory){

        // todo check filenames of exisitng resources in order to identify updated files

        // Inventarize existing resources in the cmdi ResourceProxyList
        $existing_resource_ids = [];
        $existing_filenames = [];
        $resourceProxyList = $this->Resources->ResourceProxyList;
        if (!empty($resourceProxyList->children())){

            foreach ($resourceProxyList->ResourceProxy as $resource) {

                $attributes = $resource->attributes();
                $id = (string)$attributes->id;
                $existing_resource_ids[] = $id;
                $lat_attributes = $resource->ResourceRef->attributes('lat', TRUE);

                if (isset($lat_attributes->flatURI)){
                    $flatURI = (string)$lat_attributes->flatURI;
                    $fObj = islandora_object_load($flatURI);

                    if ($fObj){
                        $existing_filenames[$id] = $fObj->label;
                    }
                }
            }
        }

        // scan all files of the bundle freeze directory and add theses as resources to the CMDI;
        if (!is_dir($directory)) return false;

        // Iterate through resources directory and add every file to the array resource with a unique resource ID as key.
        // In case a resource with an existing resource file name is found assign that resource the ID of the existing resource.
        // Otherwise, generate unique ID by incrementing the counter c until it is unqiue.

        $resources = array();
        $c=10000; # counter for resource ID (each resource within an CMDI-object needs an unique identifier)

        $rii = new RecursiveIteratorIterator(new RecursiveDirectoryIterator($directory,RecursiveDirectoryIterator::FOLLOW_SYMLINKS));

        foreach ($rii as $file) {
            if ($file->isDir()){
                continue;
            }

            $file_name = drupal_realpath($file->getPathname());

            if (in_array(basename($file_name), $existing_filenames) !== FALSE){

                $resource_id = array_search ( basename($file_name) , $existing_filenames);

            } else {

                $unique = false;
                while (!$unique) {
                    $c++;
                    $resource_id = "d$c";
                    if (in_array($resource_id, $existing_resource_ids) === FALSE) {
                        $unique = TRUE;
                    }

                }
            }
            $resources[$resource_id] = $file_name;

        }

        // Validate that all resources are accessible by ingest service
        $inaccesible_files = [];
        foreach ($resources as $rid => $file_name) {

            $fName = str_replace("\\\\", "\\", $file_name);

            if (!is_readable($fName)) {

                $inaccesible_files [] = basename($fName);

            }
        }

        if (!empty($inaccesible_files)){

            throw new CmdiHandlerException(t('One or more files are not accessible. ' . implode(', ', $inaccesible_files)));

        }

        // Add resources to simplexml variable
         $profile = $this->getNameById();
         foreach ($resources as $rid => $file_name) {

            $file_mime = self::fits_mimetype_check(drupal_realpath($file_name)) ;
            if (!$file_mime){
                throw new CmdiHandlerException('Unable to get fits mime type for specified file (' . $file_name . ')');
            }

            $file_size = filesize(drupal_realpath($file_name));

            if (in_array(basename($file_name), $existing_filenames) !== FALSE){
                $id = array_search ( basename($file_name) , $existing_filenames);

                // Add Resource to existing resource at Resources->ResourceProxyList
                $resourceProxyList = $this->Resources->ResourceProxyList;
                $resourceProxy = $resourceProxyList->xpath('cmd:ResourceProxy[@id="' . $id . '"]');
                $resourceProxy[0]->ResourceRef->addAttribute('lat:localURI', 'file:' . $file_name, "http://lat.mpi.nl/");


                // Update Resources at Components->Resources
                if ($profile == 'lat-session'){

                    // Add 'Resources'-child to Components->profile if not existing
                    if (!isset($this->Components->{$profile}->Resources)){
                        $this->Components->{$profile}->addChild('Resources');
                    }


                    $refType = 'MediaFile';


                } else{

                    $refType = 'Resource';

                }
                $node = $this->Components->{$profile};
                $resource = $node->xpath('//cmd:Resource[@ref="' . $id . '"]');


                if ($profile == 'lat-session'){

                    $resource->Type = 'document';
                    $resource->Format = $file_mime;

                }

                $resource->Size = $file_size;



            } else {

                // Add Resource to Resources->ResourceProxyList
                $resourceProxy = $this->Resources->ResourceProxyList->addChild('ResourceProxy');
                $resourceProxy->addAttribute('id', $rid);

                $resourceProxy->addChild('ResourceType', 'Resource');
                $resourceProxy->ResourceType->addAttribute('mimetype', $file_mime);

                $resourceProxy->addChild('ResourceRef');
                $resourceProxy->ResourceRef->addAttribute('lat:localURI', 'file:' . $file_name, "http://lat.mpi.nl/");

                // Add Resources to Components->Resources
                if ($profile == 'lat-session'){

                    // Add 'Resources'-child to Components->profile if not existing
                    if (!isset($this->Components->{$profile}->Resources)){
                        $this->Components->{$profile}->addChild('Resources');
                    }


                    $refType = 'MediaFile';
                    $resource = $this->Components->{$profile}->Resources->addChild($refType);

                } else{

                    $refType = 'Resource';
                    $resource = $this->Components->{$profile}->addChild($refType);

                }

                $resource->addAttribute('ref', $rid);

                if ($profile == 'lat-session'){

                    $resource->addChild('Type', 'document');
                    $resource->addChild('Format', $file_mime);

                }

                $resource->addChild('Size', $file_size);
            }


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


                $config = variable_get('flat_deposit_fits');
                $url = $config['url'] . '/examine?file=';
                $query = rawurlencode($filename);
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




    function add_attribute_tree_to_xml($data, &$xml_data ){

        foreach( $data as $key => $value ) {
            if( is_array($value) ){
                $subnode = $xml_data->$key;
                add_attribute_tree_to_xml($value, $subnode );
            } else {
                $xml_data->addAttribute($key ,$value);
            }
        }
    }


    /**
     * function definition to convert an array to xml. Don't use for attributes, use add_attribute_tree_to_xml instead
     *
     * @param $data php array
     * @param $xml_data simplexml object for which new child branches are created
     */
    function array_to_xml( $data, &$xml_data ) {
        foreach( $data as $key => $value ) {
            if( is_array($value) ) {
                if( is_numeric($key) ){
                    $key = 'item'. $key; //dealing with <0/>..<n/> issues
                }
                $subnode = $xml_data->addChild($key);
                $this->array_to_xml($value, $subnode);
            } else {
                $xml_data->addChild("$key",htmlspecialchars("$value"));
            }
        }
    }

// function definition to convert array to xml
    function array_to_xml_original ( $data, &$xml_data ) {
        foreach( $data as $key => $value ) {
            if( is_array($value) ) {
                if( is_numeric($key) ){
                    $key = 'item'. $key; //dealing with <0/>..<n/> issues
                }
                $subnode = $xml_data->addChild($key);
                array_to_xml($value, $subnode);
            } else {
                $xml_data->addChild("$key",htmlspecialchars("$value"));
            }
        }
    }


}






