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
        if (!isset($this->Resources->IsPartOfList) OR !isset($this->Resources->children()->IsPartOfList)) {
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
    public function addResources($resource_directory = null, $fid = null)
    {
        $result = $this->stripResourceProxyAndResources();

        if (!$result){
            throw new CmdiHandlerException(t("Strip of Resources was not successful"));
        }

        if ($fid){
            $this->addResourcesFromDatastream($fid);
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


        $this->addNewDirectoryResources($cmdi_resources, $directory_resources);
        $this->updateExistingResources($cmdi_resources, $directory_resources);


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
        $resourceProxyList = $this->Resources->ResourceProxyList;

        $proxies = $resourceProxyList->xpath("ResourceProxy");
        foreach ($proxies as $proxy){
            unset($proxy[0]);
        }

        $proxies = $resourceProxyList->xpath("cmd:ResourceProxy");
        foreach ($proxies as $proxy){
            unset($proxy[0]);
        }

        $profile = $this->getNameById();
        $xml = simplexml_load_file(CmdiHandler::FORM_TEMPLATES_PATH . "/$profile.xml");

        $tags = $xml->xpath('/profileToDrupal/resources/allowed_tags/tag');

        $node = $this->getNodeForAddingResources($this);

        if (!$node){
            return false;
        }

        foreach ($tags as $tag){
            $entries = $node->xpath((String)$tag['id']);
            foreach ( $entries as $entry){
                unset ($entry[0]);
            }

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

        if (!$targetNode) {
            return false;
        }

        // Add new resources to ResourceProxyList
        $new_resources = array_diff_key ($directory_resources, $cmdi_resources);

        $resourceProxyList = $this->Resources->ResourceProxyList;

        foreach ($new_resources as $rid => $file_name) {
            $file_mime = self::fits_mimetype_check(drupal_realpath($file_name));
            if (!$file_mime) {
                return false;
            }
            $proxy = $resourceProxyList->addChild("ResourceProxy");
            $proxy->addAttribute('id', $rid);
            $proxy->addChild("ResourceType", "Resource");
            $proxy->ResourceType->addAttribute('mimetype', $file_mime);
            $proxy->addChild("ResourceRef");
            $proxy->ResourceRef->addAttribute('lat:localURI', 'file:' . $file_name, "http://lat.mpi.nl/");


            // Create new DOMElements from the two SimpleXMLElements
            $tag = $this->getSourceResource($xml, $file_mime);

            if (!$tag){return false;}

            $values = array(
                'size' => filesize($file_name),
                'format' => $file_mime,
            );


            $id = (String)$tag['id'];
            $sourceResource = simplexml_load_string("<$id></$id>");

            $sourceResource->addAttribute('ref', $rid);

            foreach ($tag->updated_values as $value){
                $id = (String)$value->attributes();

                if (strtolower($id) == 'type'){
                    $values['type'] = (String)$value;
                }
                $sourceResource->addChild($id, $values[strtolower($id)]);
            }

            if (!$sourceResource){
                return false;
            }

            $domxml = dom_import_simplexml($targetNode);

            $resource = dom_import_simplexml($sourceResource);
            // Import the <Resource> into the xml document
            $domDsResource = $domxml->ownerDocument->importNode($resource, TRUE);
            // Append the <Resource> to <ResourceProxyList>
            $domxml->appendChild($domDsResource);

        }

        return true;
    }


    private function getSourceResource($profile, $file_mime)
    {
        $tags = $profile->xpath('/profileToDrupal/resources/allowed_tags/tag');

        foreach ($tags as $tag) {

            $reg_ex = (String)$tag->mime_regex[0];
            if (preg_match($reg_ex, $file_mime)) {
                return $tag;
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

        $lat_attributes = $resource->ResourceRef->attributes('lat', TRUE);

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
        $resourceProxyList = $this->Resources->ResourceProxyList;

        // get children
        $proxies = $resourceProxyList->xpath('ResourceProxy');
        foreach ($proxies as $proxy) {
            $attributes = $proxy->attributes();
            $id = (string)$attributes->id;
            $resources[$id] = '';
            $value = $this->getResourceValue($proxy);
            $resources[$id] = $value;
        }

        $proxies = $resourceProxyList->xpath('cmd:ResourceProxy');
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
        $resourceProxyList = $this->Resources->ResourceProxyList;

        $existing_resources = array_intersect_key($directory_resources, $cmdi_resources);

        foreach ($existing_resources as $rid => $file_name) {
            $file_mime = self::fits_mimetype_check(drupal_realpath($file_name));
            if (!$file_mime) {
                return false;
            }

            $proxy = $resourceProxyList->xpath('cmd:ResourceProxy[@id="'. $rid .'"]');
            if (empty($proxy)){
                $proxy = $resourceProxyList->xpath('ResourceProxy[@id="'. $rid .'"]');
            }
            if (empty($proxy)){
                return false;
            }

            $proxy[0]->ResourceRef->addAttribute('lat:localURI', $file_name, "http://lat.mpi.nl/");

            //Update resources in Components section
            $profile = $this->getNameById();
            $xml = simplexml_load_file(CmdiHandler::FORM_TEMPLATES_PATH . "/$profile.xml");

            $tag = $this->getSourceResource($xml, $file_mime);
            $searchNode = $this->getNodeForAddingResources($this);


            if (!$tag OR !$searchNode){
                return false;
            }

            $id = (String)$tag['id'];

            //for some unknown reason $node = $searchNode->xpath('/'.$id.'[@ref="'. $rid .'"]'); does not work
            foreach ($searchNode->children()->{$id} as $value) {
                if ((String)$value['ref'] == $rid) {
                    $values = array(
                        'size' => filesize($file_name),
                        'format' => $file_mime,
                    );

                    foreach ($tag->updated_values as $value) {
                        $id = (String)$value->attributes();

                        if ($id == 'type') {
                            $values['type'] = (String)$value;
                        }
                        $value->{$id} = $values[$id];
                    }
                }
            }
        }
        return true;
    }


    private function getNodeForAddingResources($cmdi){
        $profile = $cmdi->getNameById();
        $xml = simplexml_load_file(CmdiHandler::FORM_TEMPLATES_PATH . "/$profile.xml");

        $resources_root_path = (String)$xml->xpath('/profileToDrupal/resources/root_path')[0];
        $resources_root_node = $cmdi->Components->xpath($resources_root_path);

        if (empty($resources_root_node)){
            $resources_root_node = $cmdi->Components->xpath('cmd:' . $resources_root_path);
        }

        if (empty($resources_root_node)){
            return false;
        }

        $subnodes = (String)$xml->xpath('/profileToDrupal/resources/subnodes')[0];

        if (!empty($subnodes) AND empty($resources_root_node->{$subnodes})){

            $resources_root_node->addChild($subnodes);
            return $resources_root_node->{$subnodes}[0];

        }

        return $resources_root_node[0];
    }
}