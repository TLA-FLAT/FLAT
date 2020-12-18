<?php
spl_autoload_register(function ($class_name) {
    include drupal_get_path('module', 'flat_deposit') . '/Helpers/CMDI/CmdiCreator/class.' . $class_name . '.inc';

});
/**
 * Created by PhpStorm.
 * User: danrhe
 * Date: 25/01/2018
 * Time: 10:04
 */
class CmdiComposite extends CmdiComponent {

    protected $cmdiComponent;


    /**
     * CmdiComponent constructor.
     */
    public function __construct(){}

    /**
     * Adds cmdi component as child with value from form array to root component
     *
     * @param $rootNewComponents
     *
     * @param $component
     *
     * @param $data
     *
     * @param int $index
     */
    public function setCmdiComponent(&$rootNewComponents, $component, $data, $index = 0){

        $id = (string)$component['id'];

        $name = (string)$component['name'];

        // find place in data array where component data is stored
        if (isset($component->container)) {

            $container = (string)$component->container;
            if (isset($data[$container][$id])) {
                $componentData = $data[$container][$id];
            }

        } else {

            if (isset($data[$id])) {
                $componentData = $data[$id];
            }

        }

        // Unset non numeric component data arrays such as add,remove button
        if (isset($componentData)) {
            foreach (array_keys($componentData) as $key){
                if (!is_numeric($key))
                    unset($componentData[$key]);

            }
        }


        if (isset($componentData)) {

            foreach ($componentData as $index => $entry) {

                // todo check components attribute minOccurrence and if subcomponents have values
                $isRequired = ((int)$component->attributes()['minOccurrence'] > 0 );
                $hasValue = ($entry != null AND $entry != "" and is_numeric($index) and !is_array($entry));
		        $skip = isset($component->item->attributes()['skipParentInCMDI']);
		        $childValues = $this->hasNotSearchElement($componentData[$index], '');
                if ($hasValue){

                    $converted = $this->convertValue((string)$component->type, $entry);
                    $child = $rootNewComponents->addChild($name, htmlspecialchars($converted));
                }

                elseif ($isRequired & !$skip){

                    $child = $rootNewComponents->addChild($name);
                    // only add root if children with values exist.

                }

                elseif ($childValues && !$skip) {

                    $child = $rootNewComponents->addChild($name);

                }

                // Add nested components
		        if ($childValues && !$skip) {
                    foreach ($component->item as $nestedTemplateNode) {

                        $className = "Cmdi" . (string)$nestedTemplateNode['type'];
                        $nestedCmdiComponent = new $className();
                        $name = (string)$component['name'];
                        $nestedCmdiComponent->setCmdiComponent($rootNewComponents->{$name}[$index], $nestedTemplateNode, $componentData[$index]);

                        }
                    }

		        elseif ($childValues) {
                    foreach ($component->item as $nestedTemplateNode) {

                        $className = "Cmdi" . (string)$nestedTemplateNode['type'];
                        $nestedCmdiComponent = new $className();
                        $name = (string)$component['name'];
                        $nestedCmdiComponent->setCmdiComponent($rootNewComponents, $nestedTemplateNode, $componentData[$index]);

                        }
                }

                // Add attributes
                foreach ($component->attribute as $attribute) {

                    $attributeId = (string) $attribute['id'];
                    $attributeName = (string) $attribute['name'];
                    if (isset($componentData[$index][$attributeId][0])) {
                        $attributeData = (string) $componentData[$index][$attributeId][0];
                        if ($attributeData != null and $attributeData != "") {
                            $child->addAttribute($attributeName, $attributeData);
                        }
                    }

                }

            }
        }
    }




    /**
     * Function to search for value in recursive mode.
     *
     * @param  array   $array  Array to search
     *
     * @param  string  $search What to search
     *
     * @param  string  $mode   Mode search for value or key
     * @param  boolean $return Return value or key
     * @return boolean or string|array|object whatever
     */
    private function hasNotSearchElement(array $array, $search) {
        foreach (new RecursiveIteratorIterator(new RecursiveArrayIterator($array)) as $value) {
            if ($search !== $value){
                return true;
            }

        }

        return false;
    }


}
