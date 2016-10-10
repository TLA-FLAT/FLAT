<?php
/**
 * Created by PhpStorm.
 * User: danrhe
 * Date: 10/05/16
 * Time: 11:44
 */

/**
 * CONFIGURATION OF THE INGEST SERVICE
 */

//
/**
 * drupal mysql/postgres database connection settings.
 * @return array credentials and server settings
 */
function get_drupal_database_settings(){
    return array(
    'host' => "localhost",
    'port' => "5432",
    'dbname' => "drupal",
    'user' => "fedora",
    'pw' => "fedora",);
}


/**
 * Returns the fedora database settings.
 * @return array
 */
function get_configuration_fedora(){
    return array(
        'host_name' => 'localhost',
        'host_ip' => '127.0.0.1',
        'port' => 8443,
        'scheme' => 'https',
        'user' => 'fedoraAdmin',
        'password' => 'fedora',
        'context' => 'fedora',

    );
}


/**
 * Called by flat_deposit_ui_commit_form_ingest_serve_submit.
 * @return array paths and executable names which are passed to the ingest service
 */
function get_configuration_ingest_service()
{
    return array(
        # the bagit executable
        'bag_exe' => '/usr/local/bin/bagit-4.9.0/bin/bag',
        # Should errors be logged
        'log_errors' => TRUE,
        # directory to log errors)
        'error_log_file' => '/app/flat/backend/Ingest_service_error.log',
        # directory sword uses to temporarily save bags
        'sword_tmp_dir' => '/app/flat/deposit/sword/tmp/',
        # root of the alternative directory where ingest service can search for archiving files.
        # This root need to contain user data folders named just as the LDAP login name
        'alternate_dir' => '/var/www/owncloud/data/',
        # (optional) subdirectory following user login name (e.g. in owncloud this is '/files')
        'alternate_subdir' => '/files'

    );
}


function get_configuration_metadata()
{
    return array(
        'site_name' => 'MPI',
        'MdProfile' => 'clarin.eu:cr1:p_1407745712035',
        'prefix' => 'lat',
        'ext' => '.cmdi'
    );

}

function get_configuration_sword()
{
    return array(
        'url' => 'https://localhost/easy-deposit/',
        'port'=> 8443,
        'user'=> 'flat',
        'password'=> 'sword',
    );

}

function get_configuration_doorkeeper()
{
    return array(
        'url' => "http://localhost/flat/doorkeeper/",
        'port' => 8080)
        ;
}

function get_configuration_solr()
{
    return array(
        'schema' => 'https',
        'host_name' => 'lux125.mpi.nl',
        'port' => 8080,
        'path' => 'solr',
        'core' => 'collection1',
    );
}

/*
DEFAULT SETTINGS OF THE FLAT_DEPOSIT_UI MODULE AND FUNCTIONS TO SET THESE
*/

function flat_deposit_set_paths()
{
    $flat_deposit_paths = variable_get('flat_deposit_paths', array());

    //already installed
    if (!empty($flat_deposit_paths[0])) {
        variable_del('flat_deposit_paths');
    }
    variable_set('flat_deposit_paths', array(
            'freeze' => '/app/flat/backend/',
            'bag'=> "/app/flat/deposit/bags/",)
    );
    return TRUE;
}


function flat_deposit_set_names()
{
    $flat_deposit_names = variable_get('flat_deposit_names', array());

    //already installed
    if (!empty($flat_deposit_names[0])) {
        variable_del('flat_deposit_names');
    }
    variable_set('flat_deposit_names', array(
            'apache_user' => 'www-data'
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
        'activated' => TRUE,
        'admin_name' => 'admin',
        'admin_pass' => 'admin',
        'host' => $_SERVER['HTTP_HOST'],
        'root_dir'=> '/var/www/owncloud/',
        'temp_dir'=> '/tmp/'));

}
