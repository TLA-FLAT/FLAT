<?php

spl_autoload_register('autoload_cmdicreator');

function autoload_cmdicreator() {
    include drupal_get_path('module', 'flat_deposit') . '/Helpers/CMDI/CmdiCreator/class.CmdiComponent.inc';
    include drupal_get_path('module', 'flat_deposit') . '/Helpers/CMDI/CmdiCreator/class.CmdiComposite.inc';
    include drupal_get_path('module', 'flat_deposit') . '/Helpers/CMDI/CmdiCreator/class.CmdiLeaf.inc';

}


class CmdiCreatorException extends Exception {}
/**
 * Class for building a CMDI xml based on drupal form data and a form template
 */
class CmdiCreator
{


    const FORM_TEMPLATES_PATH = __DIR__ . '/../FormTemplates/';


    /**
     * @var $cmdi CmdiHandler object containing the fully generated cmdi
     */
    private $cmdi;





    /**
     * CmdiCreator constructor.
     */
    public function __construct(){}

    /**
     * Getter for Cmdi file
     * @return CmdiHandler
     */
    public function getCmdi(){
        return $this->cmdi;
    }

    /**
     * Generates a valid cmdi file.
     *
     * @param String $templateName  Name of the form profile
     *
     * @param array $form_data  data of the drupal form
     *
     * @param string $owner name of the user generating the form
     *
     */
    public function setCmdi($templateName, $form_data, $owner){


        try{
            $tree = $this->getTree($form_data, $owner);
            $components = $this->setCmdiComponents($templateName, $form_data) ;
            $this->cmdi = $this->mergeTreeAndComponents($tree, $components) ;
        } catch (CmdiCreatorException $e){
            throw new CmdiCreatorException($e->getMessage());
        }

    }



    /**
     * Combines template items with form data to create a populated xml file.
     *
     * @param $templateName Name of the used template.
     *
     * @param null $data
     */
    private function setCmdiComponents($templateName, $data = null){


        $fName = self::FORM_TEMPLATES_PATH . $templateName . '.xml';

        $template = simplexml_load_file($fName);

        if (!$template){
            throw new CmdiCreatorException('Unable to load template file');
        }

        $this->components = simplexml_load_string('<Components></Components>');
        $this->components->addChild($templateName);
        $rootNewComponents = $this->components->{$templateName};

        // add child elements for each field specified in the template
        foreach ($template->items->item as $component) {

            $className = "Cmdi" . (string)$component['type'];
            $cmdiComponent = new $className();

            $cmdiComponent->setCmdiComponent($rootNewComponents, $component, $data);

        }

        return $rootNewComponents;
    }


    /**
     * Recursively sets the form elements properties and adds nested items to the form element
     *
     * @param $newNode SimpleXMLElement cmdi xml node to which a new simplexml element will be added
     *
     * @param $item SimpleXMLElement xml node of render template providing all necessary info for element creation
     *
     * @param $form_data array data of form
     *
     */
    private function _setCmdiElement(&$newNode, $item, $form_data, $multivals)
    {
        $id = (string)$item['id'];
        $type = (string)$item['type'];
        $name = (string)$item['name'];

        if ($type == 'DrupalElement') {

            $values = $form_data[$id];
            if (isset($values['add'])) unset($values['add']);
            if (isset($values['remove'])) unset($values['remove']);
            if (isset($values['menu'])) unset($values['menu']);



            if (isset($values)) {
                foreach ($values as $value) {

                    if (!is_null($value)) {
                        $converted = $this->convertValue($item->type, $value);
                        $newNode->addChild($name, utf8_encode(htmlspecialchars($converted)));
                    }
                }
            }

            } else {


                if (isset($item->container)) {

                    $container = (string)$item->container;
                    $fieldset_data = $form_data[$container][$id];

                } else {

                    $fieldset_data = $form_data[$id];

                }

                $newNode->addChild($name);
                #$name = (string)$item['name'];
                if (isset($fieldset_data['add'])) unset($fieldset_data['add']);
                if (isset($fieldset_data['remove'])) unset($fieldset_data['remove']);

                foreach ($item->item as $newItem) {

                    foreach ($fieldset_data as $data){

                        $this->_setCmdiElement($newNode->{$name}, $newItem, $data, $multivals);

                    }


                }
            }


    }

        private function hasValue($fieldset){


            $outcomes = [];
            foreach ($fieldset as $value){

                if (!is_array($value)){

                    if (is_string($value) AND !empty($value)) {

                        $outcomes[] = TRUE;

                    } else {

                        $outcomes[] = FALSE;

                    }

                } else {

                    $outcomes [] = $this->hasValue($value);
                }
            }

            return in_array( TRUE, $outcomes);

        }



    private function mergeTreeAndComponents($tree, $components)
    {



        // convert xml to string without header
        $domComponents = dom_import_simplexml($components);
        $string = $domComponents->ownerDocument->saveXML($domComponents->ownerDocument->documentElement);


        $domTree = dom_import_simplexml($tree)->ownerDocument;
        $fragment = $domTree->createDocumentFragment();
        $fragment->appendXML($string);

        $domTree->documentElement->appendChild($fragment);

        #$fName = drupal_get_path('module','flat_deposit') .  '/Helpers/CMDI/Profiles/GENERATED_EXAMPLE.xml';
        #$domTree->save($fName);
        return simplexml_import_dom($domTree, 'CmdiHandler');

    }

    /**
     * Generates basic cmdi tree with all obligatory children and processing instructions
     *
     * @param $form_data
     *
     * @param $owner
     *
     * @return SimpleXMLElement
     */
    private function getTree($form_data, $owner){

        module_load_include('inc', 'flat_deposit', 'Helpers/CMDI/class.CmdiHandler');

        $clarinId = $form_data['data']['clarinProfileId'];

        $tree = simplexml_load_string('<CMD/>', 'CmdiHandler');

        // add processing instructions
        $processing_instruction = array (
            0 => 'xml-stylesheet',
            1 => 'type="text/xsl" href="/cmdi-xslt-1.0/browser_cmdi2html.xsl"');
        $tree->addProcessingInstruction($processing_instruction[0], $processing_instruction[1]);

        // add attributes
        $xsd = 'http://www.clarin.eu/cmd/ http://catalog.clarin.eu/ds/ComponentRegistry/rest/registry/profiles/' . $clarinId . '/xsd';
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

        $tree->add_attribute_tree_to_xml($CMD_attributes,$tree);


        // add (almost) empty xml data fields (=tree)
        $basis_tree = array(
            'Header' => array(
                'MdCreator' => $owner,
                'MdCreationDate' => date("Y-m-d"),
                'cmd:MdSelfLink' => '',
                'MdProfile' => $clarinId,
            ),
            'Resources' => array(
                'ResourceProxyList' => '',
                'JournalFileProxyList' => '',
                'ResourceRelationList' => '',
                'IsPartOfList' => ''),
        );
        $tree->array_to_xml($basis_tree,$tree);


        return $tree;
    }

}
