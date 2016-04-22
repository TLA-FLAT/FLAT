<?php
/**
 * Created by PhpStorm.
 * User: danrhe
 * Date: 18/04/16
 * Time: 17:10
 */
function flat_deposit_set_paths()
{
    $flat_deposit_paths = variable_get('flat_deposit_paths', array());

    //already installed
    if (!empty($flat_deposit_paths[0])) {
        variable_del('flat_deposit_paths');
    }
    variable_set('flat_deposit_paths', array(
            'freeze' => '/app/flat/backend',
            'temp' => "/app/flat/deposit/",
            'bag'=> "/app/flat/deposit/bags/")
    );

    return TRUE;
}


function flat_deposit_set_modules()
{
    $flat_deposit_modules = variable_get('flat_deposit_modules', array());

    //already installed
    if (!empty($flat_deposit_modules[0])) {
        variable_del('flat_deposit_modules');
    }
    variable_set('flat_deposit_modules', array(
            'owncloud' => TRUE
        )
    );

    return TRUE;
}

function flat_deposit_set_owncloud(){

    $owncloud_settings = variable_get('owncloud_settings', array());

    //already installed
    if (!empty($owncloud_settings[0])) {
        variable_del('owncloud_settings');
    }
    variable_set('owncloud_settings', array(
        'admin_name' => 'admin',
        'admin_pass' => 'admin',
        'root_dir'=> '/var/www/owncloud/'));



}