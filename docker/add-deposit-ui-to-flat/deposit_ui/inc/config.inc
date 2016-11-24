<?php

/*
DEFAULT SETTINGS OF THE FLAT_DEPOSIT MODULE AND FUNCTIONS TO SET THESE
*/

function flat_deposit_set_paths()
{
    $flat_deposit_paths = variable_get('flat_deposit_paths', array());

    //already installed
    if (!empty($flat_deposit_paths[0])) {
        variable_del('flat_deposit_paths');
    }
    variable_set('flat_deposit_paths', array(
            #backend directory for freeze of data
            'freeze' => '/app/flat/backend/',
            #backend directory where bags will be placed
            'bag'=> "/app/flat/deposit/bags/",
            # directory sword uses to temporarily save bags
            'sword_tmp_dir' => '/app/flat/deposit/sword/tmp/',
            # root of the alternative directory where ingest service can search for archiving files.
            # This root need to contain user data folders named just as the LDAP login name
            'alternate_dir' => '/var/www/owncloud/data/',
            # (optional) subdirectory following user login name (e.g. in owncloud this is '/files')
            'alternate_subdir' => '/files')
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

function flat_deposit_set_metadata()
{
    $flat_deposit_metadata = variable_get('flat_deposit_metadata', array());

    //already installed
    if (!empty($flat_deposit_metadata[0])) {
        variable_del('flat_deposit_metadata');
    }
    variable_set('flat_deposit_metadata', array(
            'site' => 'MPI',
            'type' => 'cmdi',
            'prefix' => 'lat',
            'MdProfile' => 'clarin.eu:cr1:p_1407745712035',

        )
    );

    return TRUE;
}

function flat_deposit_set_ingest_service()
{
    $flat_deposit_ingest_service = variable_get('flat_deposit_ingest_service', array());
    //already installed
    if (!empty($flat_deposit_ingest_service[0])) {
        variable_del('flat_deposit_ingest_service');
    }
    variable_set('flat_deposit_ingest_service', array(
            # the bagit executable
            'bag_exe' => '/usr/local/bin/bagit-4.9.0/bin/bag',
            # Should errors be logged
            'log_errors' => TRUE,
            # directory to log errors)
            'error_log_file' => '/app/flat/backend/Ingest_service_error.log',
            # Should everything be logged
            'log_all' => TRUE,
            # directory to log errors)
            'log_file_dir' => '/app/flat/backend/',
        )
    );

    return TRUE;
}


function flat_deposit_set_sword()
{
    $flat_deposit_sword = variable_get('flat_deposit_sword', array());
    //already installed
    if (!empty($flat_deposit_sword[0])) {
        variable_del('flat_deposit_sword');
    }
    variable_set('flat_deposit_sword', array(
        'url' => 'https://localhost/easy-deposit/',
        'port'=> 8443,
        'user'=> 'flat',
        'password'=> 'sword',
        )
    );

    return TRUE;
}


function flat_deposit_set_doorkeeper()
{
    $flat_deposit_doorkeeper = variable_get('flat_deposit_doorkeeper', array());
    //already installed
    if (!empty($flat_deposit_doorkeeper[0])) {
        variable_del('flat_deposit_doorkeeper');
    }
    variable_set('flat_deposit_doorkeeper', array(
            'url' => "http://localhost/flat/doorkeeper/",
            'port' => 8080
        )
    );

    return TRUE;
}


function flat_deposit_set_fedora()

{
    $flat_deposit_fedora = variable_get('flat_deposit_fedora', array());
    //already installed
    if (!empty($flat_deposit_fedora[0])) {
        variable_del('flat_deposit_fedora');
    }
    variable_set('flat_deposit_fedora', array(
            'host_name' => 'localhost',
            'host_ip' => '127.0.0.1',
            'port' => 8443,
            'scheme' => 'https',
            'user' => 'fedoraAdmin',
            'password' => 'fedora',
            'context' => 'fedora',

        )
    );

    return TRUE;
}

function flat_deposit_set_solr()
{
    $flat_deposit_solr = variable_get('flat_deposit_solr', array());
    //already installed
    if (!empty($flat_deposit_solr[0])) {
        variable_del('flat_deposit_solr');
    }
    variable_set('flat_deposit_solr', array(
            'schema' => 'https',
            'host_name' => 'lux125.mpi.nl',
            'port' => 8080,
            'path' => 'solr',
            'core' => 'collection1',
        )
    );

    return TRUE;
}

