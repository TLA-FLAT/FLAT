<?php


function flat_workspaces_set_variables($updated_values=NULL){

    $defaults =  array(
        #root path for locally stored data
        'mount_folder' => '/app/flat/deposit/local/',
        'workspace_folder' => 'workspaces',
        'archive_folder' => 'archive_deposit',
        );

    //clean up stored values
    $values_stored_in_database = variable_get('flat_workspaces', array());

    //already installed
    if (!empty($values_stored_in_database[0])) {variable_del('flat_workspaces'); }

    // set or update values
    $values = $updated_values ? array_merge($values_stored_in_database, $updated_values) : $defaults;
    variable_set('flat_workspaces', $values );

    return TRUE;
}

