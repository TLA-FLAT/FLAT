<?php

/**
 * Created by PhpStorm.
 * User: danrhe
 * Date: 10/10/2017
 * Time: 16:17
 */
class FlatTuque
{
    /**
     * Performs a Mulgara sparql query to retrieve all children labels of a given fedora object.
     *
     * @param $pid string fedora ID of an object.
     * @return array Strings with pid
     */
    public static function getChildrenLabels($pid)
    {

        $config = variable_get('flat_deposit_general');
        $namespace = $config['namespace'];

        $values = false;
        $fObj = islandora_object_load($pid);
        if ($fObj) {
            $values = [];
            module_load_include('php', 'flat_deposit', '/inc/fedora_queries');

            $query = query_existing_labels_in_collection($pid);

            $results = $fObj->repository->ri->sparqlQuery($query);
            foreach ($results as $result) {
                if (preg_match("/^$namespace/", $result['pid']['value'])) {
                    $values[] = $result['label']['value'];
                }
            }
        }
        return $values;
    }

    /**
     * Gets RELS-EXT 'isPartOfCollection' property for a certain fedora object
     *
     * @param $fid fedora ID
     * @return bool|String value of the isMemberOfCollection property or false
     */
    public static function getIsPartOfCollection($fid)
    {
        $return_value = false;

        $fObj = islandora_object_load($fid);
        if ($fObj) {
            $results = $fObj->relationships->get(FEDORA_RELS_EXT_URI, 'isMemberOfCollection');
            $count = 0;
            foreach ($results as $result) {
                $value = $result['object']['value'];
                $count ++;
            }

            if ($count == 1) {
                $return_value = $value;
            }
        }
        return $return_value;
    }

    /**
     * Gets RELS-EXT 'isPartOfCollection' property for a certain fedora object
     *
     * @param $fid fedora ID
     * @return bool|String value of the isMemberOfCollection property or false
     */
    public static function getCModels($fedoraObject)
    {
        $model_array = [];

        $cmodels = $fedoraObject->relationships->get('info:fedora/fedora-system:def/model#', 'hasModel');

        foreach ($cmodels as $cmodel) {
            $model_array[] = $cmodel['object']['value'];
        }
        return $model_array;
    }
}
