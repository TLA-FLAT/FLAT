<?php

/**
 * Class IngestServiceException is an exception class
 */
class CMDIHandlerException extends Exception {}


class CMDI_Handler

{
    public $profile; // profile used to create metadata

    public $xml; // simpleXML element used to populate and export
    public $user;
    public $form_data; //data from the input form
    public $bundle; // node title info
    public $collection; // node field info

    public $resources; // files to add as resources to CMDI

    public $resource_directory; // path where all resources can be found

    public $flat_created_CMDI;



    /**
     * CMDI_handler constructor.
     * @param $node stdClass node object
     * @param $rpofile
     */
    public function __construct($node)
    {
        $this->user = user_load($node->uid);
        $this->wrapper = entity_metadata_wrapper('node', $node);

        $this->bundle = $node->title;
        $this->collection = $this->wrapper->upload_collection->value();

        $this->flat_created_CMDI = $this->wrapper->upload_cmdi_creator->value();

        $file_field = $this->wrapper->upload_cmdi->value();
        try
        {
            $this->file = file_load($file_field['fid']);

            $this->load_xml();

            $this->profile = $this->xml->Components->children()[0]->getName();

            $this->set_resource_directory();



            if ($this->flat_created_CMDI){

            } else {

            }

        }
        catch (CMDIHandlerException $e){

        }

    }


    public function load_xml(){

        $this->xml = simplexml_load_file(drupal_realpath($this->file->uri));
        return $this->xml;
    }


    // clean up all CMDI resources
    public function set_resource_directory(){

        $status = $this->wrapper->upload_status->value();

        // Determine paths

        $freeze_directory = 'freeze://' . "/" . $this->user->name . "/" . $this->wrapper->upload_collection->value() . '/' . $this->bundle . '/data';
        $drupal_directory = 'private://flat_deposit/data/' . $this->user->name . "/" . $this->wrapper->upload_collection->value() . '/' . $this->bundle;


        if ($status == 'failed' OR $status == 'open') {

            $this->resource_directory = $drupal_directory;

        } else {

            $this->resource_directory = $freeze_directory;
        }

    }

    // clean up all CMDI resources
    public function remove_all_resources()
    {
        // Removal existing resources from ResourceProxy child
        $nResources = $this->xml->Resources->ResourceProxyList->ResourceProxy->count();
        for ($i = 0; $i < $nResources; $i++) {
            unset($this->xml->Resources->ResourceProxyList->ResourceProxy[0]);
        }

        // Removal exitsing resources from Components->{profile}->Resources child
        $nResources = $this->xml->Components->{$this->profile}->Resources->count();
        for ($i = 0; $i < $nResources; $i++) {
            unset($this->xml->Components->{$this->profile}->Resources->MediaFile[0]);
        }

    }

    // master function for searching data directory, add resources to xml variable and update record.cmdi file
    public function add_all_resources(){

        // Add 'Resources'-child to Components->profile if not existing
        if (!isset($this->xml->Components->{$this->profile}->Resources)){
            $this->xml->Components->{$this->profile}->addChild('Resources');
        }

        // scan all files of the bundle freeze directory and add theses as resources to the CMDI;
        $this->resources = $this->searchDir($this->resource_directory);

        // Add resources to simplexml variable
        $this->addResourcesToXml();


    }


    // Update drupal managed file (i.e. record.cmdi)
    public function save_updated_xml()
    {
        file_save_data($this->xml->asXML(), $this->file->uri, FILE_EXISTS_REPLACE);

    }

    /**
     * This function recursively    searches for files in the data directory
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
            $resources[$fid] = drupal_realpath($file->getPathname());

        }

        return $resources;


    }


    /**
     * This method fills the resources section of the cmdi file with all files found in the open bundle data directory
     *
     * <ResourceRef>../data/Write_me.txt</ResourceRef>
     */
    function addResourcesToXml(){


        foreach ($this->resources as $fid => $file_name) {

            $file_mime = mime_content_type(drupal_realpath($file_name)) ;
            $file_size = filesize(drupal_realpath($file_name));

            // Add Resources to resource proxy list
            $data = $this->xml->Resources->ResourceProxyList->addChild('ResourceProxy');
            $data->addAttribute('id', $fid);

            $data->addAttribute('localURI', $file_name, 'lat');
            $data->addChild('ResourceType', 'Resource');

            $data->ResourceType->addAttribute('mimetype', $file_mime);


            // Add Resources to Components->resource proxy list
            $data2 = $this->xml->Components->{$this->profile}->Resources->addChild('MediaFile');
            $data2->addAttribute('ref', $fid);
            $data2->addChild('Type', 'document');
            $data2->addChild('Format', $file_mime);
            $data2->addChild('Size', $file_size);
        }

    }



    // clean up all CMDI resources
    public function remove_resourceRef_value()
    {
        // Removal existing resources from ResourceProxy child
        foreach ( $this->xml->Resources->ResourceProxyList->children() as $child) {
            if(isset($child->ResourceRef)) {
                $child->ResourceRef="";
            }
        }

    }


    function checkResources($data_dir){

        $nResources_cmdi = $this->xml->Resources->ResourceProxyList->count();

        $nResources_directory = 0;

        $rii = new RecursiveIteratorIterator(new RecursiveDirectoryIterator($data_dir,RecursiveDirectoryIterator::FOLLOW_SYMLINKS));

        foreach ($rii as $file) {
            if ($file->isDir()) {
                continue;
            }

            $nResources_directory++;

        }


        if ($nResources_cmdi != $nResources_directory){

            throw new CMDIHandlerException (t('Specified resource(s) in cmdi do not match with files found in data directory '));

        }

    }




    /**
     * Transforms the relative path of all resources to an absolute path
     *
     * <ResourceRef>./Write_me.txt</ResourceRef>
     */
    function resourceRel2FreezePathTransform(){


        foreach ( $this->xml->Resources->ResourceProxyList->children() as $child) {

            $rel_path = (string)$child->ResourceRef->attributes('lat', TRUE)->localURI;
            $abs_path = drupal_realpath('freeze://'. $this->user->name . '/' . $this->collection . '/' . $this->bundle) . '/data/'. $rel_path ;


            if (file_exists($abs_path)){

                unset($child->ResourceRef->attributes('lat', TRUE)->localURI);
                $child->ResourceRef = $abs_path;

            } else {

                throw new CMDIHandlerException ('Specified resource(s) in cmdi do not match with files found in data directory ');

            }
        }

    }

    /**
     * Transforms the absolute path of all resources to an absolute path
     * Replaces the <freeze-base>/<user-name>/<collection>/<bundle> from absolut path with empty string
     *
     */
    function ResourceFreeze2RelPathTransform(){


        foreach ( $this->xml->Resources->ResourceProxyList->children() as $child) {

            $abs_path = (string)$child->ResourceRef;
            $rel_path = str_replace(drupal_realpath('freeze://'. $this->user->name . '/' . $this->collection . '/' . $this->bundle) . '/data/', '' , $abs_path);

            if ($rel_path){

                $child->ResourceRef->addAttribute('lat:localURI', $rel_path, "http://lat.mpi.nl/");
                $child->ResourceRef = "";


            } else {

                throw new CMDIHandlerException ('Unable to create absolute path for resource');

            }
        }
    }






    /**
     *  Method for cleaning up isPartOf information
     *
     * @param null $parent_pid if specified only the specified child will be removed, otherwise all children will be removed
     */
    function removeIsPartOf($parent_pid=NULL)
    {
        // clean up all references to parents
        if (isset($this->xml->Resources->IsPartOfList)) {

            if ($parent_pid) {
                unset($this->xml->Resources->IsPartOfList->IsPartOf->{$parent_pid});
            } else {

                $c = $this->xml->Resources->IsPartOfList->IsPartOf->count();
                for ($i = 0; $i < $c; $i++) {
                    unset($this->xml->Resources->IsPartOfList->IsPartOf[0]);
                }
            }
        }

    }

    function addIsPartOf($parent_pid)
    {

        // Add isPartOf property to xml
        if(!isset($this->xml->Resources->IsPartOfList)){
            $this->xml->Resources->addChild('IsPartOfList') ;
        }
        $this->xml->Resources->IsPartOfList->addChild('IsPartOf',$parent_pid);

    }


    function remove_MdSelfLink()
    {
        if (isset($this->xml->Header->MdSelfLink)) {

            unset($this->xml->Header->MdSelfLink);
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
        $clean_data['Name'] = $form_data['field_1']['Name'];
        $clean_data['Title'] = $form_data['field_1']['Title'];

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
     * This method transforms CMDI meta data as php nested array to a structure that can be used for the cmdi-create form.
     * @param $form_data array of meta data harvested from drupal form
     */
    function transformCMDIToFormData($cmdi_data){

        $cmdi_data ['field_1']= array();

        $cmdi_data ['field_1']['Title'] = $cmdi_data ['Title'];
        $cmdi_data ['field_1']['Name'] = $cmdi_data ['Name'];

        unset($cmdi_data ['Title']);
        unset($cmdi_data ['Name']);


        // transform date to array of integers
        $cmdi_data ['field_1']['Date'] = array_combine(['Year', "Month",' Day'], array_map('intval', explode('-', $cmdi_data ['Date'])));
        unset($cmdi_data ['Date']);



        return $cmdi_data;
    }



    /**
     * Transforms array to xml tree
     */
    function addComponentInfoToXml()
    {

        if (!$this->profile){
            throw new Exception('Profile has not been specified');
        }

        $value =  $this->profile;

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

