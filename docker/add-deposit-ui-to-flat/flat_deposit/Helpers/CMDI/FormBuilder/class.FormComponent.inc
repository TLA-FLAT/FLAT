<?php
spl_autoload_register(function ($class_name) {
    include drupal_get_path('module', 'flat_deposit') . '/Helpers/CMDI/Formbuilder/class.' . $class_name . '.inc';

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
     * FormComponent constructor.
     *
     * @param $numberElements
     */
    public function __construct($numberElements){

        $this->numberElements = $numberElements;
        $this->minNumberElements = $numberElements;
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
     **/
    public function setFormComponent($templateNode, $existingCmdiNode = null, $pressedButtons = null, $buttonId = null)
    {

        $componentName = (string)$templateNode['name'];

        $defaultValues = $this->extractValuesFromNode($existingCmdiNode, $componentName);

        $componentButtonPresses = $this->getComponentButtonPresses($templateNode, $pressedButtons);

        $this->adaptNumberElementsByButtonPresses($componentButtonPresses);

        $this->setButtonId($buttonId, (string)$templateNode['id']);

        $componentElements = array();

        for ($index = 0; $index < $this->numberElements; $index++){

            $defaultValue = isset($defaultValues[$index]) ? $defaultValues[$index] : null;
            $element = new FormField($templateNode, $defaultValue);
            $element->setFormField();
            $element->addNestedComponents($existingCmdiNode, $componentButtonPresses, $index, $buttonId);
            $componentElements[]  = $element->getFormField();

        }

        $this->addButtonsToFormComponent($templateNode, $componentElements, $buttonId);

#        $this->addMenuToFormComponent($templateNode, $componentElements, $buttonId);

        $this->formComponent = $this->nestComponentElements($templateNode, $componentElements);

    }

    public function getNumberElements(){return $this->numberElements;}

    public function getMinNumberElements(){return $this->minNumberElements;}

    public function setNumberElements($numberElements){
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


    private function setButtonId(&$buttonId, $nodeId){

        if ($buttonId == null){

            $buttonId = $nodeId;

        } else{

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
    private function adaptNumberElementsByButtonPresses($pressedButtons, $index = 0){


        if (isset($pressedButtons['base'])){

            $this->setNumberElements($this->getNumberElements() + $pressedButtons['base']);
        }



        if ($this->getNumberElements() < $this->getMinNumberElements()){

            $this->setNumberElements($this->getMinNumberElements());
        };


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

        if (isset($templateNode['multival']) OR (isset($templateNode['minOccurrence']) AND $templateNode['minOccurrence'] == 0)) {

            $componentId = (string)$templateNode['id'];
            $title = (string)$templateNode->title;
            // Add 'add' button
            $componentElements['add'] = array(
                '#weight' => 999,
                '#name' => $buttonId,
                '#componentid' => $componentId,
                '#type' => 'button',
                '#value' => t('Add ' . $title),
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
            $isAccessible = $this->numberElements > $this->minNumberElements ? TRUE : FALSE;
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

    private function addMenuToFormComponent($templateNode, &$componentElements, $buttonId)
    {


        $componentId = (string)$templateNode['id'];
        $menuId = 'menu_' . $buttonId;

        if (isset($templateNode->item)) {

            $options = $this->getOptions($templateNode);

            if (!empty ($options)) {

                $componentElements['menu'] = array(
                    '#weight' => -999,
                    '#name' => $menuId,
                    '#componentId' => $componentId . '_checkboxes',
                    '#type' => 'checkboxes',
                    '#options' => $options,
                    '#title' => t('Which fields should be shown?'),
                    '#ajax' => array(
                        'callback' => 'add_multival_to_cmdi_form_ajax',
                        'wrapper' => 'template-form',
                        'method' => 'replace',
                        'event' => 'click',
                        'prevent' => 'submit click mousedown',
                    ),
                    '#limit_validation_errors' => array(), // No elements will be validated
                );


            }

        }

    }




    private function getOptions($item, &$options = array())
    {

        foreach ($item->item as $item) {

            $isLeaf = ((string)$item['type'] == 'Leaf');
            $isOptional = (!isset($item['minOccurrence']) OR (isset($item['minOccurrence']) AND $item['minOccurrence'] == '0'));

            if ($isOptional) {

                    $id = (string)$item->attributes()->id;
                    $name = (string)$item->attributes()->name;
                    $options[$id] = $name;

                }
            }

        return $options;

    }

}

