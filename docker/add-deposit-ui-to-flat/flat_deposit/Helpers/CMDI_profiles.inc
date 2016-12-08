<?php

/**
 * Class IngestServiceException is an exception class
 */
class CMDIProfileException extends Exception {}

/**
 * Created by PhpStorm.
 * User: danrhe
 * Date: 16/06/16
 * Time: 11:49
 */
class CMDI_profiles
{
    public $fields;

    function __construct($profile){
        $this->profile = $profile;
    }


    /**
     * Method that returns a drupal form array
     * @throws CMDITemplateExceptio
     */
    function getForm()
    {

        switch ($this->profile) {
            case 'session':
                $fields = array();

                $fields['field_1'] = array(
                    '#title' => t(strtoupper($this->profile) . ' Template'),
                    '#type' => 'fieldset',
                    '#collapsible' => TRUE,
                    '#collapsed' => FALSE,
                    '#tree' => TRUE);

                $fields['field_1']['Name'] = array(
                    '#title' => t('Name'),
                    '#type' => 'textfield',
                    );

                $fields['field_1']['Title'] = array(
                    '#title' => t('Title'),
                    '#type' => 'textfield',
                );

                $fields['field_1']['Date'] = array(
                    '#title' => t('Date'),
                    '#type' => 'date',
                );

                $fields['field_1']['descriptions'] = array(
                    '#title' => t('descriptions'),
                    '#type' => 'fieldset',
                    '#collapsible' => TRUE,
                    '#collapsed' => TRUE,
                    '#tree' => TRUE,
                );

                $fields['field_1']['descriptions'] ['Description'] = array(
                    '#title' => t('Description'),
                    '#type' => 'textfield',
                    );

                $fields['Location'] = array(
                    '#title' => t('Location'),
                    '#type' => 'fieldset',
                    '#collapsible' => TRUE,
                    '#collapsed' => FALSE,
                    '#tree' => TRUE,
                );

                $fields['Location']['Continent'] = array(
                    '#title' => t('Continent'),
                    '#type' => 'textfield',
                );

                $fields['Location']['Country'] = array(
                    '#title' => t('Country'),
                    '#type' => 'textfield',
                );

                $fields['Location']['Region'] = array(
                    '#title' => t('Region'),
                    '#type' => 'textfield',
                );

                $fields['Location']['Address'] = array(
                    '#title' => t('Address'),
                    '#type' => 'textfield',
                );

                $fields['Project'] = array(
                    '#title' => t('Project'),
                    '#type' => 'fieldset',
                    '#collapsible' => TRUE,
                    '#collapsed' => FALSE,
                    '#tree' => TRUE,
                );

                $fields['Project']['Name'] = array(
                    '#title' => t('Name'),
                    '#type' => 'textfield',
                );

                $fields['Project']['Title'] = array(
                    '#title' => t('Title'),
                    '#type' => 'textfield',
                );

                $fields['Project']['Id'] = array(
                    '#title' => t('Id'),
                    '#type' => 'textfield',
                );

                $fields['Project']['Contact'] = array(
                    '#title' => t('Contact'),
                    '#type' => 'fieldset',
                    '#collapsible' => TRUE,
                    '#collapsed' => FALSE,
                    '#tree' => TRUE,
                );

                $fields['Project']['Contact']['Name'] = array(
                    '#title' => t('Name'),
                    '#type' => 'textfield',
                );

                $fields['Project']['Contact']['Email'] = array(
                    '#title' => t('Email'),
                    '#type' => 'textfield',
                );

                $fields['Project']['Contact']['Organisation'] = array(
                    '#title' => t('Organisation'),
                    '#type' => 'textfield',
                );

                $fields['Project']['descriptions'] = array(
                    '#title' => t('descriptions'),
                    '#type' => 'fieldset',
                    '#collapsible' => TRUE,
                    '#collapsed' => FALSE,
                    '#tree' => TRUE,
                );

                $fields['Project']['descriptions'] ['Description'] = array(
                    '#title' => t('descriptions'),
                    '#type' => 'textfield',
                );

                $this->fields = $fields;
                break;

            case 'minimal':

                $fields['field_1'] = array(
                    '#title' => t(strtoupper($this->profile) . ' Template'),
                    '#type' => 'fieldset',
                    '#collapsible' => TRUE,
                    '#collapsed' => FALSE,
                    '#tree' => TRUE,
                );


                $fields['field_1']['Name'] = array(
                    '#title' => t('Name'),
                    '#type' => 'textfield',
                );

                $fields['field_1']['Title'] = array(
                    '#title' => t('Title'),
                    '#type' => 'textfield',
                );

                $fields['field_1']['Date'] = array(
                    '#title' => t('Date'),
                    '#type' => 'date',
                );

                $fields['Project'] = array(
                    '#title' => t('Project'),
                    '#type' => 'fieldset',
                    '#collapsible' => TRUE,
                    '#collapsed' => FALSE,
                    '#tree' => TRUE,
                );

                $fields['Project']['Name'] = array(
                    '#title' => t('Name'),
                    '#type' => 'textfield',
                );

                $fields['Project']['Title'] = array(
                    '#title' => t('Title'),
                    '#type' => 'textfield',
                );

                $fields['Project']['Id'] = array(
                    '#title' => t('Id'),
                    '#type' => 'textfield',
                );

                $fields['Project']['Contact'] = array(
                    '#title' => t('Contact'),
                    '#type' => 'fieldset',
                    '#collapsible' => TRUE,
                    '#collapsed' => FALSE,
                    '#tree' => TRUE,
                );

                $fields['Project']['Contact']['Name'] = array(
                    '#title' => t('Name'),
                    '#type' => 'textfield',
                );

                $fields['Project']['Contact']['Email'] = array(
                    '#title' => t('Email'),
                    '#type' => 'textfield',
                );

                $fields['Project']['Contact']['Organisation'] = array(
                    '#title' => t('Organisation'),
                    '#type' => 'textfield',
                );

                $fields['Project']['descriptions'] = array(
                    '#title' => t('descriptions'),
                    '#type' => 'fieldset',
                    '#collapsible' => TRUE,
                    '#collapsed' => FALSE,
                    '#tree' => TRUE,
                );

                $fields['Project']['descriptions'] ['Description'] = array(
                    '#title' => t('descriptions'),
                    '#type' => 'textfield',
                );

                $this->fields = $fields;
                break;



            case 'experiment':

                $fields['field_1'] = array(
                    '#title' => t(strtoupper($this->profile) . ' Template'),
                    '#type' => 'fieldset',
                    '#collapsible' => TRUE,
                    '#collapsed' => FALSE,
                    '#tree' => TRUE,
                );


                $fields['field_1']['Name'] = array(
                    '#title' => t('Name'),
                    '#type' => 'textfield',
                );

                $fields['field_1']['Title'] = array(
                    '#title' => t('Title'),
                    '#type' => 'textfield',
                );

                $fields['field_1']['Date'] = array(
                    '#title' => t('Date'),
                    '#type' => 'date',
                );

                $fields['Experiment'] = array(
                    '#title' => t('Experiment'),
                    '#type' => 'fieldset',
                    '#collapsible' => TRUE,
                    '#collapsed' => FALSE,
                    '#tree' => TRUE,
                );

                $fields['Experiment']['Title'] = array(
                    '#title' => t('Title'),
                    '#type' => 'textfield',
                );

                $fields['Experiment']['Notebook_Name'] = array(
                    '#title' => t('Notebook Name'),
                    '#type' => 'textfield',
                );

                $fields['Experiment']['Contact'] = array(
                    '#title' => t('Contact'),
                    '#type' => 'fieldset',
                    '#collapsible' => TRUE,
                    '#collapsed' => FALSE,
                    '#tree' => TRUE,
                );

                $fields['Experiment']['Contact']['User_Name'] = array(
                    '#title' => t('User Full Name'),
                    '#type' => 'textfield',
                );

                $fields['Experiment']['Contact']['Tel_number'] = array(
                    '#title' => t('Phone Number'),
                    '#type' => 'textfield',
                );

                $fields['Experiment']['Contact']['Email'] = array(
                    '#title' => t('Email'),
                    '#type' => 'textfield',
                );

                $fields['Experiment']['descriptions'] = array(
                    '#title' => t('descriptions'),
                    '#type' => 'fieldset',
                    '#collapsible' => TRUE,
                    '#collapsed' => FALSE,
                    '#tree' => TRUE,
                );

                $fields['Experiment']['descriptions'] ['Description'] = array(
                    '#title' => t('descriptions'),
                    '#type' => 'textfield',
                );

                $fields['Experiment']['conclusions'] = array(
                    '#title' => t('Experiment description'),
                    '#type' => 'fieldset',
                    '#collapsible' => TRUE,
                    '#collapsed' => FALSE,
                    '#tree' => TRUE,
                );

                $fields['Experiment']['conclusions'] ['Conclusion'] = array(
                    '#title' => t('Conclusion'),
                    '#type' => 'textfield',
                );

                $this->fields = $fields;
                break;

            default:
                $this->fields = FALSE;
                throw new CMDIProfileException ('Template fields have not been specified');
        }
    }
}




function get_example_md ($profile){
    switch ($profile) {

        case 'session':
            $md = array(
                'field_1' => array(
                    'Title' => array('#default_value' => 'The DvR_Sandbox'),
                    'Name' => array('#default_value' => 'DvR_Sandbox'),
                    'Date' => array('#default_value' => array(
                        'day' => 25,
                        'month' => 5,
                        'year' => 2015)),
                    'descriptions' => array(
                        'Description' => array('#default_value' => 'The is a test dataset and Daniels favorite dog is called James'),
                        ),
                ),
                'Location' => array(
                    'Continent' => array('#default_value' => 'Europe'),
                    'Country' => array('#default_value' => 'The Netherlands'),
                    'Region' => array('#default_value' => 'House of Language'),
                    'Address' => array('#default_value' => 'Wundtlaan 1'),
                ),

                'Project' => array(
                    'Title' => array('#default_value' => 'Deposit module'),
                    'Name' => array('#default_value' => 'FLAT archive'),
                    'Id' => array('#default_value' => ''),
                    'Contact' => array(
                        'Name' => array('#default_value' => 'Daniel'),
                        'Email' => array('#default_value' => 'Daniel@email.nl'),
                        'Organisation' => array('#default_value' => 'MPI Nijmegen'),
                    ),
                    'descriptions' =>                          array(
                        'Description' => array('#default_value' => 'FLAT Test set')


                    )
                ));
            break;

        case 'experiment':
            $md = array(
                'field_1' =>
                    array (
                        'Title' => array('#default_value' => 'Pilot EEG Study'),
                        'Name' => array('#default_value' =>'Subject_x01'),
                        'Date' => array('#default_value' => array(
                            'day' => 25,
                            'month' =>3,
                            'year' => 2016),
                            )
                    ),
                'Experiment' =>
                    array (
                        'Title' => array('#default_value' =>'EEG study on bilingual idiomatic expressions  '),
                        'Notebook_Name' => array('#default_value' =>''),
                        'Contact' =>
                            array (
                                'User_Name' => array('#default_value' => 'Daniel Tobias'),
                                'Tel_number' => array('#default_value' => '+3164482903'),
                                'Email' => array('#default_value' =>'Daniel@mpi.nl'),
                            ),
                        'descriptions' =>
                            array (
                                'Description' => array('#default_value' =>''),
                            ),
                        'conclusions' =>
                            array (
                                'Conclusion' => array('#default_value' =>''),
                            ),
                    ),
            );
            break;
        case 'minimal':
            $md = array(
                'field_1' =>
                    array (
                        'Title' => array('#default_value' =>'Title of Bundle'),
                        'Name' => array('#default_value' =>'Name of Bundle'),
                        'Date' => array('#default_value' => array(
                            'day' => 21,
                            'month' =>2,
                            'year' => 2013),)
                    ),
                'Project'=> array(
                    'Name' => array('#default_value' =>'Project name'),
                    'Title' => array('#default_value' =>'Project Title'),
                    'Id' => array('#default_value' =>'0187502u'),

                    'Contact' =>
                        array (
                            'Name' => array('#default_value' => 'Daniel Tobias'),
                            'Email' => array('#default_value' => 'Daniel@mpi.nl'),
                            'Organisation' => array('#default_value' => 'MPI')
                        ),
                    'descriptions' =>
                        array (
                            'Description' => array('#default_value' => 'bvaboav'), ),
                ),
            );
            break;

    }
    return $md;

}


function recursive_array_intersect_key(array $array1, array $array2) {

    $array1 = array_intersect_key($array1, $array2);

    foreach ($array1 as $key => &$value) {

        if (is_array($value) && is_array($array2[$key])) {

            $value = recursive_array_intersect_key($value, $array2[$key]);

        }
    }

    return $array1;
}


function recursiveFind(array $array, $needle) {

    $iterator = new RecursiveArrayIterator($array);
    $recursive = new RecursiveIteratorIterator( $iterator, RecursiveIteratorIterator::SELF_FIRST );

    $return = [];

    foreach ($recursive as $key => $value) {

        if ($key === $needle) { $return[] = $value;
        }
    }

    return $return;
}



function array_diff_recursive($arr1, $arr2)
{
    $outputDiff = [];

    foreach ($arr1 as $key => $value)
    {
        //if the key exists in the second array, recursively call this function
        //if it is an array, otherwise check if the value is in arr2
        if (array_key_exists($key, $arr2))
        {
            if (is_array($value))
            {
                $recursiveDiff = array_diff_recursive($value, $arr2[$key]);

                if (count($recursiveDiff))
                {
                    $outputDiff[$key] = $recursiveDiff;
                }
            }
            else if (!in_array($value, $arr2))
            {
                $outputDiff[$key] = $value;
            }
        }
        //if the key is not in the second array, check if the value is in
        //the second array (this is a quirk of how array_diff works)
        else if (!in_array($value, $arr2))
        {
            $outputDiff[$key] = $value;
        }
    }

    return $outputDiff;
}

function search_in_array($haystack) {
    $path = array();

    $it = new RecursiveIteratorIterator(
        new ParentIterator(new RecursiveArrayIterator($haystack)),
        RecursiveIteratorIterator::SELF_FIRST
    );

    foreach ($it as $key => $value) {

        if (!is_array($value) ){



            for ($i = $it->getDepth() - 1; $i >= 0; $i--) {

                $path = array($it->getSubIterator($i)->key() => $path);

            }

            break;
        }
    }

    return $path;
}


