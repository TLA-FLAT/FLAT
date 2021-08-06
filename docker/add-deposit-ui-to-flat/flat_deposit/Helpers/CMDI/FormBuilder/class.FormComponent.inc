<?php
spl_autoload_register(function ($class_name) {
    include_once drupal_get_path('module', 'flat_deposit') . '/Helpers/CMDI/Formbuilder/class.' . $class_name . '.inc';

});

/**
 *
 */
abstract class FormComponent
{
    /**
     * @var array containing one ore more instances of a drupal form fields
     */
    protected $formComponent = array();

    /**
     * @var $numberElements int number of drupal form elements in this component
     */
    private $numberElements = 1;
    private $minNumberElements = 1;

    /**
     * @var string cmdi profile name
     */
    protected $cmdiProfile = null;

    /**
     * @var array cmdi node tree
     */
    protected $cmdiTree = [];


    /**
     * FormComponent constructor.
     *
     * @param $numberElements
     */
    public function __construct($numberElements)
    {

        $this->numberElements = $numberElements;
        $this->minNumberElements = $numberElements;
    }

    public function setCmdiTree($tree)
    {
        $this->cmdiTree = $tree;
    }

    public function setCmdiProfile($profile)
    {
        $this->cmdiProfile = $profile;
    }

    abstract public function nestComponentElements($templateNode, $componentElements);

    abstract public function extractValuesFromNode($node, $componentName);

    abstract public function getComponentButtonPresses($templateNode, $pressedButtons);
    /**
     * Get form key name
     *
     * @return String key name used to put component in form array
     */
    abstract public function getFormKeyName($templateNode);

    /**
     * @param SimpleXMLElement $templateNode Node of template
     *
     * @param SimpleXMLElement $cmdiNode existing cmdi node
     *
     * @param array $pressedButtons array with number of pressed buttons
     *
     * @param String $buttonId
     *
     * @param $inheritAll boolean Set to true will overwrite template attribute "inherited" and populate all form fields with data from data source
     **/
    public function setFormComponent($templateNode, $existingCmdiNode = null, $pressedButtons = null, $buttonId = null, $inheritAll = false)
    {
        $componentName = (string)$templateNode['name'];

        $defaultValues = $this->extractValuesFromNode($existingCmdiNode, $componentName);

        // count how many of these nodes are present in the existing CMDI.
        if ($existingCmdiNode) {
            $count = count($existingCmdiNode->{$componentName});
        } else {
            $count = 0;
        }

        // in case a node is present once in the existing CMDI but it has no value (empty XML element), we do not want to see it if it's optional.
        if (($count == 1) && $defaultValues && (strlen($defaultValues[0]) == 0)) {
            $count = 0;
        }

        // in case node is not present in the existing CMDI, check whether an attribute with the name is present
        $is_attribute = false;
            if ($existingCmdiNode && ($count == 0)) {
            $data = isset($existingCmdiNode->attributes(){$componentName}) ? $existingCmdiNode->attributes(){$componentName} : null;
            if ($data) {
                $count = 1; // there can be only one attribute with the same name
                $is_attribute = true;
            }
        }

        // in case of a template node with a skipParentInCMDI attribute, the default value is the value of the existing CMDI node itself
        if (isset($templateNode->attributes()['skipParentInCMDI'])) {
            $defaultValues[] = (string)$existingCmdiNode;
        }

        // in case of an optional composite component which has no children in the existing CMDI, we do not want to see it
        if (((string) $templateNode['type'] == 'Composite') && empty($existingCmdiNode->{$componentName}[0])) {
            $numberElements = 0;
        }

        $numberElements = max($count, $this->minNumberElements);

        $this->numberElements = $numberElements;

        $componentButtonPresses = $this->getComponentButtonPresses($templateNode, $pressedButtons);

        $this->adaptNumberElementsByButtonPresses($componentButtonPresses);
        $this->setButtonId($buttonId, (string)$templateNode['id']);

        $componentElements = array();

        if (isset($templateNode->container)) {
            $this->cmdiTree[] = (string)$templateNode['id'];
        }

        for ($index = 0; $index < $this->numberElements; $index++) {

            $defaultValue = isset($defaultValues[$index]) ? $defaultValues[$index] : null;
            if (!$defaultValue AND isset($templateNode->default)) {
                // see if there's a default value in the template and use that
                $defaultValue = (string)$templateNode->default;
            }

            if ($is_attribute) {
            $defaultValue = isset($existingCmdiNode->attributes(){$componentName}) ? (string)$existingCmdiNode->attributes(){$componentName} : null;
            }

            $tree = $this->cmdiTree;
            $tree[] = $index;

            $element = new FormField($templateNode, $defaultValue, $inheritAll);
            $element->setCmdiProfile($this->cmdiProfile);
            $element->setCmdiTree($tree);
            $element->setFormField();
            $element->addNestedComponents($existingCmdiNode, $componentButtonPresses, $index, $buttonId, $inheritAll);
            $componentElements[] = $element->getFormField();
        }

        $this->addButtonsToFormComponent($templateNode, $componentElements, $buttonId);

#        $this->addMenuToFormComponent($templateNode, $componentElements, $buttonId);

        $this->formComponent = $this->nestComponentElements($templateNode, $componentElements);
    }

    public function getNumberElements()
    {return $this->numberElements;}

    public function getMinNumberElements()
    {return $this->minNumberElements;}

    public function setNumberElements($numberElements)
    {
        $this->numberElements = $numberElements;
    }

    /**
     * Getter for form component.
     * @return mixed
     */
    public function getFormComponent()
    {
        return $this->formComponent;
    }

    private function setButtonId(&$buttonId, $nodeId)
    {

        if ($buttonId == null) {

            $buttonId = $nodeId;

        } else {

            $buttonId .= "_" . $nodeId;

        }

    }

    /**
     * Adds the number of pressed buttons to number of elements. This number needs to be bigger than 0.
     *
     * @param array $pressedButtons array with counted button presses.
     *
     * @param int $index determines the branch in which button presses will be searched for
     *
     */
    private function adaptNumberElementsByButtonPresses($pressedButtons, $index = 0)
    {
        if (isset($pressedButtons['base'])) {
            $this->setNumberElements($this->getNumberElements() + $pressedButtons['base']);
        }

        if ($this->getNumberElements() < $this->getMinNumberElements()) {
            $this->setNumberElements($this->getMinNumberElements());
        }

    }

    /**
     * Adds add and remove buttons to component.
     *
     * @param $templateNode
     *
     * @param $componentElements
     */
    private function addButtonsToFormComponent($templateNode, &$componentElements, $buttonId)
    {
        $showButtons = false;

        if (($templateNode->type == 'hidden') OR ($templateNode->type == 'container')) {
            // do not add buttons to hidden form elements or containers
            return;
        }

        if (isset($templateNode['multival'])) {
            $showButtons = true;
            $nMax = 999;
        } elseif (isset($templateNode['minOccurrence']) and $templateNode['minOccurrence'] == 0) {
            $showButtons = true;
            $nMax = 1;
        }

        if ($showButtons) {

            $componentId = (string) $templateNode['id'];
            $title = (string) $templateNode->title;

            // Add 'add' button
            $isAccessible = $this->numberElements < $nMax ? true : false;

            $componentElements['add'] = array(
                '#weight' => 999,
                '#name' => $buttonId,
                '#componentid' => $componentId,
                '#type' => 'button',
                '#value' => t('Add ' . $title),
                '#access' => $isAccessible,
                '#ajax' => array(
                    'callback' => 'add_multival_to_cmdi_form_ajax',
                    'wrapper' => 'template-form',
                    'method' => 'replace',
                    'event' => 'click',
                    'prevent' => 'submit click mousedown',
                ),
                '#limit_validation_errors' => array(), // No elements will be validated
            );

            // Add 'Remove' button
            $isAccessible = $this->numberElements > $this->minNumberElements ? true : false;
            $componentElements['remove'] = array(
                '#weight' => 999,
                '#name' => $buttonId,
                '#componentid' => $componentId,
                '#type' => 'button',
                '#value' => t('Remove ' . $title),
                '#access' => $isAccessible,
                '#ajax' => array(
                    'callback' => 'remove_multival_from_cmdi_form_ajax',
                    'wrapper' => 'template-form',
                    'method' => 'replace',
                    'effect' => 'fade',
                ),
                '#limit_validation_errors' => array(), // No elements will be validated

            );
        }

    }

}
