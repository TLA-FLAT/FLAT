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
        'host_ip' => '192.168.99.100',
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
function ingest_service_configuration()
{
    return array(
        # the bagit executable
        'bag_exe' => '/usr/local/bin/bagit-4.9.0/bin/bag',
        # FEDORA HOME
        'fedora_home' => '/var/www/fedora',
        # Should errors be logged
        'log_errors' => TRUE,
        # directory to log errors)
        'error_log_file' => '/app/flat/backend/Ingest_service_error.log',
        # sword upload script
        'sword_script' => "/app/flat/do-sword-upload.sh",
        #CMDI 2 DC transformation
        'cmd2dc' => '/app/flat/cmd2dc.xsl',
        #Lat2 fox java lib
        'lat2fox' => '/app/flat/lib/lat2fox.jar');

}

function get_handle_configuration()
{
    $site_configuration = drupal_realpath(drupal_get_path('module', 'flat_deposit_ui')) . '/inc/config_MPI.inc';
    if (file_exists($site_configuration)){
        require_once ($site_configuration);
        return MPI_handle_configuration();
    } else {

        return array(
            # Whether or not to use a handle server; If false, locally unique IDs will be generated instead
            'test_mode' => TRUE,
            'prefix' => 'hdl:',
            'schema' => 'http',
            'host_name' => 'hostname',
            'port' => 8000,

        );
    }
}


function get_metadata_configuration()
{
    return array(
        'site_name' => 'MPI',
        'MdProfile' => 'clarin.eu:cr1:p_1407745712035',
        'prefix' => 'lat',
        'ext' => '.cmdi'
    );

}


function get_solr_configuration()
{
    $site_configuration = drupal_realpath(drupal_get_path('module', 'flat_deposit_ui')) . '/inc/config_MPI.inc';
    if (file_exists($site_configuration)) {
        require_once($site_configuration);
        return MPI_solr_configuration();
    } else{
    return array(
        'schema' => 'https',
        'host_name' => 'lux125.mpi.nl',
        'port' => 8080,
        'path' => 'solr',
        'core' => 'collection1',
    );
    }
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
        'admin_name' => 'admin',
        'admin_pass' => 'admin',
        'host' => $_SERVER['HTTP_HOST'],
        'root_dir'=> '/var/www/owncloud/'));

}

/**
 * Default values of new bundle entries when created in drupal upload log table
 */
function get_initial_bundle_profile()
{
    return array('user_id' => '',
        'bundle' => '',
        'pid_bundle' => '',
        'collection' => '',
        'status' => 'open',
        'bag_id' => '',
        'exceptions' => '',
        'freeze_time' => 0,
        'nfiles' => 0,
        'baginplace' => 0,
        'zipped' => 0,
        'bagged' => 0,
        'foxml' => 0,
        'bag_ingested' => 0,
        'data_purged' => 0,
        'owner_added' => 0,
        'date_bundle_ingest' => 0,
    );
}