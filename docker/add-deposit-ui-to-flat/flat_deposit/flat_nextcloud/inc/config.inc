<?php


function flat_nextcloud_set_nextcloud($updated_values=NULL){

    $defaults =  array(
        'activated' => FALSE,
        'admin_name' => 'admin',
        'admin_pass' => 'admin',
        'schema' => 'https',
        'host' => $_SERVER['HTTP_HOST'],
        'root_dir'=> '/var/www/nextcloud/',
        'data_dir'=> '/var/www/nextcloud/data/',
        );

    //clean up stored values
    $values_stored_in_database = variable_get('flat_nextcloud', array());

    //already installed
    if (!empty($values_stored_in_database[0])) {variable_del('flat_nextcloud'); }

    // set or update values
    $values = $updated_values ? array_merge($values_stored_in_database, $updated_values) : $defaults;
    variable_set('flat_nextcloud', $values );

    return TRUE;
}

