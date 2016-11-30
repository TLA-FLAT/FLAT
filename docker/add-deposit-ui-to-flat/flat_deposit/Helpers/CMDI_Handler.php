<?php


class CMDI_Handler

{
    public $profile; // profile used to create metadata

    public $xml; // simpleXML element used to populate and export
    public $user;
    public $form_data; //data from the input form
    public $bundle; // node title info
    public $collection; // node field info

    public $resources; // files to add as resources to CMDI

    public $export_file; // path and name of the xml file storing the cmdi metadata




    /**
     * CMDI_handler constructor.
     * @param $node stdClass node object
     * @param $rpofile
     */
    public function __construct($node, $profile='default')
    {
        $this->profile = $profile;

        $wrapper = entity_metadata_wrapper('node', $node);
        $this->bundle = $node->title;
        $this->collection = $wrapper->upload_collection->value();

        $this->user = user_load($node->uid);

        $this->export_file = $this->determine_export_file($wrapper);
        
    }


    /**
     * Function dertermining location of CMDI. Depending on the bundles status this can be either the freeze directory or either the drupal data directory or an external directory
     * @param $wrapper
     * @return string
     */
    public function determine_export_file($wrapper){

        $status = $wrapper->upload_status->value();

        if ($status == 'open' || $status == 'failed'){

            $external = $wrapper->upload_external->value();

            if ($external) {

                $config = variable_get('flat_deposit_ingest_service');
                $location = $wrapper->upload_location->value();
                $path = $config['alternate_dir'] . $this->user->name . $config['alternate_subdir'] . "/$location";

            } else {

                $path = 'public://users/' . $this->user->name . "/$this->collection/$this->bundle";

            }

        } else {

            $freeze =  variable_get('flat_deposit_paths')['freeze'];
            $path = $freeze . '/' . $this->user->name . "/$this->collection/$this->bundle";

        }

        return $path . '/metadata/record.cmdi';
    }

    /**
     * Methods checking whether a cmdi file is found at a certain location
     *
     * @param $fileName null if a additional parameter is provided the method looks at differnent than standard location
     *
     * @return bool
     */
    function projectCmdiFileExists ($fileName=NULL){
        $checkfile = (is_null($fileName)) ? $this->export_file : $fileName;
        return file_exists($checkfile);
    }


    /**
     * This method either loads a simpleXML object from an existing CMDI file or generates a CMDI tree with only the basic nodes.
     */
    function getXML(){

    if ($this->projectCmdiFileExists()){

        $this->xml = simplexml_load_file($this->export_file);

    } else {

        $this->initiateNewCMDI();

    }
}

    /**
     * Methods that generates a new, valid CMDI file including processing instructions, empty data fields and attributes
     */
    function initiateNewCMDI(){

        if (!$this->profile){
            throw new Exception('Profile has not been specified');
        }

        $config = variable_get('flat_deposit_metadata');
        module_load_include('php', 'flat_deposit', 'inc/xml_functions');

        $this->xml = new SimpleXMLElement_Plus('<CMD/>');

        // add processing instructions
        $processing_instruction = get_processing_instructions($config['site']) ;
        $this->xml->addProcessingInstruction($processing_instruction[0], $processing_instruction[1]);

        // add attributes
        $CMD_attributes = get_attributes ($config['site']) ;
        add_attribute_tree_to_xml($CMD_attributes,$this->xml);


        //
        $components_field_value = $config ['prefix'] . '-' . $this->profile;

        // add (almost) empty xml data fields (=tree)
        $basis_tree = array(
            'Header' => array(
                'MdCreator' => '',
                'MdCreationDate' => '',
                #'MdSelfLink' => '',
                'MdProfile' => $config['MdProfile'],
            ),
            'Resources' => array(
                'ResourceProxyList' => '',
                'JournalFileProxyList' => '',
                'ResourceRelationList' => '',
                'IsPartOfList' => ''),
            'Components' => array(
                $components_field_value => '')
        );
        array_to_xml($basis_tree,$this->xml);
    }




    /**
     * This function recursively    searches for files in the user data directory
     *
     * @param $directory string     path where to search for files
     *
     * @returns $resources array    file id's as keys and file names as values
     */
    public function searchDir($directory){


        $rii = new RecursiveIteratorIterator(new RecursiveDirectoryIterator($directory,RecursiveDirectoryIterator::FOLLOW_SYMLINKS));

        $resources = array();
        $c=10000; # counter for resource ID (each resource within an CMDI-object needs an unique identifier)


        foreach ($rii as $file) {
            if ($file->isDir()){
                continue;}

            $c++;
            $fid = "d$c";
            $resources[$fid] = $file->getPathname();

        }

        return $resources;


    }


    /**
     * This method fills the resources section of the cmdi file with all files found in the open bundle data directory
     *
     * <ResourceRef>../data/Write_me.txt</ResourceRef>
     */
    function addResourcesToXml(){

        foreach ($this->resources as $fid => $file) {

            // transform drupal path to relative path as needed for fedora ingest
            #$localURI = str_replace(USER_FREEZE_DIR ."/" . $this->collection ."/" . $this->bundle, "..", $file);

            # Mimetype of the file;
            $mime = mime_content_type(drupal_realpath($file)) ;

            $data = $this->xml->Resources->ResourceProxyList->addChild('ResourceProxy');
            $data->addAttribute('id', $fid);
            $data->addChild('ResourceType', 'Resource');
            $data->addChild('ResourceRef', $file);

            $data->ResourceType->addAttribute('mimetype', $mime);

        }
    }



    function changeHeader()
    {
        $this->xml->Header->MdCreator = $this->user->name;
        $this->xml->Header->MdCreationDate = format_date(time(), 'custom', 'Y-m-d');;

    }


    /**
     * This method transforms drupal form data into valid cmdi meta data.
     * Particularly, (1) date is formatted (2) tree info is changed (e.g. move Components/field_1/Name to Components/Name)). Only profile
     *specific data is added to the class

     * @param $form_data array of meta data harvested from drupal form
     */
    function processFormData($form_data){

        $clean_data = array();
        $clean_data['Title'] = $form_data['field_1']['Title'];
        $clean_data['Name'] = $form_data['field_1']['Name'];

        if ($form_data['field_1']['Date']) {
            $month = (strlen($form_data['field_1']['Date']['month']) == 1 ) ? $form_data['field_1']['Date']['month'] : '0'.$form_data['field_1']['Date']['month'];
            $date = $form_data['field_1']['Date']['year'] . '-' . $month . '-' . $form_data['field_1']['Date']['day'];
        }
        else $date = NULL;

        $clean_data['Date'] = $date;
        $clean_data['Date'] = $date;

        // add all profile specific fields to the xml
        foreach ($form_data['form_fields_keys'] as $field){
            if ($field != "field_1"){
                $clean_data[$field] = $form_data[$field];}
        }
        $this->form_data = $clean_data;

    }


    /**
     * Transforms array to xml tree
     */
    function addComponentInfoToXml()
    {

        if (!$this->profile){
            throw new Exception('Profile has not been specified');
        }

        module_load_include('php', 'flat_deposit', 'inc/xml_functions');

        $config = variable_get('flat_deposit_metadata');
        $value = $config ['prefix'] . '-' . $this->profile;

        $data = $this->xml->Components->{$value};
        array_to_xml($this->form_data, $data);
    }

}

function get_processing_instructions($profile){
    switch ($profile){
        case "MPI":
    return array (
        0 => 'xml-stylesheet',
        1 => 'type="text/xsl" href="/cmdi-xslt-1.0/browser_cmdi2html.xsl"');
    }

}

function get_attributes ($site){
    switch ($site){
        case "MPI":
    return array(
        'xmlns:xmlns:xsi' => "http://www.w3.org/2001/XMLSchema-instance",
        'xmlns' => "http://www.clarin.eu/cmd/",
        'xmlns:xmlns:cmd' => "http://www.clarin.eu/cmd/" ,
        'xmlns:xmlns:imdi' => "http://www.mpi.nl/IMDI/Schema/IMDI",
        'xmlns:xmlns:lat' => "http://lat.mpi.nl/",
        'xmlns:xmlns:iso' => "http://www.iso.org/",
        'xmlns:xmlns:sil' => "http://www.sil.org/",
        'xmlns:xmlns:xs' => "http://www.w3.org/2001/XMLSchema",
        'xmlns:xmlns:functx' => "http://www.functx.com",
        'CMDVersion' => "1.1",
        'xmlns:xsi:schemaLocation' => "http://www.clarin.eu/cmd/ http://catalog.clarin.eu/ds/ComponentRegistry/rest/registry/profiles/clarin.eu:cr1:p_1407745712035/xsd");
    }
}

