<?php

/**
 * Drupal query of user supercollections.
 *
 * @param $pid array of strings containing supercollections to be queried
 * @param $transformed bool put data in flat structure
 *
 * @return mixed array of collection names and pids
 */
function query_user_supercollections($pid=NULL,$transformed=FALSE)
{
    global $user;
    $query = db_select('flat_supercollection', 'p')
        ->fields('p', array('collection_name', 'collection_pid',))
        ->condition('member', $user->name)
        ->orderBy('collection_name');

    if ($pid){$query->condition('collection_pid', $pid, 'IN');}

    $results =$query->execute()
        ->fetchAll(PDO::FETCH_ASSOC);

    $supercollections = array();

    if ($transformed){
        foreach ($results as $result){
            $supercollections['collection_name'][]=$result['collection_name'];
            $supercollections['collection_pid'][]=$result['collection_pid'];
        }
    } else {
        $supercollections = $results;
    }

    return $supercollections;
}




