<?php
spl_autoload_register(function ($class_name) {
    include drupal_get_path('module', 'flat_deposit') . '/Helpers/CMDI/FormBuilder/class.' . $class_name . '.inc';

});

/**
 *
 */
class FormComposite extends FormComponent {

    public function getFormKeyName($templateNode){

        if (isset($templateNode->container)){
            return (string)$templateNode->container;
        }

        return (string)$templateNode['id'];
    }


    public function nestComponentElements($templateNode, $componentElements){

        if (isset($templateNode->container)){
            $id = (string)$templateNode['id'];
            $container[$id] = $componentElements;
            $container['#type'] = 'fieldset';
            $container['#title'] = (string)$templateNode->container;
           return $container;
        }

        return $componentElements;
    }

    /**
     * @param $node
     * @param $componentName
     * @return array|null
     */
    public function extractValuesFromNode($node, $componentName){

        if (!$node OR !$node->{$componentName}) {
            return null;
        }

        $counter = $node->{$componentName}->count();

        if ($counter > 1){
            $this->setNumberElements($this->getNumberElements() + 1);
        }

        return null;
    }


    public function getComponentButtonPresses($templateNode, $pressedButtons){

        $id = (string)$templateNode['id'];

        if (isset($templateNode->container)) {
            $container = (string) $templateNode->container;

            $nPressedButtons = isset($pressedButtons[$container][$id]) ? $pressedButtons[$container][$id] : null;
        } else {

            $nPressedButtons = isset($pressedButtons[$id]) ? $pressedButtons[$id] : null;
        }

        return $nPressedButtons;
    }
}
