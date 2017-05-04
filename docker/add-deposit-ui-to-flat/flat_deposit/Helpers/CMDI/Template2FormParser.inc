<?php

class Template2FormParserException extends Exception {}

/**
 * Class translating xml template into drupal form
 */
class Template2FormParser {

    /**
     * @var SimpleXMLElement the template used to create the drupal form
     */
    private $xml;

    /**
     * @var array form elements (render arrays) and their basic attributes
     */
    private $fields;

    /**
     * @var array additional parameters for render elements e.g. for defining fields in which elements will be placed
     */
    private $additional_field_info;

    /**
     * @var array drupal form render array to be exported
     */
    private $form;


    /**
     * Form builder function
     *
     * @param $xml SimpleXMLElement
     *
     * @return array drupal render array
     */
    public function buildDrupalForm($xml){


        $this->xml = $xml;
        $this->_getFormFields();
        $this->_getDataField();
        $this->_structureForm();
        return $this->form;
    }


    /**
     * Reads in template to create form fields.
     *
     */
    private function _getFormFields(){

        $xml = $this->xml;
        // loop through array to get core attributes of each element
        foreach ($xml->fields->field as $field) {

            $id = (string)$field['id'];
            $this->fields[$id] = array();
            $this->_setTitle($id, $field);
            $this->_setType($id, $field);
            $this->_setRequired($id, $field);
            $this->_setDescription($id, $field);
            $this->_setDefaultValue($id, $field);
            $this->_setFieldset($id, $field);
        }
    }


    /**
     * This function places form elements in form fields
     */
    private function _structureForm()
    {

        foreach ($this->fields as $key => $field) {

            // set form fieldsets
            if (isset($this->additional_field_info[$key]['#subNode'])) {

                $subNode = $this->additional_field_info[$key]['#subNode'][0];

                // create fieldset if not existing
                if (!isset($this->form[$subNode])){
                    $this->form[$subNode] = array(
                        '#type' => 'fieldset',
                        '#title' => t($subNode),
                        '#collapsible' => TRUE,
                    );
                };

                $this->form[$subNode][$key] = $field;

            } else {

                $this->form[$key] = $field;
            }

        }
    }

    /**
     * Set title. If unset use field machine name
     * @param $id
     * @param $field
     */
    private function _setTitle($id, $field)
    {

        if (isset($field->title)) {
            $title = (string)$field->title;
        } else {
            $title = (string)$field['name'];
        }
        $this->fields[$id] ['#title'] = $title;

    }

    private function _setRequired($id, $field)
    {
        if (isset($field->required)) {
            $res = (string)$field->required == 'true' ? true : false;
            $this->fields[$id] ['#required'] = $res;
        } else {
            $this->fields[$id] ['#required'] = false;
        }
    }

    private function _setDescription($id , $field)
    {
        if (isset($field->description)) $this->fields[$id]['#description'] = (string)$field->description;
    }


    /**
     * return the render element. If not set, default is to return a text field.
     */

    private function _setType($id, $field)
    {
        if (!isset($field->type)) {
            $field->type = 'textfield';
        }

        $type = (string)$field->type;


        switch ($type){

            case 'dropdown':

                $data = 'select';
                $options = [];


                foreach ($field->options->option as $option){
                    $options [] = (string)$option;
                }

                $this->fields[$id]['#options'] = drupal_map_assoc($options);
                break;

            case 'date':

                $data = 'date';

                break;

            default :
                $data = 'textfield';

        }

        $this->fields[$id]['#type'] = $data;

    }


    private function _setDefaultValue($id, $field)
    {
        if (isset($field->defaultValue)) {
            $this->fields[$id]['#default_value'] = (string)$field->defaultValue;
        }

    }


    private function _setFieldset($id, $field)
    {
        if (isset($field->subNode)) {
            foreach ($field->subNode as $item )
            $this->additional_field_info[$id]['#subNode'][] = (string)$item;
        }

    }




    /**
     * Read in data field of template to create an invisible form field in drupal with necessary data to process the form
     */
    private function _getDataField(){

        $xml = $this->xml;

        $data = [];

        // loop through array to get core attributes of each element
        foreach ($xml->data->children() as $key => $value) {
            $data[$key] = (string)$value;
        }

        $field = array(
            '#type' => 'value',
            '#value' => $data,
        );
        $this->form['data'] = $field;
    }

}

