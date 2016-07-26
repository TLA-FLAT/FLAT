<?php

function dq_user_is_member_collections($conditions=[],$paged=FALSE){

    $obj= db_select('flat_deposit_ui_collection','p')
    ->fields('p',array('collection_name','member'))
    ->orderBy('collection_name');

    if (!empty($conditions)){
        foreach ($conditions as $key => $val){$obj->condition($key,$val);}
    }

    if ($paged){
        $obj = $obj->extend('PagerDefault')->limit($paged);
    }
    return $obj->execute();

}

function dq_user_bundle_data($fields,$conditions=[],$count=FALSE){
    $obj= db_select('flat_deposit_ui_upload','p')

        ->fields('p',$fields)
        ->condition('user_id', USER)
        ->orderBy('collection')
        ->orderBy('bundle');

    if (!empty($conditions)){
        foreach ($conditions as $key => $val){
            if (!is_array($val)){$obj->condition($key,$val);
            } else {
                $obj->condition($key,$val,'IN');
            }
        }
    }

    if ($count){return $obj->execute()
        ->rowCount();
    } else{
        return  $obj->execute();
    }

}


function dq_update_existing_user_bundle($fields, $conditions=[]){
    $obj = db_update('flat_deposit_ui_upload')
    ->fields($fields)
    ->condition('user_id', USER);

    if (!empty($conditions)){
        foreach ($conditions as $key => $val){$obj->condition($key,$val);}
    }


    return $obj->execute();
}

function dq_purge_bundle($uid){
    return db_delete('flat_deposit_ui_upload')
    ->condition('uid', $uid)
    ->execute();
}

/*
function clear_db_user_bundles($user){
    $results = db_delete('flat_deposit_ui_project_info')
        ->condition('user_id', $user)
        ->execute();
    return $results;
}


function refresh_db_user_projects($user){
    $results = db_select('flat_deposit_ui_project_info')
        ->fields('pname')
        ->condition('user_id', $user)
        ->execute();
    return $results;
}

function insert_db_user_projects($user, $project, $is_frozen){
    if (!$is_frozen) {$is_frozen = '0';};
    $res = db_insert('flat_deposit_ui_project_info')
        ->fields(array(
            'user_id' => $user,
            'pname' => $project,
            'is_frozen' => $is_frozen,
            'freeze_date' => REQUEST_TIME,
        ))
        ->execute();
    return $res;
}

function select_db_user_all_projects($user)
{
    $entries = array();

    $results = db_select('flat_deposit_ui_project_info', 'p')
        ->fields('p', array('pname', 'is_frozen'))
        ->condition('user_id', $user)
        ->execute();
    foreach ($results as $row) {
        $entries[] = $row;
    }
    return $entries;
}

function check_entry_exists($user, $project){
    $bool = FALSE;
    $entries = array();


    $results = db_select('flat_deposit_ui_project_info','p')
        ->fields('p',array('pname'))
        ->condition('user_id', $user)
        ->condition('pname', $project)
        ->execute();
    foreach ($results as $row){
        array_push($entries, $row);
    }
    if (count($entries) >0){$bool=TRUE;}
    return $bool;
}

function update_db_user_projects($user, $project, $is_frozen)
{
    if (!$is_frozen) {$is_frozen = '0';};
    db_update('flat_deposit_ui_project_info')
        ->fields(array(
            'is_frozen' => $is_frozen))
        ->condition('user_id', $user)
        ->condition('pname', $project)
        ->execute();
}


*/