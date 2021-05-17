<?php

/*
DEFAULT SETTINGS OF THE FLAT_DEPOSIT MODULE AND FUNCTIONS TO SET THESE
*/

function flat_deposit_set_general($updated_values = null)
{
    $defaults =  array(
        #fedora namespace for ingests
        'namespace' => 'lat',
        #directory for uploading external data
        'external' => '/var/www/html/flat/sites/default/files/private/flat_deposit/data',
        #directory for storing metadata
        'metadata' => '/var/www/html/flat/sites/default/files/private/flat_deposit/metadata',
        #backend directory for freeze of data
        'freeze' => '/app/flat/backend/',
    );

    //remove already installed variables
    $values_stored_in_database = variable_get('flat_deposit_general', array());

    if (!empty($values_stored_in_database[0])) {
        variable_del('flat_deposit_general');
    }


    // set or update values
    $values = $updated_values ? array_merge($values_stored_in_database, $updated_values) : $defaults;
    variable_set('flat_deposit_general', $values);


    // warn if directory does not exist
    $checks = ['external', 'metadata', 'freeze'];
    $paths = variable_get('flat_deposit_general');
    foreach ($checks as $check) {
        if (!file_exists($paths[$check])) {
            drupal_set_message(t('%check directory (%dir) as specified in flat_deposit module settings does not exist', array('%check'=> $check, '%dir'=> $paths[$check])), 'error');
        }
    }

    return TRUE;
}

function flat_deposit_set_cmdi_profiles($updated_values = null)
{
    $defaults = array(
        'collection_profiles' => 'clarin.eu:cr1:p_1475136016239,clarin.eu:cr1:p_1407745712064',
        'bundle_profiles' => 'clarin.eu:cr1:p_1475136016242,clarin.eu:cr1:p_1407745712035',
    );

    //clean up stored values
    $values_stored_in_database = variable_get('flat_deposit_cmdi_profiles', array());

    //already installed
    if (!empty($values_stored_in_database[0])) {
        variable_del('flat_deposit_cmdi_profiles');
    }

    // set or update values
    $values = $updated_values ? array_merge($values_stored_in_database, $updated_values) : $defaults;
    variable_set('flat_deposit_cmdi_profiles', $values);

    return TRUE;
}

function flat_deposit_set_fits($updated_values = null)
{
    $defaults = array(
        'url' => 'http://127.0.0.1/fits',
        'port' => 8080,
    );

    //clean up stored values
    $values_stored_in_database = variable_get('flat_deposit_fits', array());

    //already installed
    if (!empty($values_stored_in_database[0])) {
        variable_del('flat_deposit_fits');
    }

    // set or update values
    $values = $updated_values ? array_merge($values_stored_in_database, $updated_values) : $defaults;
    variable_set('flat_deposit_fits', $values);

    return TRUE;
}


function flat_deposit_set_ingest_service($updated_values = null)
{
    $defaults = array(
        # host name of http server
        'host_name' => 'localhost',
        # host name of http server
        'host_scheme' => 'http',
        # java home
        'java_home' => '/usr/lib/jvm/java-8-openjdk-amd64',
        # the bagit executable
        'bag_exe' => '/usr/local/bin/bagit-4.9.0/bin/bag',
        #backend directory where bags will be placed
        'bag_dir'=> "/app/flat/deposit/bags/",
        # Should errors be logged
        # directory sword uses to temporarily save bags
        'sword_tmp_dir' => '/app/flat/deposit/sword/tmp/',
        'log_errors' => TRUE,
        # directory to log errors)
        'error_log_file' => '/app/flat/backend/Ingest_service_log/',
        # Should everything be logged
        'log_all' => TRUE,
        # Amount of files allowed to be in one bundle
        'max_ingest_files' => 3,
        'max_file_size' => 300,
        'allowed_extensions' => "wav, mpg, mpeg, mp4, jpg, png, tiff, svg, eaf, pfsx, cha, tbt, TextGrid, txt, html, pdf, tbx, cut, typ, lng, set, xml, xsd, kml, img, hdr, eeg, seg, sav, spv, ehst, ehtp, mat, fig, IMA, ima, dcm, vhdr, vmkr, hfinf, pcl, sce, exp, Praat, praat, zip, gz, odt, ods, odp, csv",
    );

    //clean up stored values
    $values_stored_in_database = variable_get('flat_deposit_ingest_service', array());

    //already installed
    if (!empty($values_stored_in_database[0])) {
        variable_del('flat_deposit_ingest_service');
    }

    // set or update values
    $values = $updated_values ? array_merge($values_stored_in_database, $updated_values) : $defaults;
    variable_set('flat_deposit_ingest_service', $values);

    // warn if directory does not exist
    $checks = ['bag_exe','bag_dir','error_log_file','sword_tmp_dir'];
    $paths = variable_get('flat_deposit_ingest_service');
    foreach ($checks as $check) {
        if (!file_exists($paths[$check])) {
            drupal_set_message(t('%check directory (%dir) as specified in flat_deposit module settings does not exist', array('%check'=> $check, '%dir'=> $paths[$check])), 'error');
        }
    }

    return TRUE;
}


function flat_deposit_set_sword($updated_values = null)
{
    $defaults = array(
        'url' => 'http://localhost/flat/easy-deposit/',
        'port' => 80,
        'user' => 'flat',
        'password' => 'sword',

    );

    //clean up stored values
    $values_stored_in_database = variable_get('flat_deposit_sword', array());

    //already installed
    if (!empty($values_stored_in_database[0])) {
        variable_del('flat_deposit_sword');
    }

    // set or update values
    $values = $updated_values ? array_merge($values_stored_in_database, $updated_values) : $defaults;
    variable_set('flat_deposit_sword', $values);

    return TRUE;
}


function flat_deposit_set_doorkeeper($updated_values = null)
{
    $defaults =  array(
        'url' => "http://localhost/flat/doorkeeper/",
        'port' => 8080
    );

    //clean up stored values
    $values_stored_in_database = variable_get('flat_deposit_doorkeeper', array());

    //already installed
    if (!empty($values_stored_in_database[0])) {
        variable_del('flat_deposit_doorkeeper');
    }

    // set or update values
    $values = $updated_values ? array_merge($values_stored_in_database, $updated_values) : $defaults;
    variable_set('flat_deposit_doorkeeper', $values);

    return TRUE;
}


function flat_deposit_set_fedora($updated_values = null)
{
    $defaults =  array(
        'host_name' => 'localhost',
        'host_ip' => '127.0.0.1',
        'port' => 8443,
        'scheme' => 'https',
        'user' => 'fedoraAdmin',
        'password' => 'fedora',
        'context' => 'fedora',
    );

    //clean up stored values
    $values_stored_in_database = variable_get('flat_deposit_fedora', array());

    //already installed
    if (!empty($values_stored_in_database[0])) {
        variable_del('flat_deposit_fedora');
    }

    // set or update values
    $values = $updated_values ? array_merge($values_stored_in_database, $updated_values) : $defaults;
    variable_set('flat_deposit_fedora', $values);

    return TRUE;
}


function flat_deposit_set_solr($updated_values = null)
{
    $defaults =  array(
        'schema' => 'https',
        'host_name' => 'lux125.mpi.nl',
        'port' => 8080,
        'path' => 'solr',
        'core' => 'collection1',
    );

    //clean up stored values
    $values_stored_in_database = variable_get('flat_deposit_solr', array());

    //already installed
    if (!empty($values_stored_in_database[0])) {
        variable_del('flat_deposit_solr');
    }

    // set or update values
    $values = $updated_values ? array_merge($values_stored_in_database, $updated_values) : $defaults;
    variable_set('flat_deposit_solr', $values);

    return TRUE;
}


function flat_deposit_set_anonymous_user_permissions()
{

// Define default anonymous user role permissions
    $anonymous_user_permissions = array(
        'access content' => FALSE,

    );

    // Grant permissions to anonymous users
    user_role_change_permissions(DRUPAL_ANONYMOUS_RID, $anonymous_user_permissions);
}

function flat_deposit_set_authenticated_user_permissions()
{

// Define authenticated user role permissions
    $authenticated_user_permissions = array(
        'access content' => TRUE,

    );

    // Grant permissions to authenticated user
    user_role_change_permissions(DRUPAL_AUTHENTICATED_RID, $authenticated_user_permissions);
}

function flat_deposit_set_custom_roles()
{
    $roles = array('manager', 'depositor');
    foreach ($roles as $role) {
        $role_exists = user_role_load_by_name($role);

        if (!$role_exists) {
            // Create manager and depositor custom roles
            $roleObject = new stdClass();
            $roleObject->name = $role;
            user_role_save($roleObject);
        }
    }

    // Manager role
    $manager_role = user_role_load_by_name('manager');
    $manager_rid = $manager_role->rid;

    // Define 'manager' role permissions
    $manager_permissions = array(
        'validate bundles' => TRUE,
        'create flat_bundle content' => TRUE,
        'edit own flat_bundle content' => TRUE,
        'delete own flat_bundle content' => TRUE,
        'use deposit module' => TRUE,
        'edit any flat_bundle content' => TRUE,
        'delete any flat_bundle content' => TRUE,
        'ingest bundles' => TRUE,
        'view collection' => TRUE,
        'admin collection' => TRUE,
    );

    // Grant permissions to the 'manager' role
    user_role_change_permissions($manager_rid, $manager_permissions);


    // Depositor role
    $depositor_role = user_role_load_by_name('depositor');
    $depositor_rid = $depositor_role->rid;

    // Define 'depositor' role permissions
    $depositor_permissions = array(
        'validate bundles' => TRUE,
        'create flat_bundle content' => TRUE,
        'edit own flat_bundle content' => TRUE,
        'delete own flat_bundle content' => TRUE,
        'use deposit module' => TRUE,
        'edit any flat_bundle content' => FALSE,
        'delete any flat_bundle content' => FALSE,
        'ingest bundles' => TRUE,
        'view collection' => TRUE,
        'admin collection' => TRUE,
    );

    // Grant permissions to 'depositor' role
    user_role_change_permissions($depositor_rid, $depositor_permissions);
}


function flat_deposit_set_content_access()
{
    $admin = user_role_load_by_name('administrator');
    $manager = user_role_load_by_name('manager');

    $permissions =  array(
        'view_own' => array(
                0 => DRUPAL_AUTHENTICATED_RID,
                1 => (int)$admin->rid,
            ),
        'view' =>
            array(
                0 => (int)$admin->rid,
                1 => $manager->rid,
            ),
        );

    variable_set('content_access_flat_bundle', $permissions);
}
