<?php


function flat_deposit_admin_form($form, &$form_state)
{

    $form = array();

    $form['overview'] = array(
        '#markup' => t('Settings for deposit UI'),
        '#prefix' => '<p>',
        '#suffix' => '</p>',
    );

    // PATHS
    $form['paths'] = array(
        '#type' => 'fieldset',
        '#title' => t('Paths'),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
        '#tree' => TRUE,
    );

    $paths = variable_get('flat_deposit_paths');

    $form['paths']['freeze'] = array (
        '#type' => 'textfield',
        '#title' => t('Freeze folder'),
        '#description' => t('Directory where user bundles ready to be validated will be moved to'),
        '#default_value' => $paths ['freeze'],
        '#required' => TRUE,
    );

    $form['paths']['bag'] = array (
        '#type' => 'textfield',
        '#title' => t('Bag folder'),
        '#description' => t('Backend directory where bags will be placed'),
        '#default_value' => $paths ['bag'],
        '#required' => TRUE,
    );

    $form['paths']['sword_tmp_dir'] = array (
        '#type' => 'textfield',
        '#title' => t('Sword temporary folder'),
        '#description' => t('Directory sword uses to temporarily save bags'),
        '#default_value' => $paths ['sword_tmp_dir'],
        '#required' => TRUE,
    );

    $form['paths']['alternate_dir'] = array (
        '#type' => 'textfield',
        '#title' => t('Alternative root user data directory'),
        '#description' => t('Root of the alternative directory where ingest service can search for archiving files. This 
            root need to contain user data folders named just as the LDAP login name'),
        '#default_value' => $paths ['alternate_dir'],
        '#required' => TRUE,
    );

    $form['paths']['alternate_subdir'] = array (
        '#type' => 'textfield',
        '#title' => t('Alternative sub directory'),
        '#description' => t('subdirectory following user login name (e.g. in owncloud this is \'/files\')'),
        '#default_value' => $paths ['alternate_subdir'],
    );

    // NAMES
    $form['names'] = array(
        '#type' => 'fieldset',
        '#title' => t('Names'),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
        '#tree' => TRUE,
    );

    $names = variable_get('flat_deposit_names');

    $form['names']['apache_user'] = array (
        '#type' => 'textfield',
        '#title' => t('Apache User'),
        '#description' => t('Name of the apache user (e.g. www-data, www-)'),
        '#default_value' => $names['apache_user'],
        '#required' => TRUE,
    );

    // METADATA
    $form['metadata'] = array(
        '#type' => 'fieldset',
        '#title' => t('Metadata'),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
        '#tree' => TRUE,
    );

    $metadata = variable_get('flat_deposit_metadata');

    $form['metadata']['site'] = array (
        '#type' => 'textfield',
        '#title' => t('Archive site'),
        '#default_value' => $metadata['site'],
        '#required' => TRUE,
    );

    $form['metadata']['type'] = array (
        '#type' => 'textfield',
        '#title' => t('Metadata file format'),
        '#default_value' => $metadata['type'],
        '#required' => TRUE,
    );

    $form['metadata']['prefix'] = array (
        '#type' => 'textfield',
        '#title' => t('Namespace for handles'),
        '#default_value' => $metadata['prefix'],
        '#required' => TRUE,
    );

    $form['metadata']['MdProfile'] = array (
        '#type' => 'textfield',
        '#title' => t('Clarin metadata profile'),
        '#default_value' => $metadata['MdProfile'],
        '#required' => TRUE,
    );



    // INGEST SERVICE
    $form['ingest_service'] = array(
        '#type' => 'fieldset',
        '#title' => t('Ingest service'),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
        '#tree' => TRUE
    );

    $service = variable_get('flat_deposit_ingest_service');

    $form['ingest_service']['host_name'] = array (
        '#type' => 'textfield',
        '#title' => t('Host name'),
        '#description' => t('Host name or IP of the php server'),
        '#default_value' => $service['host_name'],
        '#required' => TRUE,
    );

    $form['ingest_service']['bag_exe'] = array (
        '#type' => 'textfield',
        '#title' => t('Bagit executable'),
        '#description' => t('Specific path to bagit executable'),
        '#default_value' => $service['bag_exe'],
        '#required' => TRUE,
    );

    $form['ingest_service']['log_errors'] = array (
        '#type' => 'checkbox',
        '#title' => t('Log backend service errors'),
        '#description' => t('Should errors during backend ingest be written to disk?'),
        '#default_value' => $service['log_errors'],
        '#required' => FALSE,
    );

    $form['ingest_service']['error_log_file'] = array (
        '#type' => 'textfield',
        '#title' => t('Path to error log file'),
        '#default_value' => $service['error_log_file'],
        '#required' => TRUE,
    );

    $form['ingest_service']['log_all'] = array (
        '#type' => 'checkbox',
        '#title' => t('Log all backend actions'),
        '#description' => t('Should all backend service activities be logged?'),
        '#default_value' => $service['log_all'],
        '#required' => FALSE,
    );

    $form['ingest_service']['log_file_dir'] = array (
        '#type' => 'textfield',
        '#title' => t('Path to log file storage'),
        '#default_value' => $service['log_file_dir'],
        '#required' => TRUE,
    );


    // SWORD
    $form['sword'] = array(
        '#type' => 'fieldset',
        '#title' => t('Sword configuration'),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
        '#tree' => TRUE
    );

    $sword = variable_get('flat_deposit_sword');

    $form['sword']['url'] = array (
        '#type' => 'textfield',
        '#title' => t('URL of the easy-deposit service'),
        '#default_value' => $sword['url'],
        '#required' => TRUE,
    );

    $form['sword']['port'] = array (
        '#type' => 'textfield',
        '#title' => t('Port used by easy-deposit service'),
        '#default_value' => $sword['port'],
        '#required' => TRUE,
    );

    $form['sword']['user'] = array (
        '#type' => 'textfield',
        '#title' => t('User ID used to connect to easy-deposit service'),
        '#default_value' => $sword['user'],
        '#required' => TRUE,
    );

    $form['sword']['password'] = array (
        '#type' => 'password',
        '#title' => t('Password corresponding to this user'),
    );

    // DOORKEEPER
    $form['doorkeeper'] = array(
        '#type' => 'fieldset',
        '#title' => t('DoorKeeper configuration'),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
        '#tree' => TRUE
    );

    $doorkeeper = variable_get('flat_deposit_doorkeeper');

    $form['doorkeeper']['url'] = array (
        '#type' => 'textfield',
        '#title' => t('URL of the doorkeeper servlet'),
        '#default_value' => $doorkeeper['url'],
        '#required' => TRUE,
    );

    $form['doorkeeper']['port'] = array (
        '#type' => 'textfield',
        '#title' => t('Port used by DoorKeeper servlet'),
        '#default_value' => $doorkeeper['port'],
        '#required' => TRUE,
    );

    // FEDORA
    $form['fedora'] = array(
        '#type' => 'fieldset',
        '#title' => t('Fedora configuration'),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
        '#tree' => TRUE
    );

    $fedora = variable_get('flat_deposit_fedora');

    $form['fedora']['host_name'] = array (
        '#type' => 'textfield',
        '#title' => t('Host name of fedora commons server'),
        '#default_value' => $fedora['host_name'],
        '#required' => TRUE,
    );

    $form['fedora']['host_ip'] = array (
        '#type' => 'textfield',
        '#title' => t('IP address of fedora commons server'),
        '#default_value' => $fedora['host_ip'],
        '#required' => TRUE,
    );

    $form['fedora']['port'] = array (
        '#type' => 'textfield',
        '#title' => t('Port used by fedora commons server'),
        '#default_value' => $fedora['port'],
        '#required' => TRUE,
    );

    $form['fedora']['scheme'] = array (
        '#type' => 'textfield',
        '#title' => t('Scheme used by fedora commons server'),
        '#descriptions' => t('http or https'),
        '#default_value' => $fedora['scheme'],
        '#required' => TRUE,
    );

    $form['fedora']['user'] = array (
        '#type' => 'textfield',
        '#title' => t('User ID used to connect to fedora commons server'),
        '#default_value' => $fedora['user'],
        '#required' => TRUE,
    );

    $form['fedora']['password'] = array (
        '#type' => 'password',
        '#title' => t('Password corresponding to this user'),
    );

    $form['fedora']['context'] = array (
        '#type' => 'textfield',
        '#title' => t('Context'),
        '#description' => t('Variable specifying the context of the connection'),
        '#default_value' => $fedora['context'],
        '#required' => TRUE,
    );

    // SOLR
    $form['solr'] = array(
        '#type' => 'fieldset',
        '#title' => t('solr configuration'),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
        '#tree' => TRUE
    );

    $solr = variable_get('flat_deposit_solr');

    $form['solr']['host_name'] = array (
        '#type' => 'textfield',
        '#title' => t('Host name of SOLR'),
        '#default_value' => $solr['host_name'],
        '#required' => TRUE,
    );

    $form['solr']['port'] = array (
        '#type' => 'textfield',
        '#title' => t('Port used by solr'),
        '#default_value' => $solr['port'],
        '#required' => TRUE,
    );

    $form['solr']['schema'] = array (
        '#type' => 'textfield',
        '#title' => t('scheme to connect to solr'),
        '#descriptions' => t('http or https'),
        '#default_value' => $solr['schema'],
        '#required' => TRUE,
    );

    $form['solr']['path'] = array (
        '#type' => 'textfield',
        '#title' => t('Solr path'),
        '#default_value' => $solr['path'],
        '#required' => TRUE,
    );

    $form['solr']['core'] = array (
        '#type' => 'textfield',
        '#title' => t('Solr core'),
        '#default_value' => $solr['core'],
        '#required' => TRUE,
    );

    // BUTTONS
    $form['buttons']['submit'] = array (
        '#type' => 'submit',
        '#value' => t('Save'),
        '#submit' => array(
            'keep_default_password',
            'flat_deposit_admin_form_save_submit',
            )
    );

    $form['buttons']['restore'] = array (
        '#type' => 'submit',
        '#value' => t('Reset to defaults'),
        '#submit' => array('flat_deposit_admin_form_reset_submit'),
        '#limit_validation_errors' => array(),
    );

    $form['buttons']['import'] = array (
        '#type' => 'submit',
        '#value' => t('Import settings'),
        '#submit' => array('flat_deposit_admin_form_import_submit'),
        '#validate' => array('flat_deposit_admin_form_import_validate'),
    );

    $form['buttons']['export'] = array (
        '#type' => 'submit',
        '#value' => t('Export settings'),
        '#submit' => array('flat_deposit_admin_form_export_submit'),
    );

    $form['file']['import'] = array (
        '#name' => 'File_selector',
        '#type' => 'file',
        '#title' => t('Choose a file'),
        '#title_display' => 'invisible',
        '#size' => 22,
        '#theme_wrappers' => array(),
        '#weight' => 999,
    );

    return $form;
}

/**
 * keeps the default password in case no
 */
function keep_default_password($form, &$form_state) {

    if (empty($form_state['values']['sword']['password'])) {
        $form_state['values']['sword']['password'] = variable_get('flat_deposit_sword')['password'];
    }

    if (empty($form_state['values']['fedora']['password'])) {
        $form_state['values']['fedora']['password'] = variable_get('flat_deposit_fedora')['password'];
    }

}

function flat_deposit_admin_form_save_submit ($form, &$form_state)
{

    module_load_include('inc', 'flat_deposit', 'inc/config');

    #variable_set('flat_deposit_paths', $form_state['values']['paths']);
    #variable_set('flat_deposit_names', $form_state['values']['names']);
    #variable_set('flat_deposit_metadata', $form_state['values']['metadata']);
    #variable_set('flat_deposit_ingest_service', $form_state['values']['ingest_service']);
    #variable_set('flat_deposit_sword', $form_state['values']['sword']);
    #variable_set('flat_deposit_doorkeeper', $form_state['values']['doorkeeper']);
    #variable_set('flat_deposit_fedora', $form_state['values']['fedora']);
    #variable_set('flat_deposit_solr', $form_state['values']['solr']);

    flat_deposit_set_paths($form_state['values']['paths']);
    flat_deposit_set_names($form_state['values']['names']);
    flat_deposit_set_metadata($form_state['values']['metadata']);
    flat_deposit_set_ingest_service($form_state['values']['ingest_service']);
    flat_deposit_set_sword($form_state['values']['sword']);
    flat_deposit_set_doorkeeper($form_state['values']['doorkeeper']);
    flat_deposit_set_fedora($form_state['values']['fedora']);
    flat_deposit_set_solr($form_state['values']['solr']);
    drupal_set_message(t('Changed values have been saved'));

}



function flat_deposit_admin_form_reset_submit ($form, &$form_state){
    module_load_include('inc', 'flat_deposit', 'inc/config');

    flat_deposit_set_paths();
    flat_deposit_set_names();
    flat_deposit_set_metadata();
    flat_deposit_set_ingest_service();
    flat_deposit_set_sword();
    flat_deposit_set_doorkeeper();
    flat_deposit_set_fedora();
    flat_deposit_set_solr();

    drupal_set_message(t('All settings have been reset to default'));
}


function flat_deposit_admin_form_export_submit()
{

    $config = array(
        'paths' =>  variable_get('flat_deposit_paths'),
        'names' => variable_get('flat_deposit_names'),
        'metadata' => variable_get('flat_deposit_metadata'),
        'ingest_service' => variable_get('flat_deposit_ingest_service'),
        'sword' => variable_get('flat_deposit_sword'),
        'doorkeeper' => variable_get('flat_deposit_doorkeeper'),
        'fedora' => variable_get('flat_deposit_fedora'),
        'solr' => variable_get('flat_deposit_solr'),
    );


    $URI = "temporary://flat_deposit_settings.php";

    file_unmanaged_save_data(serialize($config), $URI, FILE_EXISTS_REPLACE);

    $header =  array('Content-type' => 'application/x-php',
        'Content-Disposition' => 'attachment; filename=\'flat_deposit_settings.php\'');

    file_transfer($URI, $header);

}


function flat_deposit_admin_form_import_validate ($form, &$form_state)
{

    // Check if file is specified
    if (empty($_FILES['File_selector']['name'])) {
        form_set_error('import', t('File needs to be specified for import'));

        return $form;
    }

    // Check mime type of file
    $tmp = $_FILES['File_selector']['tmp_name'];

    $type = mime_content_type(drupal_realpath($tmp));

    if ($type != 'text/plain') {

        form_set_error('import', t('File needs to be a plain text file '));
    }


    // Check if file can be read in
    $content = file_get_contents($tmp);

    if ($content === FALSE) {

        form_set_error('import', t('File can not be processed'));
        return $form;
    }


    // Check allowed characters
    if (preg_match('/[^a-zA-Z0-9_.;:"\s\-\{\}\/]/', $content) !== 0) {

        form_set_error('import', t('File contains forbidden strings. Only allowed strings are numbers, letters, underscores (semi-)colons curly brackets.'));
        return $form;
    }

    // Check if string can be unserialized
    $imported_values = @unserialize($content);

    if ($imported_values === false) {

        form_set_error('import', t('Unable to unserialize the string'));
        return $form;

    }

    // validate that unserialization of imported string results in arrays corresponding to the drupal 'flat_deposit'-variables
    $checks = array('paths', 'names', 'metadata', 'ingest_service', 'sword', 'doorkeeper', 'fedora', 'solr');

    foreach ($checks as $check) {
        if (!empty(array_diff_key($imported_values[$check], $form_state['values'][$check]))) {

            form_set_error('imported variable keys do not match with flat_deposit variable keys');

        };
    }


    $form_state['values']['imported_values'] = $imported_values;

    return $form;
}



function flat_deposit_admin_form_import_submit ($form, &$form_state){


    variable_set('flat_deposit_paths', $form_state['values']['imported_values']['paths']);
    variable_set('flat_deposit_names', $form_state['values']['imported_values']['names']);
    variable_set('flat_deposit_metadata', $form_state['values']['imported_values']['metadata']);
    variable_set('flat_deposit_ingest_service', $form_state['values']['imported_values']['ingest_service']);
    variable_set('flat_deposit_sword', $form_state['values']['imported_values']['sword']);
    variable_set('flat_deposit_doorkeeper', $form_state['values']['imported_values']['doorkeeper']);
    variable_set('flat_deposit_fedora', $form_state['values']['imported_values']['fedora']);
    variable_set('flat_deposit_solr', $form_state['values']['imported_values']['solr']);



    drupal_set_message(t('Setting import completed'));


}