<?php

/*
DEFAULT SETTINGS OF THE FLAT_DEPOSIT MODULE AND FUNCTIONS TO SET THESE
*/

function flat_deposit_set_paths($updated_values=NULL)
{

    $defaults =  array(
        #directory for locally stored data
        'local' => '/app/flat/deposit/local/',
        #subdirectory within $local/$department tree where archive data can be found
        'local_subfolder' => '/workspaces/archive_deposit',
        #directory for uploading external data
        'external' => '/var/www/html/flat/sites/default/files/private/flat_deposit/data',
        #directory for storing data
        'metadata' => '/var/www/html/flat/sites/default/files/private/flat_deposit/metadata',
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
        'alternate_subdir' => '/files'
    );

    //remove already installed variables
    $values_stored_in_database = variable_get('flat_deposit_paths', array());

    if (!empty($values_stored_in_database[0])) {variable_del('flat_deposit_paths'); }


    // set or update values
    $values = $updated_values ? array_merge($values_stored_in_database, $updated_values) : $defaults;
    variable_set('flat_deposit_paths', $values );


    // warn if directory does not exist
    $checks = ['local','external', 'metadata', 'freeze','bag', 'sword_tmp_dir', 'alternate_dir' ];
    $paths = variable_get('flat_deposit_paths');
    foreach ($checks as $check){
        if (!file_exists($paths[$check])){
            drupal_set_message(t('%check directory (%dir) as specified in flat_deposit module settings does not exist', array('%check'=> $check, '%dir'=> $paths[$check])), 'error');
        }
    }

    return TRUE;
}

function flat_deposit_set_names($updated_values=NULL)
{
    $defaults =   array(
        'apache_user' => 'www-data'
    );

    $values_stored_in_database = variable_get('flat_deposit_names', array());

    //already installed
    if (!empty($values_stored_in_database[0])) {variable_del('flat_deposit_names'); }

    // set or update values
    $values = $updated_values ? array_merge($values_stored_in_database, $updated_values) : $defaults;
    variable_set('flat_deposit_names', $values );


    return TRUE;
}

function flat_deposit_set_fits($updated_values=NULL)
{
    $defaults = array(
        'url' => 'http://127.0.0.1/fits',
        'port' => 8080,
    );

    //clean up stored values
    $values_stored_in_database = variable_get('flat_deposit_fits', array());

    //already installed
    if (!empty($values_stored_in_database[0])) {variable_del('flat_deposit_fits'); }

    // set or update values
    $values = $updated_values ? array_merge($values_stored_in_database, $updated_values) : $defaults;
    variable_set('flat_deposit_fits', $values );

    return TRUE;

}


function flat_deposit_set_ingest_service($updated_values=NULL)
{
    $defaults = array(
        # host name of http server
        'host_name' => 'localhost',
        # host name of http server
        'host_scheme' => 'http',
        # the bagit executable
        'bag_exe' => '/usr/local/bin/bagit-4.9.0/bin/bag',
        # Should errors be logged
        'log_errors' => TRUE,
        # directory to log errors)
        'error_log_file' => '/app/flat/backend/Ingest_service_log/',
        # Should everything be logged
        'log_all' => TRUE,
        # directory to log errors)
        'log_file_dir' => '/app/flat/backend/',
    );

    //clean up stored values
    $values_stored_in_database = variable_get('flat_deposit_ingest_service', array());

    //already installed
    if (!empty($values_stored_in_database[0])) {variable_del('flat_deposit_ingest_service'); }

    // set or update values
    $values = $updated_values ? array_merge($values_stored_in_database, $updated_values) : $defaults;
    variable_set('flat_deposit_ingest_service', $values );
    return TRUE;
}





function flat_deposit_set_sword($updated_values=NULL)
{
    $defaults = array(
        'url' => 'https://localhost/easy-deposit/',
        'port' => 8443,
        'user' => 'flat',
        'password' => 'sword',

    );

    //clean up stored values
    $values_stored_in_database = variable_get('flat_deposit_sword', array());

    //already installed
    if (!empty($values_stored_in_database[0])) {variable_del('flat_deposit_sword'); }

    // set or update values
    $values = $updated_values ? array_merge($values_stored_in_database, $updated_values) : $defaults;
    variable_set('flat_deposit_sword', $values );

    return TRUE;

}


function flat_deposit_set_doorkeeper($updated_values=NULL)
{
    $defaults =  array(
        'url' => "http://localhost/flat/doorkeeper/",
        'port' => 8080
    );

    //clean up stored values
    $values_stored_in_database = variable_get('flat_deposit_doorkeeper', array());

//already installed
    if (!empty($values_stored_in_database[0])) {variable_del('flat_deposit_doorkeeper'); }

    // set or update values
    $values = $updated_values ? array_merge($values_stored_in_database, $updated_values) : $defaults;
    variable_set('flat_deposit_doorkeeper', $values );

    return TRUE;
}


function flat_deposit_set_fedora($updated_values=NULL) {

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
    if (!empty($values_stored_in_database[0])) {variable_del('flat_deposit_fedora'); }

    // set or update values
    $values = $updated_values ? array_merge($values_stored_in_database, $updated_values) : $defaults;
    variable_set('flat_deposit_fedora', $values );

    return TRUE;
}



function flat_deposit_set_solr($updated_values=NULL)
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
    if (!empty($values_stored_in_database[0])) {variable_del('flat_deposit_solr'); }

    // set or update values
    $values = $updated_values ? array_merge($values_stored_in_database, $updated_values) : $defaults;
    variable_set('flat_deposit_solr', $values );


    return TRUE;
}



function flat_deposit_set_anonymous_user_permissions(){

// Define authenticated user role permissions
    $anonymous_user_permissions = array(
        'access content' => FALSE,

    );

    // Grant permissions to our 'corpman' role
    user_role_change_permissions(DRUPAL_ANONYMOUS_RID, $anonymous_user_permissions);

}

function flat_deposit_set_authenticated_user_permissions(){

// Define authenticated user role permissions
    $authenticated_user_permissions = array(
        'validate bundles' => TRUE,
        'create flat_bundle content' => TRUE,
        'edit own flat_bundle content' => TRUE,
        'delete own flat_bundle content' => TRUE,
        'use deposit module' => TRUE,
        'access content' => TRUE,

    );

    // Grant permissions to our 'corpman' role
    user_role_change_permissions(DRUPAL_AUTHENTICATED_RID, $authenticated_user_permissions);

}




function flat_deposit_set_custom_roles(){
    $corpman = user_role_load_by_name('corpman');

    if (!$corpman){
        // Create corpus manager permission role
        $role = new stdClass();
        $role->name = 'corpman';
        user_role_save($role);

    }
    // Role to grant the permissions to
    $corpman_role = user_role_load_by_name('corpman');
    $corpman_rid = $corpman_role->rid;

    // Define our 'corpman' role permissions
    $corpman_permissions = array(
        'edit any flat_bundle content' => TRUE,
        'delete any flat_bundle content' => TRUE,
        'ingest bundles' => TRUE,
        'view collection' => TRUE,
        'admin collection' => TRUE,
    );

    // Grant permissions to our 'corpman' role
    user_role_change_permissions($corpman_rid, $corpman_permissions);


    $certified_user = user_role_load_by_name('certified user');

    if (!$certified_user){

        // Create corpus manager permission role
        $role = new stdClass();
        $role->name = 'certified user';
        user_role_save($role);

    }

    // Role to grant the permissions to
    $certified_user_role = user_role_load_by_name('certified user');
    $certified_user_rid = $certified_user_role->rid;

    // Define our 'corpman' role permissions
    $certified_user_permissions = array(
        'edit any flat_bundle content' => FALSE,
        'delete any flat_bundle content' => FALSE,
        'ingest bundles' => TRUE,
        'view collection' => TRUE,
        'admin collection' => TRUE,
    );

    // Grant permissions to our 'certified user' role
    user_role_change_permissions($certified_user_rid, $certified_user_permissions);


}


function flat_deposit_set_content_access(){


    $admin = user_role_load_by_name('administrator');
    $corpman = user_role_load_by_name('corpman');


    $permissions =  array(
        'view_own' => array(
                0 => DRUPAL_AUTHENTICATED_RID,
                1 => (int)$admin->rid,
            ),
        'view' =>
            array(
                0 => (int)$admin->rid,
                1 => $corpman->rid,
            ),
        );


    variable_set('content_access_flat_bundle', $permissions);

    }