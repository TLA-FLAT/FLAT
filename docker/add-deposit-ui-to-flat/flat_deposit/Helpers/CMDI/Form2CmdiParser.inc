<?php


class Form2CmdiParserException extends Exception{}

/**
 * Class for building a CMDI xml based on drupal form data and a form template
 */
class Form2CmdiParser
{


    /**
     * @var $components SimpleXMLElement object of the xml form template
     */
    private $template;

    /**
     * @var $components SimpleXMLElement object containing the basic tree of the cmdi without components
     */
    private $tree;

    /**
     * @var $components SimpleXMLElement object containing the components part of the cmdi
     */
    private $components;

    /**
     * @var $cmdi SimpleXMLElement object containing the fully generated cmdi
     */
    private $cmdi;


    /**
     * @param $profile_name string Name of the profile
     *
     * @param $template SimpleXMLElement

     * @param $user_name string name of the user generating the form

     * @param $form_data array data of the drupal form
     *
     * @return SimpleXMLElement cmdi file
     */
    public function buildCmdi($profile_name, $template, $user_name, $form_data){

        $clarin_id = $form_data['data']['clarinProfileId'];
        $this->template = $template;

        $this->initCmdiTree($clarin_id, $user_name) ;

        $this->_populateComponentNode($profile_name, $form_data) ;
        $this->_mergeTreeAndComponents() ;

        return $this->cmdi;


    }


    private function initCmdiTree($id, $user_name){

        module_load_include('php', 'flat_deposit', 'inc/xml_functions');

        $this->tree = new SimpleXMLElement_Plus('<CMD/>');


        // add processing instructions
        $processing_instruction = array (
            0 => 'xml-stylesheet',
            1 => 'type="text/xsl" href="/cmdi-xslt-1.0/browser_cmdi2html.xsl"');
        $this->tree->addProcessingInstruction($processing_instruction[0], $processing_instruction[1]);

        // add attributes
        $xsd = 'http://www.clarin.eu/cmd/ http://catalog.clarin.eu/ds/ComponentRegistry/rest/registry/profiles/' . $id . '/xsd';
        $CMD_attributes = array(
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
            'xmlns:xsi:schemaLocation' => $xsd,
        );

        add_attribute_tree_to_xml($CMD_attributes,$this->tree);




        // add (almost) empty xml data fields (=tree)
        $basis_tree = array(
            'Header' => array(
                'MdCreator' => $user_name,
                'MdCreationDate' => date("Y-m-d"),
                'MdSelfLink' => '',
                'MdProfile' => $id,
            ),
            'Resources' => array(
                'ResourceProxyList' => '',
                'JournalFileProxyList' => '',
                'ResourceRelationList' => '',
                'IsPartOfList' => ''),
            );
        array_to_xml($basis_tree,$this->tree);
    }


    /**
     * Combines the template with form data to create a populated xml file.

     * @param $profile_name String

     * @param $data
     */
    private function _populateComponentNode($profile_name, $data){


        $this->components = simplexml_load_string('<Components></Components>');
        $this->components->addChild($profile_name);


        // add child elements for each field specified in the template
        foreach ($this->template->fields->field as $field) {

            $id = (string)$field['id'];
            $name = (string)$field['name'];

            // get data from form
            if (isset($field->subNode)){
                $arrayKey = (string)$field->subNode[0];
                $field_data =  $data[$arrayKey][$id];

            } else {
                $field_data =  $data[$id];
            }


            // get data from form
            $values = (array)$field_data; // convert to array in order to unify input
            $no_button_values = array_diff_key($values, ['add'=>'add', 'remove'=>'remove']);
            $only_entered_values = array_filter($no_button_values, function($value) { return $value !== ''; });
            if (!empty($only_entered_values)){

                $converted_values = $this->convertValues($field->type, $only_entered_values); // convert values into xml - compatible format

            } else {

                continue;
            };

            // correctly set the xml tree parent location where child will be created
            if (isset($field->subNode)){

                $base = $this->components->$profile_name;

                foreach ($field->subNode as $subNode){

                    if (!isset($base->$subNode)){
                        $base->addChild($subNode);
                    }
                    $base = $base->$subNode;
                }

                $parent = $base;

            } else {
                $parent = $this->components->$profile_name;
            }


            // add array values val by val as children to $parent
            if ($converted_values){

                foreach ($converted_values as $val){
                    $parent->addChild($name,$val);
                }
            }
        }

    }


    private function convertValues($type, array $values){

        switch ($type){
            case 'date':
                $month = (strlen($values[0]['month']) == 2 ) ? $values[0]['month'] : '0'. $values[0]['month'];
                $day = (strlen($values[0]['day']) == 2 ) ? $values[0]['day'] : '0'. $values[0]['day'];
                $date = $values[0]['year'] . '-' . $month . '-' . $day;
                $converted = array($date);
                break;
            default :
                $converted = $values;
        }

        return $converted;

    }

    private function _mergeTreeAndComponents()
    {

        // convert xml to string without header
        $domComponents = dom_import_simplexml($this->components);
        $string = $domComponents->ownerDocument->saveXML($domComponents->ownerDocument->documentElement);


        $domTree = dom_import_simplexml($this->tree)->ownerDocument;
        $fragment = $domTree->createDocumentFragment();
        $fragment->appendXML($string);

        $domTree->documentElement->appendChild($fragment);

        #$fName = drupal_get_path('module','flat_deposit') .  '/Helpers/CMDI/Profiles/GENERATED_EXAMPLE.xml';
        #$domTree->save($fName);
        $this->cmdi = simplexml_import_dom($domTree);

    }
}