<?php


function flat_workspaces_set_variables($updated_values=NULL){

    $defaults =  array(
        'activated' => FALSE,
        'mount_folder' => '/app/flat/deposit/local/',
        'workspace_folder' => 'workspaces',
        'archive_folder' => 'archive_deposit',
        'department_mapping' => array(
            'lac' => 'Language and Cognition',
            'ladd' => 'Language Development',
            'lag' => 'Language and Genetics',
            'nbl' => 'Neurobiology of Language',
            'nvc' => 'Neurogenetics of Vocal Communication',
            'pol' => 'Psychology of Language',
            'tg' => 'Technical group',
        ),
        );

    //clean up stored values
    $values_stored_in_database = variable_get('flat_workspaces', array());

    //already installed
    if (!empty($values_stored_in_database[0])) {variable_del('flat_workspaces'); }

    // set or update values
    $values = $updated_values ? array_merge($values_stored_in_database, $updated_values) : $defaults;
    variable_set('flat_workspaces', $values );

    return TRUE;
}

