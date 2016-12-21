<?php


function flat_owncloud_set_owncloud($updated_values=NULL){

    $defaults =  array(
        'activated' => FALSE,
        'admin_name' => 'admin',
        'admin_pass' => 'admin',
        'schema' => 'https',
        'host' => $_SERVER['HTTP_HOST'],
        'root_dir'=> '/var/www/owncloud/',
        'temp_dir'=> '/tmp/',
        );

    //clean up stored values
    $values_stored_in_database = variable_get('flat_owncloud', array());

    //already installed
    if (!empty($values_stored_in_database[0])) {variable_del('flat_owncloud'); }

    // set or update values
    $values = $updated_values ? array_merge($values_stored_in_database, $updated_values) : $defaults;
    variable_set('flat_owncloud', $values );

    return TRUE;
}

