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
    const FORM_TEMPLATES_PATH = __DIR__ . '/FormTemplates/';

    const CMDI_XSLTS = __DIR__ . '/XSLT/';


    static public function simplexml_load_cmdi_file($file_name){
        $xml = new DOMDocument;
        $xml->load($file_name);

        $xsl = new DOMDocument;
        $xsl->load(CmdiHandler::CMDI_XSLTS . '/ApplyCmdiPrefix.xsl');


        $proc = new XSLTProcessor;
        $proc->importStyleSheet($xsl);

        $result = $proc->transformToXML($xml);

        return simplexml_load_string($result, 'CmdiHandler');
    }


    static public function simplexml_load_cmdi_string(String $string){
        $xml = new DOMDocument;
        $xml->loadXML($string);

        $xsl = new DOMDocument;
        $xsl->load(CmdiHandler::CMDI_XSLTS . '/ApplyCmdiPrefix.xsl');


        $proc = new XSLTProcessor;
        $proc->importStyleSheet($xsl);

        $result = $proc->transformToXML($xml);

        return simplexml_load_string($result, 'CmdiHandler');
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
            return $ds->content;
        }

        return false;
    }

    static public function loadCleanedCmdi($string){
        $string = preg_replace('/cmd:Resource/', 'Resource', $string);
        return simplexml_load_string($string, 'CmdiHandler');
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
        $node = $this->xpath('cmd:Header/cmd:MdProfile')[0];

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

            case 'clarin.eu:cr1:p_1407745712035' :
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

    /**
     * Extracts CMDI profile name from datastream of fedora object
     *
     * @param $fid
     * @return bool
     *
     */
     static public function getCmdiProfileFromDatastream($fid){

         $cmdi_str = CmdiHandler::getCmdiFromDatastream($fid);
         $cmdi = CmdiHandler::loadCleanedCmdi($cmdi_str);

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
        $res = $this->xpath('cmd:Resources/cmd:IsPartOfList');
        if (!isset($res[0])) {
            $this->children('cmd',TRUE)->Resources->addChild('IsPartOfList');
        }

        $this->children('cmd',TRUE)->Resources->IsPartOfList->addChild('IsPartOf', $parent_pid);

    }

    /**
     * Cleans up MdSelfLink
     *
     */
    public function cleanMdSelfLink()
    {
        $res = $this->xpath('/cmd:CMD/cmd:Header/cmd:MdSelfLink');
        if (empty($res)){
            return true;

        }
        $value = $res[0];

        $pattern1 = '/^hdl/';
        $pattern2 = '/^handle/';
        $hasHandle = (preg_match($pattern1, $value) OR preg_match($pattern2, $value));

        if (!$hasHandle){
            unset($res[0][0]);
        }

        return true;

    }


    /**
     * Checks MdSelfLink
     *
     */
    public function validateMdSelfLink()
    {
        $res = $this->xpath('//cmd:MdSelfLink');
        if (empty($res)){
            return true;

        }

        $value = $res[0];
        $attributes = $value->attributes('lat', TRUE);
        $flatURI = $attributes['flatURI'];

        $pattern1 = '/^hdl/';
        $pattern2 = '/^handle/';
        $hasHandle = (preg_match($pattern1, $value) OR preg_match($pattern2, $value));

        if ($hasHandle AND !$flatURI){
            return "Invalid combination of MdSelfLink value and attributes. Cmdi files providing a handle at CMD->Header->MdSelfLink " .
            "also need to provide a flatURI attribute.";
        }

        return true;

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
        $md = $this->xpath('cmd:Header/cmd:MdSelfLink')[0];
        $md = $handle;
        $md->addAttribute('lat:flatURI', $fid, 'http://lat.mpi.nl/');
    }


    /**
     * Removes MdSelfLink child from xml
     *
     */
    public function removeMdSelfLink()
    {

        $header = $this->xpath('cmd:Header');
        $children = $header->children('cmd', true);

        if (isset($children->MdSelfLink)) {

            unset($children->MdSelfLink);
        }
    }

    /**
     * Removes all resources from xml file
     *
     */
    public function striplocalURI()
    {
        // Removal existing resources from ResourceProxy children
        foreach ($this->Resources->ResourceProxyList->children() as $resource) {
            $value = $resource->ResourceRef;

            if (isset($value)) {
                $attributes = $resource->ResourceRef->attributes('lat', TRUE);

                if (isset($attributes->localURI)) {
                    unset ($attributes->localURI);
                }

            }
        }
        // Removal existing resources from namespaced ResourceProxy children
        foreach ($this->Resources->ResourceProxyList->children('cmd', TRUE) as $resource) {
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
     * Adds resources to ResourceProxyList and Components section
     *
     * @param $resource_directory String Name of directory containing all files to be added
     *
     * @param null $fid String PID of a fedora object with a CMD datastream that has resource
     *
     * @throws CmdiHandlerException
     */
    public function addResources($metadata_source = 'new', $resource_directory = null, $fid = null)
    {
        switch ($metadata_source){
            case 'new':
                $result = $this->stripResourceProxyAndResources();
                if ($fid){
                    $this->addResourcesFromDatastream($fid);
                }
                if (!$result){
                    throw new CmdiHandlerException(t("Strip of Resources was not successful"));
                }
                break;
            case 'import':

                break;
        }


        if (is_null($resource_directory)){
            return true;
        }

        if (!is_dir($resource_directory)){
            throw new CmdiHandlerException(t("Directory is not accessible"));
        }

        $cmdi_resources  = $this->getCmdiResources();

        $directory_resources = $this->getDirectoryResources($resource_directory, $cmdi_resources);

        // Validate that all new resources are accessible
        $inaccesible_resources = $this->checkAccessibilityFiles($directory_resources);
        if (!empty($inaccesible_resources)){
            throw new CmdiHandlerException(t('One or more files are not accessible. ' . implode(', ', $inaccesible_resources)));
        }

        $result = $this->addNewDirectoryResources($cmdi_resources, $directory_resources);
        if (!$result){
            throw new CmdiHandlerException(t("Failed adding new resources to Cmdi"));
        }

        $result = $this->updateExistingResources($cmdi_resources, $directory_resources);

        if (!$result){
            throw new CmdiHandlerException(t("Failed updating cmdi resources"));
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

    function canBeValidated(){

        $name = $this->getNameById();
        if (!file_exists(self::FORM_TEMPLATES_PATH . $name . '.xsd')){
            throw new CmdiHandlerException("Could not find schema file for cmdi profile " . $name);
        }

        if (!file_exists(self::FORM_TEMPLATES_PATH . $name . '.xml')){
            throw new CmdiHandlerException("Could not find profile form file for cmdi profile " . $name);
        }

        return TRUE;
    }


    private function stripResourceProxyAndResources()
    {
        // Strip ResourceProxy
        $proxies = $this->xpath('cmd:Resources/cmd:ResourceProxyList/cmd:ResourceProxy');


        foreach ($proxies as $proxy){
            unset($proxy[0]);
        }


        $profile = $this->getNameById();
        $xml = simplexml_load_file(CmdiHandler::FORM_TEMPLATES_PATH . "/$profile.xml");

        $tags = $xml->xpath('/profileToDrupal/resources/allowed_tags/tag');

        $node = $this->getNodeForAddingResources($this);

        if ($node === false){
            return false;
        }

        foreach ($tags as $tag){
            $entries = $node->xpath('cmd:' . (String)$tag['id']);
            foreach ( $entries as $entry){
                unset ($entry[0]);
            }

        }
        return true;
    }


    /**
     * Copies resources from an existing fedora object cmdi datastream to a the cmdi object.
     *
     * @param $fid String Fedora PID of a fedora object
     */
    private function addNewDirectoryResources($cmdi_resources, $directory_resources)
    {
        // prepare
        $profile = $this->getNameById();
        $xml = simplexml_load_file(CmdiHandler::FORM_TEMPLATES_PATH . "/$profile.xml");

        $targetNode = $this->getNodeForAddingResources($this);

        if ($targetNode === false) {
            return false;
        }

        // Add new resources to ResourceProxyList
        $new_resources = array_diff_key ($directory_resources, $cmdi_resources);

        $resourceProxyList = $this->xpath('cmd:Resources/cmd:ResourceProxyList')[0];

        foreach ($new_resources as $rid => $file_name) {
            $file_mime = self::fits_mimetype_check(drupal_realpath($file_name));
            if (!$file_mime) {
                return false;
            }
            $proxy = $resourceProxyList->addChild("cmd:ResourceProxy", '', "http://www.clarin.eu/cmd/");
            $proxy->addAttribute('id', $rid);
            $proxy->addChild("ResourceType", "Resource");
            $proxy->children('cmd',true)->ResourceType->addAttribute('mimetype', $file_mime);
            $proxy->addChild("ResourceRef");
            $proxy->children('cmd',true)->ResourceRef->addAttribute('lat:localURI', 'file:' . $file_name, "http://lat.mpi.nl/");


            // Create new DOMElements from the two SimpleXMLElements
            $tag = $this->getTag($xml, $file_mime);

            if (!$tag){
                return false;
            }



            $id = (String)$tag['id'];
            $sourceResource = simplexml_load_string("<cmd:$id xmlns:cmd=\"http://www.clarin.eu/cmd/\"></cmd:$id>");

            $sourceResource->addAttribute('ref', $rid);

            foreach ($tag->updated_values as $value){
                $child_id = (String)$value->attributes();

                if ((String)$value[0] != ''){
                    $sourceResource->addChild('cmd:'. $child_id, (String)$value[0]);
                    continue;
                }

                if (strtolower($child_id) == 'size'){
                    $sourceResource->addChild('cmd:'. $child_id, filesize($file_name));
                    continue;
                }

                if (strtolower($child_id) == 'format'){
                    $sourceResource->addChild('cmd:'. $child_id, $file_mime);
                    continue;
                }
                if (strtolower($child_id) == 'type'){
                    $sourceResource->addChild('cmd:'. $child_id, explode('/', $file_mime)[0]);
                    continue;
                }
            }

            if (!$sourceResource){
                return false;
            }

            $resource = dom_import_simplexml($sourceResource);
            $insertBefores = $tag->xpath('insert_before');;
            $existingChildren = $targetNode->children('cmd',true)->xpath('child::node()');

            switch(true) {

                    //Case: Resource section has no children
                case empty($existingChildren):
                    //Case: Resource section has no other resources to care about
                case empty($insertBefores):

                    $parent = $targetNode;
                    $domxml = dom_import_simplexml($parent);
                    $domDsResource = $domxml->ownerDocument->importNode($resource, TRUE);
                    $domxml->appendChild($domDsResource);
                    break;

                default:
                    $found = false;
                    foreach ($insertBefores as $insertBefore) {

                        $results = $targetNode->xpath('cmd:' . $insertBefore);
                        if (!empty($results)) {
                            $parent = $results[0];
                            $found = TRUE;
                            break;
                        }
                    }
                    if ($found){
                            $domxml = dom_import_simplexml($parent);
                            $domDsResource = $domxml->ownerDocument->importNode($resource, TRUE);
                            $domxml->parentNode->insertBefore($domDsResource, $domxml);
                            break;
                    }
                    $parent = $targetNode;
                    $domxml = dom_import_simplexml($parent);
                    $domDsResource = $domxml->ownerDocument->importNode($resource, TRUE);
                    $domxml->appendChild($domDsResource);
                    break;

            }
        }
        return true;
    }

    private function getTag($profile, $file_mime){

        $tags = $profile->xpath('/profileToDrupal/resources/allowed_tags/tag');
        foreach ($tags as $tag){

            $pattern = $tag->mime_regex[0];
            if (preg_match($pattern, $file_mime)){
                return $tag[0];
            }

        }
        return false;
    }

    private function getResourceInComponentSection($cmdi, $profile, $resource_id)
    {


        $root = $cmdi->getNodeForAddingResources($cmdi);

        if ($root ===false){
            return false;
        }

        $tags = $profile->xpath('/profileToDrupal/resources/allowed_tags/tag');

        foreach ($tags as $tag) {
            $query = 'cmd:' . (String)$tag[0]['id'] . '[@ref=\'' .$resource_id . '\']';
            $results = $root[0]->xpath($query);

            if (!empty($results)) {
                return $results[0];
            }
        }
        return false;
    }

    /**
     * Copies resources from an existing fedora object cmdi datastream to a the cmdi object.
     *
     * @param $fid String Fedora PID of a fedora object
     */
    private function addResourcesFromDatastream($fid)
    {

        $cmdi_str = CmdiHandler::getCmdiFromDatastream($fid);
        $fObjectCmdi = CmdiHandler::loadCleanedCmdi($cmdi_str);
        $resourceProxyList = $fObjectCmdi->Resources->ResourceProxyList;

        if (!empty($resourceProxyList->children()) OR !empty($resourceProxyList->children('cmd',TRUE))) {

            // Create new DOMElements from the two SimpleXMLElements
            $domxml = dom_import_simplexml($this->Resources->ResourceProxyList);

            foreach ($resourceProxyList->children('cmd',TRUE) as $resource) {
                $domDsResource  = dom_import_simplexml($resource);
                // Import the <Resource> into the xml document
                $domDsResource  = $domxml->ownerDocument->importNode($domDsResource, TRUE);
                // Append the <Resource> to <ResourceProxyList>
                $domxml->appendChild($domDsResource);
            }

            foreach ($resourceProxyList->children() as $resource) {
                $domDsResource  = dom_import_simplexml($resource);
                // Import the <Resource> into the xml document
                $domDsResource  = $domxml->ownerDocument->importNode($domDsResource, TRUE);
                // Append the <Resource> to <ResourceProxyList>
                $domxml->appendChild($domDsResource);
            }

        }
        //Copy resources to Components
        $this->addResourcesToComponents($fObjectCmdi);

        return true;
    }

    /**
     * Allows to add resources to the componentes section of a otherwise populated cmdi file
     *
     * @param $sourceCmdi CmdiHandler a second cmdi file with resources to be added to the Components section
     */
    public function addResourceSectionToComponents($sourceCmdi){

        $this->addResourcesToComponents($sourceCmdi);
    }

    /**
     *
     * @param $sourceCmdi
     */
    private function addResourcesToComponents($sourceCmdi){

        //Copy resources to Components
        $searchNode = $this->getNodeForAddingResources($sourceCmdi);
        $profile = $this->getNameById();
        $xml = simplexml_load_file(CmdiHandler::FORM_TEMPLATES_PATH . "/$profile.xml");
        $tags = $xml->xpath('/profileToDrupal/resources/allowed_tags/tag');

        foreach ($tags as $tag){

            $entries = $searchNode->xpath((String)$tag['id']);
            foreach ( $entries as $entry){
                $targetNode = $this->getNodeForAddingResources($this);
                // Create new DOMElements from the two SimpleXMLElements
                $domxml = dom_import_simplexml($targetNode);
                $resource = dom_import_simplexml($entry);
                // Import the <Resource> into the xml document
                $domDsResource = $domxml->ownerDocument->importNode($resource, TRUE);
                // Append the <Resource> to <ResourceProxyList>
                $domxml->appendChild($domDsResource);
            }

            $entries = $searchNode->xpath('cmd:' . (String)$tag['id']);
            foreach ( $entries as $entry){
                $targetNode = $this->getNodeForAddingResources($this);
                // Create new DOMElements from the two SimpleXMLElements
                $domxml = dom_import_simplexml($targetNode);
                $resource = dom_import_simplexml($entry);
                // Import the <Resource> into the xml document
                $domDsResource = $domxml->ownerDocument->importNode($resource, TRUE);
                // Append the <Resource> to <ResourceProxyList>
                $domxml->appendChild($domDsResource);
            }
        }


    }

    private function getResourceValue($resource){

        $ref = $resource->xpath('cmd:ResourceRef')[0];
        $lat_attributes = $ref->attributes('lat', TRUE);

        if (isset($lat_attributes['localURI'])){
            return basename($lat_attributes['localURI']);
        }

        if (isset($ref)){
            return basename((String)$ref);
        }


        if (isset($lat_attributes->flatURI)){
            $pid = (string)$lat_attributes->flatURI;
        } else {
            // SPARQL query with hdl
            $handle = (String)$resource->ResourceRef[0];
            $query = "SELECT ?fid WHERE { ?fid <http://purl.org/dc/elements/1.1/identifier> \"" .
                str_replace("hdl:", "https://hdl.handle.net/", $handle) . "\" }";

            $connection = islandora_get_tuque_connection();
            $res = $connection->repository->ri->sparqlQuery($query);

            if (!empty($res)) {
                $pid = $res[0]['fid']['value'];
            } else {
                $pid = false;
            }
        }

        $fObj = islandora_object_load($pid);

        $value = $fObj ? $fObj->label : "";

        return $value;

    }

    private function getCmdiResources(){
        $resources = array();
        $proxies = $this->xpath('cmd:Resources/cmd:ResourceProxyList/cmd:ResourceProxy');


        foreach ($proxies as $proxy) {
            $attributes = $proxy->attributes();
            $id = (string)$attributes->id;
            $resources[$id] = '';
            $value = $this->getResourceValue($proxy);
            $resources[$id] = $value;
        }

        return $resources;
    }



    /**
     * Iterates through directory and adds every file to an array of resource with a unique resource ID as key.
     * In case an existing resource with identical file name is found, assign that resource the ID of the existing resource.
     * Otherwise, generate unique ID by incrementing the counter c until it is unqiue.
     *
     * @param $directory
     * @param $existing_resources array with keys as unique ids and values as file names
     * @return array
     */

    private function getDirectoryResources($directory, $existing_resources = array()){

        $resources = array();
        $c=10000; # counter for resource ID (each resource within an CMDI-object needs an unique identifier)

        $rii = new RecursiveIteratorIterator(new RecursiveDirectoryIterator($directory,RecursiveDirectoryIterator::FOLLOW_SYMLINKS));

        foreach ($rii as $file) {
            if ($file->isDir()){continue;}

            $file_name = drupal_realpath($file->getPathname());

            $resource_id = array_search(basename($file_name) , $existing_resources);

            // generate new ID if resource name is unknown
            if (!$resource_id){
                $unique = false;

                while (!$unique) {
                    $c++;
                    $resource_id = "d$c";
                    if (in_array($resource_id, array_keys($existing_resources)) === FALSE) {
                        $unique = TRUE;
                    }
                }
            }
            $resources[$resource_id] = $file_name;
        }
        return $resources;
    }


    /**
     * Validate that all resources are accessible
     * @param $resources
     * @return array
     */

    private function checkAccessibilityFiles($resources){
        $inaccesible_files = [];
        foreach ($resources as $rid => $file_name) {

            $fName = str_replace("\\\\", "\\", $file_name);

            if (!is_readable($fName)) {

                $inaccesible_files [] = basename($fName);

            }
        }
        return $inaccesible_files;
    }

    private function updateExistingResources($cmdi_resources, $directory_resources){


        // Add lat:flatURI attribute to updated resources in ResourceProxyList
        $resourceProxyList = $this->xpath('cmd:Resources/cmd:ResourceProxyList')[0];

        $existing_resources = array_intersect_key($directory_resources, $cmdi_resources);

        foreach ($existing_resources as $rid => $file_name) {
            $file_mime = self::fits_mimetype_check(drupal_realpath($file_name));
            if (!$file_mime) {
                return false;
            }

            $proxy = $resourceProxyList->xpath('cmd:ResourceProxy[@id="'. $rid .'"]');
            if (empty($proxy)){
                return false;
            }

            // change value of existing attribute or add new one
            $attributes = $proxy[0]->children('cmd',TRUE)->ResourceRef->attributes('lat', true);
            if (isset($attributes['localURI'])){
                $attributes['localURI'] = 'file:' . $file_name;
            } else {
                $proxy[0]->children('cmd',TRUE)->ResourceRef->addAttribute('lat:localURI', 'file:' . $file_name, "http://lat.mpi.nl/");
            }

            $value = (String)$proxy[0]->children('cmd',TRUE)->ResourceRef;
            if(!empty($value)){
                $proxy[0]->children('cmd',TRUE)->ResourceRef = '';
            }

            //Update resources in Components section
            $profile = $this->getNameById();
            $xml = simplexml_load_file(CmdiHandler::FORM_TEMPLATES_PATH . "/$profile.xml");

            $resource = $this->getResourceInComponentSection($this, $xml, $rid);

            if (!$resource){
                return false;
            }

            $values = array(
                'size' => filesize($file_name),
                'format' => $file_mime,
            );

            $id = $resource->getName();
            $tag = $xml->resources->allowed_tags->xpath('tag[@id=\''. $id .'\']');
            foreach ($tag->updated_values as $value) {
                $id = (String)$value->attributes();

                if ($id == 'type') {
                    $values['type'] = (String)$value;
                }
                $value->{$id} = $values[$id];
            }
        }
        return true;
    }


    private function getNodeForAddingResources($cmdi){
        $profile = $cmdi->getNameById();
        $xml = simplexml_load_file(CmdiHandler::FORM_TEMPLATES_PATH . "/$profile.xml");

        $resources_root_path = (String)$xml->xpath('/profileToDrupal/resources/root_path')[0];
        $resources_root_node = $cmdi->children('cmd',true)->Components->{$resources_root_path};


        if (empty($resources_root_node)){
            return false;
        }

        // Return subNode if this is set for the specific profile
        $subnodes = (String)$xml->xpath('/profileToDrupal/resources/subnodes')[0];

        if (!empty($subnodes)){
            // add subnode if this node does not exist and return false if this is not possible
            if (empty($resources_root_node[0]->xpath('cmd:' .$subnodes))){
                $resources_root_node[0]->addChild('cmd:' . $subnodes, '', 'http://www.clarin.eu/cmd/');
            }

            if (empty($resources_root_node[0]->xpath('cmd:' .$subnodes))){
                return false;
            }

            return $resources_root_node[0]->xpath('cmd:' .$subnodes)[0];
        }

        return $resources_root_node[0];
    }
}