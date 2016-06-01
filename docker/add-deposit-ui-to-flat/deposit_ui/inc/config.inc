<?php
/**
 * Created by PhpStorm.
 * User: danrhe
 * Date: 10/05/16
 * Time: 11:44
 */

// drupal mysql/postgres database connection settings.
// The string is used by the ingest service
$host = "localhost";
$port = "5432";
$dbname = "drupal";
$user = "fedora";
$pw = "fedora";
$conn_string = "host=$host port=$port dbname=$dbname user=$user password=$pw";


/**
 * Returns the fedora database settings.
 * @return array
 */

function get_configuration_fedora(){
    return array(
        'host_ip' => 'https://192.168.99.100',
        'port' => 8443,
        'user' => 'fedoraAdmin',
        'password' => 'fedora'
    );
}

/**
 *initial bundle profile (used to generate new entries in table "flat_deposit_ui_upload_log"
 */
$intial_bundle_profile = array(
    'user_id' => '',
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
