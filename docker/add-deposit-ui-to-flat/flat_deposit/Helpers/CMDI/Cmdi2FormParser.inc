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

            return $this->default_values;
        }

        return false;

    }


    /**
     * Loop through Form Template to get all fields with thw inherited attribute. If is set to true, get values from parent cmdi and save as default_value.
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
                    // loop through parent cmdi if values are set
                    if (isset($this->parentData->Components->{$this->parentProfile}->$sub->{$name})){

                        $count = 0;
                        foreach ($this->parentData->Components->{$this->parentProfile}->$sub->{$name} as $child) {

                            $value = (string)$this->parentData->Components->{$this->parentProfile}->$sub->{$name}[$count];
                            if ($type == 'date') {

                                $value = $this->convertDate($value);

                            }

                            $default_values[$sub][$id][(string)$count]['#default_value'] = $value;

                        $count++;
                        }
                    }

                } else {

                    // loop through parent cmdi if values are set
                    if (isset($this->parentData->Components->{$this->parentProfile}->{$name})) {

                        $count = 0;

                        foreach ($this->parentData->Components->{$this->parentProfile}->{$name} as $child) {
                            $value = (string)$this->parentData->Components->{$this->parentProfile}->{$name}[$count];
                            if ($type == 'date') {

                                $value = $this->convertDate($value);

                            }

                            $default_values[$id][$count] ['#default_value'] = $value;
                            $count++;

                        }

                    }

                }

            }

        };

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


    /**
     * Version of array_merge_recursive without overwriting numeric keys
     *
     * @param  array $array1 Initial array to merge.
     * @param  array ...     Variable list of arrays to recursively merge.
     *
     * @link   http://www.php.net/manual/en/function.array-merge-recursive.php#106985
     * @author Martyniuk Vasyl <martyniuk.vasyl@gmail.com>
     */
    function array_merge_recursive_new()
    {
        $arrays = func_get_args();
        $base = array_shift($arrays);
        foreach($arrays as $array) {
            reset($base);
            while(list($key, $value) = @each($array)) {
                if(is_array($value) && @is_array($base[$key])) {
                    $base[$key] = self::array_merge_recursive_new($base[$key], $value);
                }
                else {
                    $base[$key] = $value;
                }
            }
        }
        return $base;
    }



}