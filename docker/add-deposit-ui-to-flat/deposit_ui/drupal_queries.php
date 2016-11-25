<?php

/**
 * Drupal query of user collections.
 *
 * @param $pid array of strings containing collections to be queried
 * @param $transformed bool put data in flat structure
 *
 * @return mixed array of collection names and pids
 */
function query_user_collections($pid=NULL,$transformed=FALSE)
{
    global $user;
    $query = db_select('flat_collection', 'p')
        ->fields('p', array('collection_name', 'collection_pid',))
        ->condition('member', $user->name)
        ->orderBy('collection_name');

    if ($pid){$query->condition('collection_pid', $pid, 'IN');}

    $results =$query->execute()
        ->fetchAll(PDO::FETCH_ASSOC);

    $collections = array();

    if ($transformed){
        foreach ($results as $result){
            $collections['collection_name'][]=$result['collection_name'];
            $collections['collection_pid'][]=$result['collection_pid'];
        }
    } else {
        $collections = $results;
    }

    return $collections;
}




