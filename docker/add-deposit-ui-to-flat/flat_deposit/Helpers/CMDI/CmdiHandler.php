<?php

module_load_include('inc','flat_deposit','Helpers/CMDI/Template2FormParser');
module_load_include('inc','flat_deposit','Helpers/CMDI/Drupal2CmdiParser');


class CmdiHandlerException extends Exception {}
/**
 * Interface CmdiProfile
 */
class CmdiHandler
{

    const FORM_TEMPLATES_PATH = '/Helpers/CMDI/FormTemplates/';


    /**
     * Loads specified file as SimpleXML object.
     *
     * @param $fileName
     *
     * @return SimpleXMLElement
     *
     * @throws CmdiProfileException
     */
    static public function loadXml($fileName)
    {

        if (simplexml_load_file($fileName) === false) {

            $message = 'Error loading schema file. ';

            foreach (libxml_get_errors() as $error) {
                $message .= "Line: $error->line($error->column) $error->message <br>";
            }

            return $message;

        } else {
            $xml = simplexml_load_file($fileName);
            return $xml;
        }
    }

    /**
     * Generates the drupal input form for a specified profile
     *
     * @return mixed array containing renderable form array or false
     */
    static public function generateDrupalForm($profile)
    {

        $fName = drupal_get_path('module', 'flat_deposit') . CmdiHandler::FORM_TEMPLATES_PATH . $profile . '.xml';

        $template = CmdiHandler::loadXml($fName);

        if (is_string($template)) {
            return $template;
        }

        $parser = new Template2FormParser();
        $form = $parser->buildDrupalForm($template);


        return $form;

    }

    /**
     * Generates a new cmdi file based on an xml parser file
     *
     * @param $user_name
     *
     * @param $form_data
     *
     * @return SimpleXMLElement|string
     */
    static public function generateCmdi($profile, $user_name, $form_data)
    {

        $fName = drupal_get_path('module', 'flat_deposit') . CmdiHandler::FORM_TEMPLATES_PATH . $profile . '.xml';

        $template = CmdiHandler::loadXml($fName);

        if (is_string($template)) {
            return $template;
        }

        $parser = new Drupal2CmdiParser();
        $cmdi = $parser->buildCmdi($profile, $template, $user_name, $form_data);

        return $cmdi;


    }


    /**
     * @param $xml SimpleXMLElement cmdi xml file
     * @param $parent_pid String fedora identifier of the parent
     */
    static public function addIsPartOfProperty(&$xml, $parent_pid)
    {

        // Add isPartOf property to xml
        if (!isset($xml->Resources->IsPartOfList)) {
            $xml->Resources->addChild('IsPartOfList');
        }
        $xml->Resources->IsPartOfList->addChild('IsPartOf', $parent_pid);


    }


}