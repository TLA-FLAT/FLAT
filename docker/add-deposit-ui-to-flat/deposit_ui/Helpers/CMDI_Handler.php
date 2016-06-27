<?php

/**
 * Created by PhpStorm.
 * User: danrhe
 * Date: 09/06/16
 * Time: 12:24
 */
class CMDI_Handler

{
    public $bundle;
    public $template;
    public $field_name;
    public $handle;
    public $xml;
    public $export_file;
    public $prefix;
    public $files_info;
    public $form_data;
    // ToDo: Implement that we define a form_fields array that contains all fields per template that can be filled in by a researcher
    public $form_fields;

    /**
     * CMDI_creator constructor.
     * @param $bundle
     * @param $template
     */
    public function __construct($bundle, $template)
    {
        $this->bundle = $bundle;
        $this->template = $template;
        $this->field_name = 'lat-' . $template;

        $this->export_file = USER_DRUPAL_DATA_DIR .'/' . $bundle . '/metadata/'. $bundle . '.cmdi';


        #$import_file = drupal_realpath(drupal_get_path('module', 'flat_deposit_ui')) . '/inc/CMDI_templates/' . $template . '.cmdi';
    
    }

    /**
     * This method creates or loads an CMDI xml file
     */
    function getXML(){

    if ($this->projectCmdiFileExists()){
        $this->xml = simplexml_load_file($this->export_file);
    } else { $this->initiateNewCMDI(); }


}

    /**
     * Methods that generates a new, valid CMDI file including processing instructions, empty data fields and attributes
     */
    function initiateNewCMDI(){

        module_load_include('php', 'flat_deposit_ui', 'inc/xml_functions');
        $this->xml = new SimpleXMLElement_Plus('<CMD/>');

        // add processing instructions
        $processing_instruction = get_processing_instructions() ;
        $this->xml->addProcessingInstruction($processing_instruction[0], $processing_instruction[1]);

        // add attributes
        $CMD_attributes = get_attributes ("CMDI") ;
        add_attribute_tree_to_xml($CMD_attributes,$this->xml);

        // add (almost) empty xml data fields (=tree)
        $basis_tree = array(
            'Header' => array(
                'MdCreator' => '',
                'MdCreationDate' => '',
                'MdSelfLink' => '',
                'MdProfile' => 'clarin.eu:cr1:p_1407745712035',
            ),
            'Resources' => array(
                'ResourceProxyList' => '',
                'JournalFileProxyList' => '',
                'ResourceRelationList' => '',),
            'Components' => array(
                $this->field_name => '')
        );
        array_to_xml($basis_tree,$this->xml);

    }

    function projectCmdiFileExists ($fileName=NULL){
        $checkfile = (is_null($fileName)) ? $this->export_file : $fileName;
        return file_exists($checkfile);
    }

    function createCompleteXmlFile()
    {
        $this->handle = $this->getHandle();

        $this->changeHeader();

        $this->addIngestFileInfoToXml();

        $this->addComponentInfoToXml();

    }

    public function getHandle()
    {
        $this->handle = get_example_handle($this->bundle);
    }

    function addIngestFileInfoToXml(){

        $files = $this->getIngestFileInfo();
        foreach ($files as $file) {

            $data = $this->xml->Resources->ResourceProxyList->addChild('ResourceProxy');
            $data->addAttribute('id', $file['attributes']['id']);
            $data->addChild('ResourceType', $file['ResourceType']);
            $data->addChild('ResourceRef', $file['ResourceRef']);

            // optional attributes
            if (array_key_exists('mimetype', $file['attributes'])) $data->ResourceType->addAttribute('mimetype', $file['attributes']['mimetype']);
            if (array_key_exists('localURI', $file['attributes'])) {
                $data->ResourceRef->addAttribute("xmlns:lat:localURI", $file['attributes']['localURI']);
            }
        }
    }

    function changeHeader()
    {
        $this->xml->Header->MdCreator = USER;
        $this->xml->Header->MdCreationDate = format_date(time(), 'custom', 'Y-m-d');;
        $this->xml->Header->MdSelfLink = $this->prefix . $this->handle;
    }


    /**
     * This method transforms drupal form data into valid cmdi meta data.
     * Particularly, (1) date is formatted (2) tree info is changed (e.g. move Components/field_1/Name to Components/Name)). Only template specific data is added to the class

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

        // add all template specific fields to the xml
        foreach ($form_data['form_fields_template'] as $field){
            if ($field != "field_1"){
                $clean_data[$field] = $form_data[$field];}
        }
        $this->form_data = $clean_data;

    }


    /**
     * Partial example data as found in a  cmdi-file
     * @return array
     */
    function getIngestFileInfo()
    {
        return get_example_ingest_file_info($this->bundle);
    }


    /**
     * Transforms array to xml tree
     */
    function addComponentInfoToXml()
    {
        module_load_include('php', 'flat_deposit_ui', 'inc/xml_functions');

        $data = $this->xml->Components->{$this->field_name};
        array_to_xml($this->form_data, $data);
    }

}

function get_example_ingest_file_info($bundle, $prefix='hdl:'){

    $files = array();

    switch ($bundle) {
        case 'First_try':
            $files [] = array(
                'attributes' => array(
                    'id' =>'d1814511e175',
                    'mimetype' => 'video/mp4',
                    'localURI' => '../Media/The_bushfire.mp4'),
                'ResourceType' => 'Resource',
                'ResourceRef' => $prefix . '11142/00-E21BA440-7EB1-4B7D-9F4D-0FB50FD6B2BC');

            $files [] = array(
                'attributes' => array(
                    'id' =>'d1814511e14',
                    'localURI' => '../Media/The_bushfire.pfsx'),
                'ResourceType' => 'Resource',
                'ResourceRef' => $prefix . '11142/00-4D3B46CF-99D0-4F63-A96C-96B055E86588');


            $files [] = array(
                'attributes' => array(
                    'id' =>'d1814511e16',
                    'localURI' => '../Info/The_bushfire.eaf'),
                'ResourceType' => 'Resource',
                'ResourceRef' => $prefix . '11142/00-150F188B-2156-436C-BBF7-52A513BD47DA');

            $files [] = array(
                'attributes' => array(
                    'id' =>'landingpage',),
                'ResourceType' => 'LandingPage',
                'ResourceRef' => $prefix . '11142/00-E3C00A6B-CFB2-4BE4-BB90-E410A36F5F3B' . '@view');

        case 'DvR_Sandbox':

            $files [] = array(
                'attributes' => array(
                    'id' =>'d111111',
                    'mimetype' => 'audio/x-mpeg3',
                    'localURI' => '../data/Media/Test.mp3'),
                'ResourceType' => 'Resource',
                'ResourceRef' => $prefix . '1839/00-0000-0000-0215-4A87-5');

            $files [] = array(
                'attributes' => array(
                    'id' =>'d111112',
                    'mimetype' => 'application/pdf',
                    'localURI' => '../data/Media/Test.pdf'),
                'ResourceType' => 'Resource',
                'ResourceRef' => $prefix . '1839/00-0000-0000-0315-4A87-5');

            $files [] = array(
                'attributes' => array(
                    'id' =>'landingpage',),
                'ResourceType' => 'LandingPage',
                'ResourceRef' => $prefix . '1839/00-0000-0000-0415-4A87-5');


    }

    return $files;
}



/**
 * Returns a form array with fields that may be filled in by researchers to generate metadata to be archived together with their data
 *
 * @param string $template  The name of the template to be used (at the moment options are experiment, session, and minimal
 * @param array $md     Passed meta data will be used to fill the form with default values
 * @return array $form which will be appended to a form array in drupal and rendered.
 */
function get_template_form($template, $bundle, $md=NULL)
{
    module_load_include('inc', 'flat_deposit_ui', 'inc/CMDI_templates');

    $tmp = new CMDI_templates($template);
    $tmp->getTemplate($md);

    $form = $tmp->fields;

    return $form;
}



function generate_drupal_form($form, $template){
}


function get_example_md ($bundle){
    switch ($bundle) {

        case 'DvR_Sandbox':
            $md = array(
                'field_1' => array(
                    'Name' => 'DvR_Sandbox',
                    'Title' => 'The DvR_Sandbox',
                    'Date' => array(
                        'day' => 25,
                        'month' =>5,
                        'year' => 2016),
                    'descriptions' => array(
                        'Description' => 'The is a test dataset and Daniels favorite dog is called James',)
                ),

                'Location' => array(
                    'Continent'=>'Europe',
                    'Country'=> 'The Netherlands',
                    'Region'=> 'House of Language',
                    'Address'=> 'Wundtlaan 1',
                ),

                'Project' => array(
                    'Name' =>'FLAT archive',
                    'Title' =>'Deposit module',
                    'Id' =>'',
                    'Contact' => array(
                        'Name' => 'Daniel',
                        'Email' => 'Daniel@email.nl',
                        'Organisation' => 'MPI Nijmegen',
                    ),
                    'descriptions' => array(
                        'Description' => 'FLAT Test set')
                )
            );

    }

    return $md;
}

function get_example_handle ($bundle)
{
    switch ($bundle) {
        case 'First_try':
            $handle = '11142/00-E3C00A6B-CFB2-4BE4-BB90-E410A36F5F3B';

        case 'DvR_Sandbox':
            $handle = '1839/00-0000-0000-0415-4A87-5';
    }
    return $handle;
}

function get_processing_instructions(){
    return array (
        0 => 'xml-stylesheet',
        1 => 'type="text/xsl" href="/cmdi-xslt-1.0/browser_cmdi2html.xsl"');
}

function get_attributes ($xml){
    switch ($xml){
        case "CMDI":
    return array(
        'xmlns' => "http://www.clarin.eu/cmd/",
        'xmlns:xmlns:imdi' => "http://www.mpi.nl/IMDI/Schema/IMDI",
        'xmlns:xmlns:cmd' => "http://www.clarin.eu/cmd/" ,
        'xmlns:xmlns:iso' => "http://www.iso.org/",
        'xmlns:xmlns:sil' => "http://www.sil.org/",
        'xmlns:xmlns:functx' => "http://www.functx.com",
        'xmlns:xmlns:xs' => "http://www.w3.org/2001/XMLSchema",
        'xmlns:xmlns:lat' => "http://lat.mpi.nl/",
        'xmlns:xmlns:xsi' => "http://www.w3.org/2001/XMLSchema-instance",
        'CMDVersion' => "1.1",
        'xmlns:xsi:schemaLocation' => "http://www.clarin.eu/cmd/ http://catalog.clarin.eu/ds/ComponentRegistry/rest/registry/profiles/clarin.eu:cr1:p_1407745712035/xsd");
    }
}

