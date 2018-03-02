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
            $componentData = $data[$container][$id];

        } else {

            $componentData = $data[$id];

        }

        // Unset non numeric component data arrays such as add,remove button
        foreach (array_keys($componentData) as $key){
            if (!is_numeric($key))
                unset($componentData[$key]);

        }


        if (isset($componentData)) {

            foreach ($componentData as $index => $entry) {

                // todo check components attribute minOccurrence and if subcomponents have values
                $isRequired = ((int)$component->attributes()['minOccurrence'] > 0 );
                $hasValue = ($entry != null AND $entry != "" and is_numeric($index) and !is_array($entry));

                if ($hasValue){

                    $converted = $this->convertValue((string)$component->type, $entry);
                    $rootNewComponents->addChild($name, utf8_encode(htmlspecialchars($converted)));

                } elseif ($isRequired){

                    $rootNewComponents->addChild($name);

                    // only add root if children with values exist.
                } elseif ($this->hasNotSearchElement($componentData[$index], '')) {

                    $rootNewComponents->addChild($name);


                } else{

                    continue;
                }


                // Add nested components
                foreach ($component->item as $nestedTemplateNode) {

                    $className = "Cmdi" . (string)$nestedTemplateNode['type'];
                    $nestedCmdiComponent = new $className();

                    $name = (string)$component['name'];

                    $nestedCmdiComponent->setCmdiComponent($rootNewComponents->{$name}[$index], $nestedTemplateNode, $componentData[$index]);


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
