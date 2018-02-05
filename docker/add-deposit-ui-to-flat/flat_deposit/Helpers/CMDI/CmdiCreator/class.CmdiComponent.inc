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
abstract class CmdiComponent
{

    protected $cmdiComponent;


    /**
     * CmdiComponent constructor.
     */
    public function __construct(){}

    abstract public function setCmdiComponent(&$rootNewComponents, $component, $data);



    protected function convertValue($type, $value){

        switch ($type){
            case 'date':
                $month = (strlen($value['month']) == 2 ) ? $value['month'] : '0'. $value['month'];
                $day = (strlen($value['day']) == 2 ) ? $value['day'] : '0'. $value['day'];
                $date = $value['year'] . '-' . $month . '-' . $day;
                $converted = $date;
                break;
            default :
                $converted = $value;
        }

        return $converted;

    }

}
