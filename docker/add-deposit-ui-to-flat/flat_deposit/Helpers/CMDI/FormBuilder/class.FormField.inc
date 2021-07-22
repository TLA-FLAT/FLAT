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
     * @var string selected cmdi profile
     */
    private $cmdiProfile = null;

    /**
     * @var array cmdi node tree path
     */
    private $cmdiTree = [];


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

    public function setCmdiProfile($profile)
    {
        $this->cmdiProfile = $profile;
    }

    public function setCmdiTree($tree)
    {
        $this->cmdiTree = $tree;
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
        $this->setFieldDataAttributes($formField);
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

        if (($this->formTemplateNode['inherited'] or $this->inheritAll) and $this->defaultValue) {

            $formField['#default_value'] = $this->convertValue($this->defaultValue);
        }
        else {
            // see if there's a default value in the template and use that
            if (isset($this->formTemplateNode->default)) {
                $formField['#default_value'] = (string)$this->formTemplateNode->default;
            }
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

        if (($this->formTemplateNode->type == 'fieldset') OR ($this->formTemplateNode->type == 'container')) {

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
                    $options[] = (string)$option;
                }

                $formField['#options'] = drupal_map_assoc($options);
                break;

            case 'date':

                $type = 'date';

                break;

            case 'fieldset':

                $type = 'fieldset';

                $component_id = (string)$this->formTemplateNode['id'];

                // $formField['#flat_cmdi_metadata'] = ['component_id' => $component_id, 'profile' => $this->cmdiProfile, 'tree' => $this->cmdiTree];
                $formField['flat_cmdi_templates'] = CmdiTemplateManager::build($component_id, $this->cmdiProfile, $this->cmdiTree);
                $formField['#panel_actions'] = CmdiTemplateManager::buildActions($this->cmdiProfile, $component_id, $this->cmdiTree);

                break;

            case 'container':

                 $type = 'container';

                break;

            case 'textarea':

                $type = 'textarea';

                break;

            case 'hidden':

                $type = 'hidden';

                break;

            default:
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

        if (isset($this->formTemplateNode['minOccurrence']) and (string)$this->formTemplateNode['minOccurrence'] == '1' and (string)$this->formTemplateNode['type'] == 'Leaf') {

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
        if (isset($this->formTemplateNode->description)) {
            $formField['#description'] = (string)$this->formTemplateNode->description;
        }

    }

    /**
     * Set data attributes of form formFields
     * @param $id
     * @param $field
     */
    private function setFieldDataAttributes(&$formField)
    {
        if (isset($this->formTemplateNode->data)) {

            foreach ($this->formTemplateNode->data->children() as $attribute => $value) {
                $formField['#attributes']['data-' . $attribute] = (string)$value;
            }
        }

    }

    private function convertValue($value)
    {

        $type = (string)$this->formTemplateNode->type;

        switch ($type) {
            case 'date':
                $converted_date = array();
                $keys = ['year', 'month', 'day'];
                $index = 0;
                $tmp = explode('-', $value);

                foreach ($tmp as $el) {

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
    public function addNestedComponents($existingCmdiNode, $pressedButtons, $index = 0, $buttonId, $inheritAll = false)
    {

        if (isset($this->formTemplateNode->item)) {

            foreach ($this->formTemplateNode->item as $nestedNode) {

                $className = "Form" . (string)$nestedNode['type'];

                $numberElements = isset($nestedNode['minOccurrence']) ? (int)$nestedNode['minOccurrence'] : 0;

                $key = $this->getFormField()['#title'];

                $nestedFormComponent = new $className($numberElements);

                $tree   = $this->cmdiTree;
                $tree[] = $nestedFormComponent->getFormKeyName($nestedNode);

                $nestedFormComponent->setCmdiProfile($this->cmdiProfile);
                $nestedFormComponent->setCmdiTree($tree);

                $nestedDataNode = isset($existingCmdiNode->{$key}[$index]) ? $existingCmdiNode->{$key}[$index] : null;

                $nestedButtonId = $buttonId . "_" . (int)$index;

                $nestedPressedButton = isset($pressedButtons[$index]) ? $pressedButtons[$index] : null;

                $nestedFormComponent->setFormComponent($nestedNode, $nestedDataNode, $nestedPressedButton, $nestedButtonId, $inheritAll);

                $nestedNodeName = $nestedFormComponent->getFormKeyName($nestedNode);
                $this->formField[$nestedNodeName] = $nestedFormComponent->getFormComponent();

                // attributes of the nested items

                foreach ($nestedNode->attribute as $attribute) {

                    $className = "Form" . (string)$attribute['type'];

                    $numberElements = isset($attribute['minOccurrence']) ? (int)$attribute['minOccurrence'] : 0;

                    $attributeFormComponent = new $className($numberElements);

                    $attributeKey = (string)$attribute->attributes()['name'];

                    $attributeDataNode = $existingCmdiNode->{$key}[$index];

                    $attributeButtonId = $buttonId . "_" . (int)$index;

                    $attributePressedButton = isset($pressedButtons[$index]) ? $pressedButtons[$index] : null;

                    $attributeFormComponent->setFormComponent($attribute, $attributeDataNode, $attributePressedButton, $attributeButtonId, $inheritAll);

                    $attributeNodeName = $attributeFormComponent->getFormKeyName($attribute);

                    $this->formField[$attributeNodeName] = $attributeFormComponent->getFormComponent();

                }

            }

        }

        // attributes of the composite items

        if ($this->formTemplateNode['type'] == 'Composite' && isset($this->formTemplateNode->attribute)) {

            foreach ($this->formTemplateNode->attribute as $attribute) {

                $className = "Form" . (string)$attribute['type'];

                $numberElements = isset($attribute['minOccurrence']) ? (int)$attribute['minOccurrence'] : 0;

                $attributeFormComponent = new $className($numberElements);

                $attributeKey = (string)$attribute->attributes()['name'];

                $attributeDataNode = $existingCmdiNode->{$key}[$index];

                $attributeButtonId = $buttonId . "_" . (int)$index;

                $attributePressedButton = isset($pressedButtons[$index]) ? $pressedButtons[$index] : null;

                $attributeFormComponent->setFormComponent($attribute, $attributeDataNode, $attributePressedButton, $attributeButtonId, $inheritAll);

                $attributeNodeName = $attributeFormComponent->getFormKeyName($attribute);

                $this->formField[$attributeNodeName] = $attributeFormComponent->getFormComponent();

            }

        }

        // $this->formField['#cmdi_template_fields'] = array_filter(array_keys($this->formField), function($field) {
        //     return $field !== 'flat_cmdi_templates' || $field !== '#panel_actions';
        // });
    }

}
