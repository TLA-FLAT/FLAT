<?php

module_load_include('php','flat_deposit','Helpers/CMDI/CmdiHandler');

class Cmdi2FormParserException extends Exception{}

/**
 * Class for getting drupal form values from existing cmdi file or data stream
 */
class Cmdi2FormParser
{
    /**
     * @var string Name of the cmdi profile used to generate form
     */
    private $bundleProfile;

    /**
     * @var SimpleXMLElement xml object containing the form template
     */
    private $template;

    /**
     * @var SimpleXMLElement xml object containing cmdi data of a parent
     */
    private $parentData;


    /**
     * @var string Name of the cmdi profile of the parent
     */
    private $parentProfile;




    /**
     * @var array associative array of default values
     */
    private $default_values;


    /**
     * Gets default values from a provided cmdi file
     *
     * @return mixed array containing a nested #default_value properties
     */
    public function getDefaultValuesFromCmdi($profile, $parentCmdi)
    {

        $this->bundleProfile = $profile;

        $fName = drupal_get_path('module', 'flat_deposit') . CmdiHandler::FORM_TEMPLATES_PATH . $this->bundleProfile . '.xml';

        if (!file_exists($fName)){
            throw new Cmdi2FormParserException("Unable to find form template for profile $profile");
        }

        $this->template = CmdiHandler::loadXml($fName);

        $this->parentData = $parentCmdi;
        $this->parentProfile = CmdiHandler::getNameById($this->parentData->Header->MdProfile);


        if ($this->parentData) {

            $this->default_values = $this->_getInheritedValues();

            #$arraydata = json_decode(json_encode((array)$data2), true); // data as ass array
            // need to transform data keys into drupal form element ids as specified in Form Templates



            return $this->default_values;
        }

        return false;

    }


    /**
     * Reads in template to create form fields.
     *
     */
    private function _getInheritedValues(){

        $default_values = [];

        // loop through array to get core attributes of each element
        foreach ($this->template->fields->field as $field) {

            if((bool)$field->inherited) {

                $id = (string)$field['id'];
                $name = (string)$field['name'];
                $type = (string)$field->type;

                if (isset($field->subNode)) {

                    $sub = (string)$field->subNode[0];

                    $value = (string)$this->parentData->Components->{$this->parentProfile}->$sub->{$name};

                    $default_values[$sub][$id]['#default_value'] = '';

                    $df = &$default_values[$sub][$id]['#default_value'];

                } else {

                    $value = (string)$this->parentData->Components->{$this->parentProfile}->{$name};

                    $default_values[$id] ['#default_value'] = '';

                    $df = &$default_values[$id]['#default_value'];

                }

                if ($type == 'date') {

                    $value = $this->convertDate($value);

                }

                $df = $value;


            };


        }
        return $default_values;
    }

    function convertDate($date){

        $converted_date = array();
        $keys = ['year','month', 'day'];
        $index = 0;
        $tmp = explode('-', $date);

        foreach ($tmp as $el){


            $el = (int)$el; // get rid of 0 in front of months and days
            $converted_date[$keys[$index]] = (string)$el;
            $index++;
        }

        return $converted_date;
    }



}