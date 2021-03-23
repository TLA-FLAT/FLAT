<?php

module_load_include('inc', 'flat_deposit', 'Helpers/CMDI/Template2FormParser');
module_load_include('inc', 'flat_deposit', 'Helpers/CMDI/Form2CmdiParser');

class RecursiveDotFilterIterator extends RecursiveFilterIterator
{
    public function accept()
    {
        return '.' !== substr($this->current()->getFilename(), 0, 1);
    }
}

class CmdiHandlerException extends Exception
{

}
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

    public static function simplexml_load_cmdi_file($file_name)
    {
        $xml = new DOMDocument;
        $xml->load($file_name);
        $xsl = new DOMDocument;
        $xsl->load(CmdiHandler::CMDI_XSLTS . '/ApplyCmdiPrefix.xsl');


        $proc = new XSLTProcessor;
        $proc->importStyleSheet($xsl);

        $result = $proc->transformToXML($xml);
        return simplexml_load_string($result, 'CmdiHandler');
    }

    public static function simplexml_load_cmdi_string(String $string)
    {
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
    public static function getCmdiFromDatastream($fid)
    {
        $ds = islandora_datastream_load('CMD', $fid);

        if ($ds) {
            return $ds->content;
        }

        return FALSE;
    }

    public static function loadCleanedCmdi($string)
    {
        $string = preg_replace('/cmd:Resource/', 'Resource', $string);
        return simplexml_load_string($string, 'CmdiHandler');
    }



    /**
     * Function that allows including processing instructions into exportable xml object.
     * @param $name
     * @param $value
     */
    public function addProcessingInstruction($name, $value)
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
     * @return bool|string Either name associated with ID or FALSE.
     */
    public function getNameById()
    {
        $node = $this->xpath('cmd:Header/cmd:MdProfile')[0];
        if (!$node) {
            // possibly non-namespaced CMDI
            $node = $this->xpath('Header/MdProfile')[0];
        }

        if (!isset($node) OR empty((string)$node)) {
            return FALSE;
        }

        $id = (string)$node;

        switch ($id) {
            case 'clarin.eu:cr1:p_1475136016242':
                $name = 'MPI_Bundle';
                break;

            case 'clarin.eu:cr1:p_1475136016239':
                $name = 'MPI_Collection';
                break;

            case 'clarin.eu:cr1:p_1407745712035':
                $name = 'lat-session';
                break;

            case 'clarin.eu:cr1:p_1417617523856':
                $name = 'lat-SL-session';
                break;

            case 'clarin.eu:cr1:p_1407745712064':
                $name = 'lat-corpus';
                break;

            default:
                $url = "https://catalog.clarin.eu/ds/ComponentRegistry/rest/registry/1.x/profiles/$id";

                $ch = curl_init();
                curl_setopt_array($ch, array(

                    CURLOPT_RETURNTRANSFER => 1,
                    CURLOPT_CONNECTTIMEOUT => 5,
                    CURLOPT_TIMEOUT => 60,
                    CURLOPT_URL => $url));

                $result = curl_exec($ch);
                $xml = simplexml_load_string($result);
                if (!isset($xml->Header->Name)) {
                    return FALSE;
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
    public static function getCmdiProfileFromDatastream($fid)
    {
        $cmdi_str = CmdiHandler::getCmdiFromDatastream($fid);
        $cmdi = CmdiHandler::loadCleanedCmdi($cmdi_str);
        if (!$cmdi AND isset($cmdi->Header->MdProfile)) {
            return (string)$cmdi->Header->MdProfile;
        }

        return FALSE;
    }

    // determine CMDI profile type as defined in general settings

    public function getCmdiProfileType()
    {
        $profile_id = $this->Header->MdProfile;
        if (!$profile_id) {
            // probably namespaced CMDI
            $this->registerXPathNamespace('cmd', 'http://www.clarin.eu/cmd/');
            $profile_id = $this->xpath('//cmd:MdProfile')[0];
        }
        $collection_profiles = variable_get('flat_deposit_cmdi_profiles')['collection_profile_ids'];
        $collection_profile_values = explode(',', $collection_profiles);
        $bundle_profiles = variable_get('flat_deposit_cmdi_profiles')['bundle_profile_ids'];
        $bundle_profile_values = explode(',', $bundle_profiles);
        if (in_array($profile_id, $collection_profile_values)) {
            return "collection";
        } else if (in_array($profile_id, $bundle_profile_values)) {
            return "bundle";
        } else {
            return FALSE;
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
            $this->children('cmd', TRUE)->Resources->addChild('IsPartOfList');
        }

        $this->children('cmd', TRUE)->Resources->IsPartOfList->addChild('IsPartOf', $parent_pid);

    }

    /**
     * Cleans up MdSelfLink
     *
     */
    public function cleanMdSelfLink()
    {
        $res = $this->xpath('/cmd:CMD/cmd:Header/cmd:MdSelfLink');
        if (empty($res)) {
            return TRUE;
        }
        $value = $res[0];

        $pattern1 = '/^hdl/';
        $pattern2 = '/^handle/';
        $pattern3 = '/^https\:\/\/hdl\.handle\.net/';
        $hasHandle = (preg_match($pattern1, $value) OR preg_match($pattern2, $value) OR preg_match($pattern3, $value));

        if (!$hasHandle) {
            unset($res[0][0]);
        }

        return TRUE;
    }


    /**
     * Checks MdSelfLink
     *
     */
    public function validateMdSelfLink()
    {
        $res = $this->xpath('//cmd:MdSelfLink');
        if (empty($res)) {
            return TRUE;
        }

        $value = $res[0];
        $attributes = $value->attributes('lat', TRUE);
        $flatURI = $attributes['flatURI'];

        $pattern1 = '/^hdl/';
        $pattern2 = '/^handle/';
        $pattern3 = '/^https\:\/\/hdl\.handle\.net/';
        $hasHandle = (preg_match($pattern1, $value) OR preg_match($pattern2, $value) OR preg_match($pattern3, $value));

        if ($hasHandle AND !$flatURI) {
            return "Invalid combination of MdSelfLink value and attributes. Cmdi files providing a handle at CMD->Header->MdSelfLink " .
            "also need to provide a flatURI attribute.";
        }

        return TRUE;
    }

    /**
     * Sets MdSelfLink in CMDI header
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
     * Removes MdSelfLink
     *
     */
    public function removeMdSelfLink()
    {

        $mdSelfLink = $this->xpath('//cmd:MdSelfLink');

        if (empty($mdSelfLink)) {
            return TRUE;
        }

        unset($mdSelfLink[0][0]);

        return TRUE;
    }

    /**
     * Removes localURI from ResourceProxies
     *
     */
    public function striplocalURI()
    {
        // Remove localURI from ResourceProxy children
        foreach ($this->Resources->ResourceProxyList->children() as $resource) {
            $value = $resource->ResourceRef;

            if (isset($value)) {
                $attributes = $resource->ResourceRef->attributes('lat', TRUE);

                if (isset($attributes->localURI)) {
                    unset($attributes->localURI);
                }
            }
        }
        // Remove localURI from namespaced ResourceProxy children
        foreach ($this->Resources->ResourceProxyList->children('cmd', TRUE) as $resource) {
            $value = $resource->ResourceRef;

            if (isset($value)) {
                $attributes = $resource->ResourceRef->attributes('lat', TRUE);

                if (isset($attributes->localURI)) {
                    unset($attributes->localURI);
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
        switch ($metadata_source) {
            case 'new':
            case 'template':
                $result = $this->stripResourceProxyAndResources();
                if ($fid) {
                    $this->addResourcesFromDatastream($fid);
                }
                if (!$result) {
                    throw new CmdiHandlerException(t("Strip of Resources was not successful"));
                }
                break;
            case 'import':
            case 'existing':
                break;
        }

        if (is_null($resource_directory)) {
            return TRUE;
        }

        if (!is_dir($resource_directory)) {
            throw new CmdiHandlerException(t("Directory is not accessible"));
        }

        $cmdi_resources  = $this->getCmdiResources();

        $directory_resources = $this->getDirectoryResources($resource_directory, $cmdi_resources);

        // Validate that all new resources are accessible
        $inaccesible_resources = $this->checkAccessibilityFiles($directory_resources);
        if (!empty($inaccesible_resources)) {
            throw new CmdiHandlerException(t('One or more files are not accessible. ' . implode(', ', $inaccesible_resources)));
        }

        $result = $this->addNewDirectoryResources($cmdi_resources, $directory_resources);
        if (!$result) {
            throw new CmdiHandlerException(t("Failed adding new resources to Cmdi"));
        }

        $result = $this->updateExistingResources($cmdi_resources, $directory_resources);

        if (!$result) {
            throw new CmdiHandlerException(t("Failed updating cmdi resources"));
        }
    }

    /**
     * Call to fits REST API allowing to determine the mime type of a specified file.
     * Returns FALSE if file is not accessible, FITS service returns wrong response code or the format attribute within
     * the xml file returned by fits service is not set.
     *
     * @param string $filename name of the file to be checked
     *
     * @return bool|string
     *
     */
    public static function fits_mimetype_check($filename)
    {

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
                CURLOPT_TIMEOUT => 300,));

        $result = curl_exec($ch);
        $httpcode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
        curl_close($ch);

        if ($httpcode < 200 OR $httpcode >= 300) {
            return FALSE;
        }

        $xml = simplexml_load_string($result);

        if (!isset($xml->identification->identity['mimetype'])) {
            return FALSE;
        }

        return (string)$xml->identification->identity['mimetype'];
    }

    /**
     * get mime-type of a file using the linux file command. Much quicker but less reliable than calling FITS,
     * however mime types are being re-evaluated using FITS in the doorkeeper anyway.
     *
     * @param string $filename name of the file to be checked
     *
     * @return bool|string
     *
     */
    public static function linux_mimetype_check($file)
    {
        $result = trim(shell_exec("file -b --mime-type \"$file\""));
        return $result;
    }


    function add_attribute_tree_to_xml($data, &$xml_data)
    {
        foreach ($data as $key => $value) {
            if (is_array($value)) {
                $subnode = $xml_data->$key;
                add_attribute_tree_to_xml($value, $subnode);
            } else {
                $xml_data->addAttribute($key, $value);
            }
        }
    }


    /**
     * function definition to convert an array to xml. Don't use for attributes, use add_attribute_tree_to_xml instead
     *
     * @param $data php array
     * @param $xml_data simplexml object for which new child branches are created
     */
    function array_to_xml($data, &$xml_data)
    {
        foreach ($data as $key => $value) {
            if (is_array($value)) {
                if (is_numeric($key)) {
                    $key = 'item'. $key; //dealing with <0/>..<n/> issues
                }
                $subnode = $xml_data->addChild($key);
                $this->array_to_xml($value, $subnode);
            } else {
                $xml_data->addChild("$key", htmlspecialchars("$value"));
            }
        }
    }

    // function definition to convert array to xml
    function array_to_xml_original($data, &$xml_data)
    {
        foreach ($data as $key => $value) {
            if (is_array($value)) {
                if (is_numeric($key)) {
                    $key = 'item'. $key; //dealing with <0/>..<n/> issues
                }
                $subnode = $xml_data->addChild($key);
                array_to_xml($value, $subnode);
            } else {
                $xml_data->addChild("$key", htmlspecialchars("$value"));
            }
        }
    }

    function canBeValidated()
    {

        $name = $this->getNameById();
        if (!file_exists(self::FORM_TEMPLATES_PATH . $name . '.xsd')) {
            throw new CmdiHandlerException("Could not find schema file for cmdi profile " . $name);
        }

        if (!file_exists(self::FORM_TEMPLATES_PATH . $name . '.xml')) {
            throw new CmdiHandlerException("Could not find profile form file for cmdi profile " . $name);
        }

        return TRUE;
    }


    public function stripResourceProxyAndResources()
    {
        // Strip ResourceProxy
        $proxies = $this->xpath('cmd:Resources/cmd:ResourceProxyList/cmd:ResourceProxy');


        foreach ($proxies as $proxy) {
            $rid = $proxy['id'];
            // remove all references to this proxy
            // (removing entire ref attribute even though it can have multiple values, because we're stripping all resources anyway)
            $nodes = $this->xpath("//cmd:*[contains(@ref, '$rid')]");
            foreach ($nodes as $node) {
                unset($node[0]['ref']);
            }
            unset($proxy[0]);
        }


        $profile = $this->getNameById();
        $xml = simplexml_load_file(CmdiHandler::FORM_TEMPLATES_PATH . "/$profile.xml");

        $tags = $xml->xpath('/profileToDrupal/resources/allowed_tags/tag');

        $node = $this->getNodeForAddingResources($this);

        if ($node === FALSE) {
            return FALSE;
        }

        foreach ($tags as $tag) {
            $entries = $node->xpath('cmd:' . (String)$tag['id']);
            foreach ($entries as $entry) {
                unset($entry[0]);
            }
        }
        return TRUE;
    }


    /**
     * Removes specified resources
     *
     * Note!! Removes all elements that have a "ref" attribute with the particular resourceID value.
     *
     * This may be inappropriate for some CMDI profiles (e.g. in case of obligatory elements)
     *
     * @param $resourceIDs array of resource IDs of the resources to be removed
     */
    public function stripResources($resourceIDs)
    {
        foreach ($resourceIDs as $rid) {
            $proxy = $this->xpath("cmd:Resources/cmd:ResourceProxyList/cmd:ResourceProxy[@id='$rid']");
            unset($proxy[0][0]);
            if (null !== ($this->xpath("//cmd:*[@ref='$rid']"))) {
                $component = $this->xpath("//cmd:*[@ref='$rid']");
                unset($component[0][0]);
            }
        }
    }


    /**
     * Adds new resources from the data directory to the cmdi object.
     *
     * @param $cmdi_resources array of existing resources already present in the cmdi object
     *
     * @param $directory_resources array of new resources in the data directory, to be added to the cmdi object
     *
     */
    private function addNewDirectoryResources($cmdi_resources, $directory_resources)
    {
        // prepare
        $profile = $this->getNameById();
        $xml = simplexml_load_file(CmdiHandler::FORM_TEMPLATES_PATH . "/$profile.xml");

        $targetNode = $this->getNodeForAddingResources($this);

        if ($targetNode === FALSE) {
            return FALSE;
        }

        // Add new resources to ResourceProxyList
        $new_resources = array_diff_key($directory_resources, $cmdi_resources);
        $resourceProxyList = $this->xpath('cmd:Resources/cmd:ResourceProxyList')[0];

        foreach ($new_resources as $rid => $file_name) {
            $file_mime = self::linux_mimetype_check(drupal_realpath($file_name));
            if (!$file_mime) {
                return FALSE;
            }
            $proxy = $resourceProxyList->addChild("cmd:ResourceProxy", '', "http://www.clarin.eu/cmd/");
            $proxy->addAttribute('id', $rid);
            $proxy->addChild("ResourceType", "Resource");
            $proxy->children('cmd', TRUE)->ResourceType->addAttribute('mimetype', $file_mime);
            $proxy->addChild("ResourceRef");
            $proxy->children('cmd', TRUE)->ResourceRef->addAttribute('lat:localURI', 'file:' . $file_name, "http://lat.mpi.nl/");

            // Create new DOMElements from the two SimpleXMLElements
            $tag = $this->getTag($xml, $file_mime);

            if (!$tag) {
                return FALSE;
            }

            $id = (String) $tag['id'];
            $sourceResource = simplexml_load_string("<cmd:$id xmlns:cmd=\"http://www.clarin.eu/cmd/\"></cmd:$id>");

            $sourceResource->addAttribute('ref', $rid);

            foreach ($tag->updated_values as $value) {
                $child_id = (String) $value->attributes();

                if ((String) $value[0] != '') {
                    $sourceResource->addChild('cmd:' . $child_id, (String) $value[0]);
                    continue;
                }

                if (strtolower($child_id) == 'size') {
                    $sourceResource->addChild('cmd:' . $child_id, filesize($file_name));
                    continue;
                }

                if (strtolower($child_id) == 'format') {
                    $sourceResource->addChild('cmd:' . $child_id, $file_mime);
                    continue;
                }
                if (strtolower($child_id) == 'type') {
                    $sourceResource->addChild('cmd:' . $child_id, explode('/', $file_mime)[0]);
                    continue;
                }
            }

            if (!$sourceResource) {
                return FALSE;
            }

            $resource = dom_import_simplexml($sourceResource);
            $insertBefores = $tag->xpath('insert_before');
            $existingChildren = $targetNode->children('cmd', TRUE)->xpath('child::node()');

            switch (TRUE) {
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
                    $found = FALSE;
                    foreach ($insertBefores as $insertBefore) {
                        $results = $targetNode->xpath('cmd:' . $insertBefore);
                        if (!empty($results)) {
                            $parent = $results[0];
                            $found = TRUE;
                            break;
                        }
                    }
                    if ($found) {
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
        return TRUE;
    }



    private function getTag($profile, $file_mime) {

        $tags = $profile->xpath('/profileToDrupal/resources/allowed_tags/tag');
        foreach ($tags as $tag) {
            $pattern = $tag->mime_regex[0];
            if (preg_match($pattern, $file_mime)) {
                return $tag[0];
            }
        }
        return FALSE;
    }

    private function getResourceInComponentSection($cmdi, $profile, $resource_id)
    {


        $root = $cmdi->getNodeForAddingResources($cmdi);

        if ($root === FALSE){
            return FALSE;
        }

        $tags = $profile->xpath('/profileToDrupal/resources/allowed_tags/tag');

        foreach ($tags as $tag) {
            $query = 'cmd:' . (String)$tag[0]['id'] . '[@ref=\'' .$resource_id . '\']';
            $results = $root[0]->xpath($query);

            if (!empty($results)) {
                return $results[0];
            }
        }
        return FALSE;
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

        if (!empty($resourceProxyList->children()) OR !empty($resourceProxyList->children('cmd', TRUE))) {

            // Create new DOMElements from the two SimpleXMLElements
            $domxml = dom_import_simplexml($this->Resources->ResourceProxyList);

            foreach ($resourceProxyList->children('cmd', TRUE) as $resource) {
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

        return TRUE;
    }

    /**
     * Allows to add resources to the componentes section of a otherwise populated cmdi file
     *
     * @param $sourceCmdi CmdiHandler a second cmdi file with resources to be added to the Components section
     */
    public function addResourceSectionToComponents($sourceCmdi)
    {
        $this->addResourcesToComponents($sourceCmdi);
    }

    /**
     *
     * @param $sourceCmdi
     */
    private function addResourcesToComponents($sourceCmdi)
    {
        //Copy resources to Components
        $searchNode = $this->getNodeForAddingResources($sourceCmdi);
        $profile = $sourceCmdi->getNameById();
        $xml = simplexml_load_file(CmdiHandler::FORM_TEMPLATES_PATH . "/$profile.xml");
        $tags = $xml->xpath('/profileToDrupal/resources/allowed_tags/tag');

        foreach ($tags as $tag) {
            $entries = $searchNode->xpath((String)$tag['id']);
            foreach ($entries as $entry) {
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
            foreach ($entries as $entry) {
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

    private function getResourceValue($resource)
    {
        $ref = $resource->xpath('cmd:ResourceRef')[0];
        $lat_attributes = $ref->attributes('lat', TRUE);

        if (isset($lat_attributes['localURI'])) {
            return basename($lat_attributes['localURI']);
        }

        /*
        if (isset($ref)){
            return basename((String)$ref);
        }
        */

        if (isset($lat_attributes->flatURI)) {
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
                $pid = FALSE;
            }
        }

        $fObj = islandora_object_load($pid);

        $value = $fObj ? $fObj->label : "";

        return $value;
    }

    private function getCmdiResources()
    {
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

    private function getDirectoryResources($directory, $existing_resources = array())
    {
        $resources = array();
        $c=10000; # counter for resource ID (each resource within an CMDI-object needs an unique identifier)

        //$rii = new RecursiveIteratorIterator(new RecursiveDirectoryIterator($directory,RecursiveDirectoryIterator::FOLLOW_SYMLINKS));

        $rii = new RecursiveIteratorIterator(
            new RecursiveDotFilterIterator(
                new RecursiveDirectoryIterator($directory, RecursiveDirectoryIterator::FOLLOW_SYMLINKS)
            )
        );

        foreach ($rii as $file) {
            if ($file->isDir()) {
                continue;
            }

            $file_name = drupal_realpath($file->getPathname());

            $resource_id = array_search(basename($file_name), $existing_resources);

            // generate new ID if resource name is unknown
            if (!$resource_id) {
                $unique = FALSE;

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

    private function checkAccessibilityFiles($resources)
    {
        $inaccesible_files = [];
        foreach ($resources as $rid => $file_name) {
            $fName = str_replace("\\\\", "\\", $file_name);

            if (!is_readable($fName)) {
                $inaccesible_files [] = basename($fName);
            }
        }
        return $inaccesible_files;
    }

    private function updateExistingResources($cmdi_resources, $directory_resources)
    {
        // Add lat:flatURI attribute to updated resources in ResourceProxyList
        $resourceProxyList = $this->xpath('cmd:Resources/cmd:ResourceProxyList')[0];

        $existing_resources = array_intersect_key($directory_resources, $cmdi_resources);

        foreach ($existing_resources as $rid => $file_name) {
            $file_mime = self::linux_mimetype_check(drupal_realpath($file_name));
            if (!$file_mime) {
                return FALSE;
            }

            $proxy = $resourceProxyList->xpath('cmd:ResourceProxy[@id="'. $rid .'"]');
            if (empty($proxy)) {
                return FALSE;
            }

            // change value of existing ResourceRef attribute or add new one
            $attributes = $proxy[0]->children('cmd', TRUE)->ResourceRef->attributes('lat', TRUE);
            if (isset($attributes['localURI'])) {
                $attributes['localURI'] = 'file:' . $file_name;
            } else {
                $proxy[0]->children('cmd', TRUE)->ResourceRef->addAttribute('lat:localURI', 'file:' . $file_name, "http://lat.mpi.nl/");
            }

            // delete value of the resourceproxy "ResourceRef" in case it is not a handle
            $value = (String)$proxy[0]->children('cmd',TRUE)->ResourceRef;
            if(!empty($value)){
                if (strpos($value,'http://hdl') !== 0 && strpos($value,'https://hdl') !== 0 && strpos($value,'hdl:') !== 0) {
                    $proxy[0]->children('cmd',TRUE)->ResourceRef = '';
                }
            }

            //Update resources in Components section
            $profile = $this->getNameById();
            $xml = simplexml_load_file(CmdiHandler::FORM_TEMPLATES_PATH . "/$profile.xml");

            $resource = $this->getResourceInComponentSection($this, $xml, $rid);

            if (!$resource) {
                return FALSE;
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
        return TRUE;
    }


    private function getNodeForAddingResources($cmdi)
    {
        $profile = $cmdi->getNameById();
        $xml = simplexml_load_file(CmdiHandler::FORM_TEMPLATES_PATH . "/$profile.xml");

        $resources_root_path = (!empty($xml->xpath('/profileToDrupal/resources/root_path'))) ? (String)$xml->xpath('/profileToDrupal/resources/root_path')[0] : FALSE;
        $resources_root_node = $cmdi->children('cmd', TRUE)->Components->{$resources_root_path};

        if (empty($resources_root_node)) {
            return FALSE;
        }

        // Return subNode if this is set for the specific profile
        $subnodes = (!empty($xml->xpath('/profileToDrupal/resources/subnodes'))) ? (String)$xml->xpath('/profileToDrupal/resources/subnodes')[0] : FALSE;

        if (!empty($subnodes)) {
            // add subnode if this node does not exist and return FALSE if this is not possible
            if (empty($resources_root_node[0]->xpath('cmd:' .$subnodes))) {
                $resources_root_node[0]->addChild('cmd:' . $subnodes, '', 'http://www.clarin.eu/cmd/');
            }

            if (empty($resources_root_node[0]->xpath('cmd:' .$subnodes))) {
                return FALSE;
            }

            return $resources_root_node[0]->xpath('cmd:' .$subnodes)[0];
        }

        return $resources_root_node[0];
    }

    /**
    * Set the title of a CMDI XML object. Title field is retrieved from form templates
    * @param string $title New title for the CMDI object
    *
    */
    public function setTitle($title)
    {
        $profile = $this->getNameById();
        $xml = simplexml_load_file(CmdiHandler::FORM_TEMPLATES_PATH . "/$profile.xml");
        $title_element = $xml->xpath("/profileToDrupal/items/item[@id='title_field']");
        $title_element_name = (string)$title_element[0]->attributes()->{'name'};
        $cmdi_title_element = $this->xpath("//cmd:Components/cmd:*/cmd:$title_element_name");
        $cmdi_title_element[0][0] = $title;
    }

    public function logging($message)
    {
            error_log(date(DATE_ATOM) . "\t" . $message ."\n", $message_type = 3 , '/easylat/www/htdocs/cmdi_handler.log');
    }
}
