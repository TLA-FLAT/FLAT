<?php


function flat_deposit_set_owncloud(){

    $owncloud_settings = variable_get('flat_owncloud', array());

    //already installed
    if (!empty($owncloud_settings[0])) {
        variable_del('flat_owncloud');
    }
    variable_set('flat_owncloud', array(
        'activated' => TRUE,
        'admin_name' => 'admin',
        'admin_pass' => 'admin',
        'host' => $_SERVER['HTTP_HOST'],
        'root_dir'=> '/var/www/owncloud/',
        'temp_dir'=> '/tmp/'));

}
