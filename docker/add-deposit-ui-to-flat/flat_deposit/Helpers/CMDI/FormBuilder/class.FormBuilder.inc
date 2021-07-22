<?php
spl_autoload_register('autoload_formbuilder');

function autoload_formbuilder()
{
    include_once drupal_get_path('module', 'flat_deposit') . '/Helpers/CMDI/FormBuilder/class.FormComponent.inc';
    include_once drupal_get_path('module', 'flat_deposit') . '/Helpers/CMDI/FormBuilder/class.FormComposite.inc';
    include_once drupal_get_path('module', 'flat_deposit') . '/Helpers/CMDI/FormBuilder/class.FormLeaf.inc';
    include_once drupal_get_path('module', 'flat_deposit') . '/Helpers/CMDI/FormBuilder/class.FormField.inc';
    include_once drupal_get_path('module', 'flat_deposit') . '/Helpers/CMDI/CmdiTemplate/class.CmdiModalBuilder.inc';
    include_once drupal_get_path('module', 'flat_deposit') . '/Helpers/CMDI/CmdiTemplate/class.CmdiActionsBuilder.inc';
    include_once drupal_get_path('module', 'flat_deposit') . '/Helpers/CMDI/CmdiTemplate/class.CmdiTemplateBuilder.inc';
    include_once drupal_get_path('module', 'flat_deposit') . '/Helpers/CMDI/CmdiTemplate/class.CmdiTemplateRebuilder.inc';
    include_once drupal_get_path('module', 'flat_deposit') . '/Helpers/CMDI/CmdiTemplate/class.CmdiTemplateSaveLoaded.inc';
    include_once drupal_get_path('module', 'flat_deposit') . '/Helpers/CMDI/CmdiTemplate/class.CmdiTemplateLoader.inc';
    include_once drupal_get_path('module', 'flat_deposit') . '/Helpers/CMDI/CmdiTemplate/class.CmdiTemplateSaver.inc';
    include_once drupal_get_path('module', 'flat_deposit') . '/Helpers/CMDI/CmdiTemplate/class.CmdiTemplateDb.inc';
    include_once drupal_get_path('module', 'flat_deposit') . '/Helpers/CMDI/CmdiTemplate/class.CmdiTemplateData.inc';
    include_once drupal_get_path('module', 'flat_deposit') . '/Helpers/CMDI/CmdiTemplate/class.CmdiTemplateManager.inc';
    include_once drupal_get_path('module', 'flat_deposit') . '/Helpers/CMDI/CmdiTemplate/class.CmdiValueExtractor.inc';
    include_once drupal_get_path('module', 'flat_deposit') . '/Helpers/CMDI/CmdiTemplate/class.CmdiValueSyncer.inc';

}

/**
 * Translates an xml form template into a drupal form
 */
class FormBuilder
{
    // Path containing xml form templates files for drupal form generation
    const FORM_TEMPLATES_PATH = __DIR__  . '/../FormTemplates/';


    /**
     * @var array A drupal form render array to be exported.
     */
    private $form = array();

    /**
     * @var array Nested array containing all template components as render arrays
     */
    private $formComponents = array();

    /**
     * @var array Nested array containing a render array with #type data
     */
    private $data = array();

    /**
     * @var either drupal MPI_Bundle node or IslandoraObject
     */

    private $node = null;

    /**
     * @var this contains selected cmdi profile template name
     */
    private $templateName = null;

    /**
     * Scans the cmdi2Drupal form path for profiles and returns profile names of a certain content type
     *
     * @param $content_type String drupal content type of the profile (e.g. flat_collection or flat_bundle)
     *
     * @return array associative array with file names
     */
    public static function getAvailableTemplates($content_type)
    {
        $templates = [];
        foreach (glob(self::FORM_TEMPLATES_PATH . "*.xml") as $filename) {
            $xml = simplexml_load_file($filename);
            $fileContentType = (string)$xml->header->content_type;

            if ($fileContentType === $content_type) {
                $template_names [] = (string)$xml->header->template_name;
                $template_descriptions [] = (string)$xml->header->template_description;
            }
        }
        return array_combine($template_names, $template_descriptions);
    }

    public static function checkTemplateItems($templateName)
    {

        $filename = self::FORM_TEMPLATES_PATH . $templateName .".xml";

        $xml = simplexml_load_file($filename);
        $res = $xml->xpath('/profileToDrupal/items/item');
        if (empty($res)) {
            return "Chosen profile has no complete form template";
        }

        return TRUE;
    }

    /**
     * Template2FormParser constructor.
     */
    public function __construct($node)
    {
        $this->node = $node;
    }

    public static function getDataForTemplate($templateName)
    {
        $fName = self::FORM_TEMPLATES_PATH . $templateName . '.xml';
        $template = simplexml_load_file($fName);

        if (!$template) {
            return false;
        }

        $data = [];

        // loop through array to get core attributes of each element
        foreach ($template->data->children() as $key => $value) {
            $data[$key] = (string)$value;
        }

        return $data;
    }


    /**
     * Builds the form based on a template file.
     *
     * @param $templateName String Name of the template used to create the drupal form.
     *
     * @param $cmdi SimpleXMLElement An existing cmdi file with data to fill form fields with "default" values.
     *
     * @param $pressedButtons array associative array with aggregated button presses
     *
     * @param $inheritAll boolean Set to true will overwrite template attribute "inherited" and populate all form fields with data from data source
     *
     * @return boolean Success of operation.
     *
     */
    public function setForm($templateName, $cmdi = null, $pressedButtons = null, $inheritAll = false)
    {

        $fName = self::FORM_TEMPLATES_PATH . $templateName . '.xml';
        $template = simplexml_load_file($fName);

        if (!$template) {
            return false;
        }

        $dataNode = $this->getHeadNode($cmdi);

        if ($this->node) {
            if ((get_class($this->node)) !== 'IslandoraFedoraObject') {
                $this->clearInheritedTitle($dataNode);
            }
        }

        // adding this so profile can be referenced in CmdiTemplateManager
        // for the CmdiTemplate functionality
        $this->templateName = $templateName;

        $setComponets = $this->setFormComponents($template, $dataNode, $pressedButtons, $index = 0, $inheritAll);
        if (!$setComponets) {
            return false;
        }
        $this->setData($template);

        foreach ($this->formComponents as $key => $component) {
            $this->form[$key]= $component;
        }

        $this->form['data'] = $this->data;

        if ($this->node) {
            if ((get_class($this->node)) == 'IslandoraFedoraObject') {
                $this->form['title_field'][0]['#default_value'] = $this->node->label;
            } else {
                $this->form['title_field'][0]['#default_value'] = $this->node->title;
            }
        }

        return true;
    }

    /**
     * Puts form component and data together and exports
     *
     */
    public function getForm()
    {

        return $this->form;

    }


    private function clearInheritedTitle(&$dataNode)
    {
        if ($this->node == null) {
            return;
        }
        if (!empty($dataNode)) {
            if (!empty($dataNode->children('cmd', TRUE)->Name)) {
                $title = $dataNode->children('cmd', TRUE)->Name;
                }
            else {
                $title = $dataNode->children('cmd', TRUE)->Title;
            }

            if (isset($title[0])) {
                    $title[0] = '';
            }
        }
    }


    /**
     * Creates nested form render arrays from form template.
     *
     *  Runs along xml tree of a xml template file to generate visible form fields.
     *
     * @param $template SimpleXMLElement A Drupal form template describing the layout of the form.
     *
     * @param $dataNode SimpleXMLElement Node of an existing Cmdi file with data to fill form fields with "default" values.
     *
     * @param $pressedButtons array Counts of button presses per component.
     *
     * @param int $index Indicates in which branch of the pressedButton array will be searched for counts.
     *
     * @param $inheritAll boolean Set to true will overwrite template attribute "inherited" and populate all form fields with data from data source
     *
     */
    private function setFormComponents($template, $dataNode = null, $pressedButtons = null, $index = 0, $inheritAll = false)
    {

        if (!isset($template->items)) {
            return false;
        }

        foreach ($template->items->item as $component) {
            $className = "Form" . (string)$component['type'];

            $numberElements = isset($component['minOccurrence']) ? (int)$component['minOccurrence'] : 0;
            $formComponent = new $className($numberElements);

            $nodeName = $formComponent->getFormKeyName($component);
            $formComponent->setCmdiProfile($this->templateName);
            $formComponent->setCmdiTree([$nodeName]);
            $formComponent->setFormComponent($component, $dataNode, $pressedButtons, $index, $inheritAll);
            $this->formComponents[$nodeName] = $formComponent->getFormComponent();
        }

        return true;
    }



    /**
     * Determines the position in cmdi to look for existing data
     * @param $cmdi CmdiHandler
     * @return head node of the cmdi components section
     */
    private function getHeadNode($cmdi)
    {

        if ($cmdi) {
            $name = $cmdi->getNameById();
            return $cmdi->children('cmd', true)->Components->{$name};
        }

        return null;
    }

    /**
     * Reads in 'data'-child of template to create an invisible form element in drupal with necessary data to process the form
     *
     * @param $template SimpleXMLElement Template file with 'data' child
     */
    private function setData($template)
    {
        $data = [];

        // loop through array to get core attributes of each element
        foreach ($template->data->children() as $key => $value) {
            $data[$key] = (string)$value;
        }

        $renderArray = array(
            '#type' => 'value',
            '#value' => $data,
        );

        $this->data = $renderArray;
    }

    /**
     * Transforms form_state 'clicked_button' value in aggregated data (i.e. associative array with '#name' property
     * as ID and #value-property as switch which action (i.e. add or substract)) to perform. Ass array is saved in form_state
     *
     * @param $form_state
     *
     */
    public function aggregatePressedButtons(&$form_state)
    {
        // return if no button has been pressed
        if (!isset($form_state['clicked_button'])) {
            return false;
        }

        // return if button has ignore flag set to true
        if (isset($form_state['clicked_button']['#flat_ignore_button']) && $form_state['clicked_button']['#flat_ignore_button'] === true) {
            return false;
        }

        $componentId = $form_state['clicked_button']['#componentid'];
        $isAddButton = is_int(strpos($form_state['clicked_button']['#value'], 'Add '));

        #if (!isset($form_state['pressedButtons'])){$form_state['pressedButtons'] = array();};

        $pointer = &$form_state['pressedButtons'];
        $parents = $form_state['clicked_button']['#parents'];

        foreach ($parents as $key => $parent) {
            if ($parent === $componentId) {
                $pointer[$componentId]['base'] = isset($pointer[$componentId]['base']) ? $pointer[$componentId]['base'] :  0;
                $pointer = &$pointer[$componentId]['base'];
                break;
            }

            if (!isset($pointer[$parent])) {
                $pointer[$parent] = array();
            }

            $pointer = &$pointer[$parent];
        }

        // Get saved value or set to 0;

        if ($isAddButton) {
            $pointer = $pointer + 1;
        } else {
            $pointer = $pointer - 1;
        }
    }
}


/**
 * Ajax callback for the selected option fields.
 */
function select_profile_name_ajax_callback($form, &$form_state)
{
    return $form['template_container'];
}

/**
 * Ajax callback for click an add button
 */
function add_multival_to_cmdi_form_ajax($form, $form_state)
{
    return $form['template_container'];
}

/**
 * Ajax callback for click an remove button
 */
function remove_multival_from_cmdi_form_ajax($form, $form_state)
{
    return $form['template_container'];
}
