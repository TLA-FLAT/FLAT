<?php


class FormField
{
    /**
     * @var array form field render array
     */
    private $formField = array();

    private $formTemplateNode = array();

    private $defaultValue;

    private $inheritAll = false;


    /**
     * FormField constructor.
     *
     * @param $formTemplateNode SimpleXMLElement Form Template node containing render-specific info
     *
     * @param $defaultValue Array of default values
     *
     * @param $inheritAll boolean Set to true will overwrite template attribute "inherited" and populate all form fields with data from data source
     */
    public function __construct($formTemplateNode, $defaultValue = null, $inheritAll = false)
    {

        $this->formTemplateNode = $formTemplateNode;

        $this->defaultValue = $defaultValue;

        $this->inheritAll = $inheritAll;
    }



    /**
     * @return array
     */
    public function getFormField(): array
    {

        return $this->formField;
    }

    /**
     * @param array $data default value of the field
     */
    public function setFormField()
    {

        $formField = [];

        $this->setFieldTitle($formField);
        $this->setFieldTree($formField);
        $this->setFieldType($formField);
        $this->setFieldDefaultValue($formField);
        $this->setFieldRequired($formField);
        $this->setFieldDescription($formField);
        $this->formField = $formField;
    }




    ###################
    //set title of item
    ###################
    /**
     * Set title. If unset use field machine name
     * @param $id
     * @param $field
     */
    private function setFieldTitle(&$formField)
    {

        if (isset($this->formTemplateNode->label)) {
            $title = (string)$this->formTemplateNode->label;
        } else {
            $title = (string)$this->formTemplateNode->title;
        }

        $formField['#title'] = $title;
    }

    private function setFieldDefaultValue(&$formField)
    {

        if (($this->formTemplateNode['inherited'] OR $this->inheritAll) AND $this->defaultValue) {

            $formField['#default_value'] = $this->convertValue($this->defaultValue);
        }
    }



    /**
     * Set tree attribute for formfield formFields
     *
     * @param $newOutput array form render array
     *
     * @param $item SimpleXMLField xml formField with info regarding field
     *
     * @param $id string ID of the field formField
     */
    private function setFieldTree(&$formField)
    {


        if ($this->formTemplateNode->type == 'fieldset') {

            $formField['#tree'] = true;

        }
    }


    /**
     * Set form formField type
     */

    private function setFieldType(&$formField)
    {
        if (!isset($this->formTemplateNode->type)) {
            $this->formTemplateNode->type = 'textfield';
        }

        $type = (string)$this->formTemplateNode->type;


        switch ($type) {

            case 'dropdown':

                $type = 'select';
                $options = [];


                foreach ($this->formTemplateNode->options->option as $option) {
                    $options [] = (string)$option;
                }

                $formField['#options'] = drupal_map_assoc($options);
                break;

            case 'date':

                $type = 'date';

                break;
            case 'fieldset':

                $type = 'fieldset';

                break;

            default :
                $type = 'textfield';

        }
        $formField['#type'] = $type;


    }

    /**
     * Set required property of form formFields
     * @param $id
     * @param $field
     */
    private function setFieldRequired(&$formField)
    {
        $formField['#required'] = false;

        if (isset($this->formTemplateNode['minOccurrence']) AND (string)$this->formTemplateNode['minOccurrence'] == '1' AND (string)$this->formTemplateNode['type'] == 'Leaf') {

            $formField['#required'] = true;
        }

    }


    /**
     * Set description property of form formFields
     * @param $id
     * @param $field
     */

    private function setFieldDescription(&$formField)
    {
        if (isset($this->formTemplateNode->description)) $formField['#description'] = (string)$this->formTemplateNode->description;
    }



    private function convertValue($value){

        $type = (string)$this->formTemplateNode->type;


        switch ($type) {
            case 'date':
                $converted_date = array();
                $keys = ['year','month', 'day'];
                $index = 0;
                $tmp = explode('-', $value);

                foreach ($tmp as $el){


                    $el = (int)$el; // get rid of 0 in front of months and days
                    $converted_date[$keys[$index]] = (string)$el;
                    $index++;
                }
                $convertedValue = $converted_date;
                break;
            default:
                $convertedValue = $value;
                break;
        }



        return $convertedValue;

        }


    /**
     * Loops through nested items to generate nested form components.
     *
     * @param $CmdiHandler $existingCmdiNode
     * @param array $pressedButtons
     * @param int $index
     * @param String $buttonId
     *
     */
    public function addNestedComponents($existingCmdiNode, $pressedButtons, $index = 0, $buttonId, $inheritAll = false){


        if(isset($this->formTemplateNode->item)) {


            foreach ($this->formTemplateNode->item as $nestedNode) {

                $className = "Form" . (string)$nestedNode['type'];

                $numberElements = isset($nestedNode['minOccurrence']) ? (int)$nestedNode['minOccurrence'] : 0;

                $nestedFormComponent = new $className($numberElements);

                $key = $this->getFormField()['#title'];
                $nestedDataNode = isset($existingCmdiNode->{$key}[$index]) ? $existingCmdiNode->{$key}[$index] : null;

                $nestedButtonId = $buttonId . "_" . (int)$index ;

                $nestedPressedButton = isset($pressedButtons[$index]) ? $pressedButtons[$index] : null;

                $nestedFormComponent->setFormComponent($nestedNode, $nestedDataNode, $nestedPressedButton, $nestedButtonId, $inheritAll);

                $nestedNodeName = $nestedFormComponent->getFormKeyName($nestedNode);
                $this->formField[$nestedNodeName] = $nestedFormComponent->getFormComponent();

            }

        }
    }

}