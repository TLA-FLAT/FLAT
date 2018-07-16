<?php
spl_autoload_register(function ($class_name) {
    include drupal_get_path('module', 'flat_deposit') . '/Helpers/CMDI/FormBuilder/class.' . $class_name . '.inc';

});

/**
 * Created by PhpStorm.
 * User: danrhe
 * Date: 04/01/2018
 * Time: 14:16
 */
class FormLeaf extends FormComponent {


    /**
     * Determines the name of node which will be added to the new form array
     * @return string
     */
    public function getFormKeyName($templateNode){

        return (string)$templateNode['id'];
    }


    /**
     * Form leafs are not nested deeply.
     *
     * @return string Array with component elements
     */
    public function nestComponentElements($templateNode, $componentElements){


        return $componentElements;
    }

    public function addNestedComponents(&$componentElements, $templateNode, $existingCmdiNode, $pressedButtons, $index){

    }

    /**
     * @param $node
     * @param $componentName
     * @return array|null
     */
    public function extractValuesFromNode($node, $componentName){
        if (!$node) {
            return null;
        }

        if (!$node->{$componentName} OR get_class($node->{$componentName}) != 'CmdiHandler') {
            return null;

        }

        $counter = 0;
        foreach ($node->{$componentName} as $value){

            $values[] = (string)$value;
            $counter++;

            if ($counter > 1){
                $this->setNumberElements($this->getNumberElements() +1);
            }

        }

        return $values;

    }


    public function getComponentButtonPresses($templateNode, $pressedButtons){
        $id = (string)$templateNode['id'];

        return isset($pressedButtons[$id]) ? $pressedButtons[$id] : null;
    }


}
