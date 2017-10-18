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
     * @param $pid string fedpra ID of an object.
     * @return array Strings with pid
     */
    static public function getChildrenLabels($pid){
        $values = false;
        $fObj = islandora_object_load($pid);
        if ($fObj){

            $values = [];
            module_load_include('php', 'flat_deposit', '/inc/fedora_queries');

            $query = query_existing_labels_in_collection($pid);

            $results = $fObj->repository->ri->sparqlQuery($query);
            foreach ($results as $result) {

                $values[] = $result['label']['value'];
            }

        }
        return $values;


    }



}