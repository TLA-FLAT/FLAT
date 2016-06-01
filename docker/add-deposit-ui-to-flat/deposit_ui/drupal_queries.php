<?php


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


