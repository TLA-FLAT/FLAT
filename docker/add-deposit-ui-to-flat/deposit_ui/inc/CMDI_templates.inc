<?php

/**
 * Class IngestServiceException is an exception class
 */
class CMDITemplateException extends Exception {}

/**
 * Created by PhpStorm.
 * User: danrhe
 * Date: 16/06/16
 * Time: 11:49
 */
class CMDI_templates
{
    public $fields;

    function __construct($template_name){
        $this->Template_Name = $template_name;
    }


    /**
     * Method that returns a drupal form array
     * @param $filled_in_fields
     * @throws CMDITemplateExceptio
     */
    function getTemplate($filled_in_fields)
    {

        switch ($this->Template_Name) {
            case 'session':
                $fields = array();

                $fields['field_1'] = array('#title' => t(strtoupper($this->Template_Name) . ' Template'),'#type' => 'fieldset', '#collapsible' => TRUE, '#collapsed' => FALSE, '#tree' => TRUE);
                $fields['field_1']['Name'] = array('#title' => t('Name'),'#type' => 'textfield','#default_value'=>$filled_in_fields['field_1']['Name']);
                $fields['field_1']['Title'] = array('#title' => t('Title'), '#type' => 'textfield','#default_value'=>$filled_in_fields['field_1']['Title']);
                $fields['field_1']['Date'] = array('#title' => t('Date'), '#type' => 'date','#default_value'=>$filled_in_fields['field_1']['Date']);
                $fields['field_1']['descriptions'] = array('#title' => t('descriptions'), '#type' => 'fieldset', '#collapsible' => TRUE, '#collapsed' => TRUE, '#tree' => TRUE);
                $fields['field_1']['descriptions'] ['Description'] = array('#title' => t('Description'), '#type' => 'textfield', '#default_value'=>$filled_in_fields['field_1']['descriptions']['Description']);
                $fields['Location'] = array('#title' => t('Location'),'#type' => 'fieldset', '#collapsible' => TRUE, '#collapsed' => FALSE, '#tree' => TRUE);
                $fields['Location']['Continent'] = array('#title' => t('Continent'), '#type' => 'textfield','#default_value'=>$filled_in_fields['Location']['Continent']);
                $fields['Location']['Country'] = array('#title' => t('Country'), '#type' => 'textfield', '#default_value'=>$filled_in_fields['Location']['Country']);
                $fields['Location']['Region'] = array('#title' => t('Region'), '#type' => 'textfield','#default_value'=>$filled_in_fields['Location']['Region']);
                $fields['Location']['Address'] = array('#title' => t('Address'), '#type' => 'textfield', '#default_value'=>$filled_in_fields['Location']['Address']);
                $fields['Project'] = array('#title' => t('Project'),'#type' => 'fieldset', '#collapsible' => TRUE, '#collapsed' => FALSE, '#tree' => TRUE);
                $fields['Project']['Name'] = array('#title' => t('Name'), '#type' => 'textfield','#default_value'=>$filled_in_fields['Project']['Name'],'#default_value'=>$filled_in_fields['Project']['Name']);
                $fields['Project']['Title'] = array('#title' => t('Title'), '#type' => 'textfield','#default_value'=>$filled_in_fields['Project']['Title']);
                $fields['Project']['Id'] = array('#title' => t('Id'), '#type' => 'textfield','#default_value'=>$filled_in_fields['Project']['Name'],'#default_value'=>$filled_in_fields['Project']['Id']);
                $fields['Project']['Contact'] = array('#title' => t('Contact'),'#type' => 'fieldset', '#collapsible' => TRUE, '#collapsed' => FALSE, '#tree' => TRUE);
                $fields['Project']['Contact']['Name'] = array('#title' => t('Name'), '#type' => 'textfield','#default_value'=>$filled_in_fields['Project']['Contact']['Name']);
                $fields['Project']['Contact']['Email'] = array('#title' => t('Email'), '#type' => 'textfield','#default_value'=>$filled_in_fields['Project']['Contact']['Email']);
                $fields['Project']['Contact']['Organisation'] = array('#title' => t('Organisation'), '#type' => 'textfield','#default_value'=>$filled_in_fields['Project']['Contact']['Organisation']);
                $fields['Project']['descriptions'] = array('#title' => t('descriptions'),'#type' => 'fieldset', '#collapsible' => TRUE, '#collapsed' => FALSE, '#tree' => TRUE);
                $fields['Project']['descriptions'] ['Description'] = array('#title' => t('descriptions'),'#type' => 'textfield', '#default_value'=>$filled_in_fields['Project']['descriptions']['Description']);

                $this->fields = $fields;
                break;

            case 'minimal':
                $fields['field_1'] = array('#title' => t(strtoupper($this->Template_Name) . ' Template'),'#type' => 'fieldset', '#collapsible' => TRUE, '#collapsed' => FALSE, '#tree' => TRUE);
                $fields['field_1']['Name'] = array('#title' => t('Name'),'#type' => 'textfield','#default_value'=>$filled_in_fields['field_1']['Name']);
                $fields['field_1']['Title'] = array('#title' => t('Title'), '#type' => 'textfield','#default_value'=>$filled_in_fields['field_1']['Title']);
                $fields['field_1']['Date'] = array('#title' => t('Date'), '#type' => 'date','#default_value'=>$filled_in_fields['field_1']['Date']);
                $fields['Project'] = array('#title' => t('Project'),'#type' => 'fieldset', '#collapsible' => TRUE, '#collapsed' => FALSE, '#tree' => TRUE);
                $fields['Project']['Name'] = array('#title' => t('Name'), '#type' => 'textfield');
                $fields['Project']['Title'] = array('#title' => t('Title'), '#type' => 'textfield');
                $fields['Project']['Id'] = array('#title' => t('Id'), '#type' => 'textfield');
                $fields['Project']['Contact'] = array('#title' => t('Contact'),'#type' => 'fieldset', '#collapsible' => TRUE, '#collapsed' => FALSE, '#tree' => TRUE);
                $fields['Project']['Contact']['Name'] = array('#title' => t('Name'), '#type' => 'textfield');
                $fields['Project']['Contact']['Email'] = array('#title' => t('Email'), '#type' => 'textfield');
                $fields['Project']['Contact']['Organisation'] = array('#title' => t('Organisation'), '#type' => 'textfield');
                $fields['Project']['descriptions'] = array('#title' => t('descriptions'),'#type' => 'fieldset', '#collapsible' => TRUE, '#collapsed' => FALSE, '#tree' => TRUE);
                $fields['Project']['descriptions'] ['Description'] = array('#title' => t('descriptions'),'#type' => 'textfield', '#default_value'=>$filled_in_fields['Project']['descriptions']['Description']);
                $this->fields = $fields;
                break;



            case 'experiment':
                $fields['field_1'] = array('#title' => t(strtoupper($this->Template_Name) . ' Template'),'#type' => 'fieldset', '#collapsible' => TRUE, '#collapsed' => FALSE, '#tree' => TRUE);
                $fields['field_1']['Name'] = array('#title' => t('Name'),'#type' => 'textfield','#default_value'=>$filled_in_fields['field_1']['Name']);
                $fields['field_1']['Date'] = array('#title' => t('Date'), '#type' => 'date','#default_value'=>$filled_in_fields['field_1']['Date']);
                $fields['Experiment'] = array('#title' => t('Experiment'),'#type' => 'fieldset', '#collapsible' => TRUE, '#collapsed' => FALSE, '#tree' => TRUE);
                $fields['Experiment']['Title'] = array('#title' => t('Title'), '#type' => 'textfield', '#default_value'=>$filled_in_fields['Experiment']['Title']);
                $fields['Experiment']['Notebook_Name'] = array('#title' => t('Notebook Name'), '#type' => 'textfield', '#default_value'=>$filled_in_fields['Experiment']['Notebook_Name']);
                $fields['Experiment']['Contact'] = array('#title' => t('Contact'),'#type' => 'fieldset', '#collapsible' => TRUE, '#collapsed' => FALSE, '#tree' => TRUE);
                $fields['Experiment']['Contact']['User_Name'] = array('#title' => t('User Full Name'), '#type' => 'textfield', '#default_value'=>$filled_in_fields['Experiment']['Contact']['User_Name']);
                $fields['Experiment']['Contact']['Tel_number'] = array('#title' => t('Phone Number'), '#type' => 'textfield', '#default_value'=>$filled_in_fields['Experiment']['Contact']['Tel_number']);
                $fields['Experiment']['Contact']['Email'] = array('#title' => t('Email'), '#type' => 'textfield', '#default_value'=>$filled_in_fields['Experiment']['Contact']['Email']);
                $fields['Experiment']['descriptions'] = array('#title' => t('descriptions'),'#type' => 'fieldset', '#collapsible' => TRUE, '#collapsed' => FALSE, '#tree' => TRUE);
                $fields['Experiment']['descriptions'] ['Description'] = array('#title' => t('descriptions'),'#type' => 'textfield', '#default_value'=>$filled_in_fields['Experiment']['descriptions']['Description']);
                $fields['Experiment']['conclusions'] = array('#title' => t('Experiment description'),'#type' => 'fieldset', '#collapsible' => TRUE, '#collapsed' => FALSE, '#tree' => TRUE);
                $fields['Experiment']['conclusions'] ['Conclusion'] = array('#title' => t('Conclusion'),'#type' => 'textfield', '#default_value'=>$filled_in_fields['Experiment']['conclusions']['Conclusion']);
                $this->fields = $fields;
                break;

            default:
                $this->fields = FALSE;
                throw new CMDITemplateException ('Template fields have not been specified');
        }
    }
}



     
