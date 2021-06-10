<?php


function flat_deposit_admin_form($form, &$form_state)
{

    $form = array();

    $form['overview'] = array(
        '#markup' => t('Settings for the FLAT Deposit module'),
        '#prefix' => '<p>',
        '#suffix' => '</p>',
    );

    // GENERAL
    $form['general'] = array(
        '#type' => 'fieldset',
        '#title' => t('General settings'),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
        '#tree' => TRUE,
    );

    $general = variable_get('flat_deposit_general');

    $form['general']['namespace'] = array(
        '#type' => 'textfield',
        '#title' => t('Fedora namespace'),
        '#description' => t('Fedora namespace to be used for ingests'),
        '#default_value' => $general['namespace'],
        '#required' => true,
    );

    $form['general']['external'] = array (
        '#type' => 'textfield',
        '#title' => t('Upload folder for external data'),
        '#description' => t('Directory where uploaded data will be stored'),
        '#default_value' => $general ['external'],
        '#required' => TRUE,
    );

    $form['general']['metadata'] = array (
        '#type' => 'textfield',
        '#title' => t('Metadata folder'),
        '#description' => t('Directory for temporarily storing metadata'),
        '#default_value' => $general ['metadata'],
        '#required' => TRUE,
    );

    $form['general']['freeze'] = array (
        '#type' => 'textfield',
        '#title' => t('Freeze folder'),
        '#description' => t('Directory where user bundles will be placed upon validation'),
        '#default_value' => $general ['freeze'],
        '#required' => TRUE,
    );

    // CMDI profiles
    $form['cmdi_profiles'] = array(
        '#type' => 'fieldset',
        '#title' => t('CMDI profile settings'),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
        '#tree' => TRUE,
    );

    $profiles = variable_get('flat_deposit_cmdi_profiles');

    $form['cmdi_profiles']['collection_profile_ids'] = array (
        '#type' => 'textfield',
        '#title' => t('Collection CMDI profile IDs'),
        '#description' => t('CMDI profiles that should be treated as collections'),
        '#default_value' => $profiles['collection_profiles'],
        '#required' => TRUE,
    );

    $form['cmdi_profiles']['bundle_profile_ids'] = array (
        '#type' => 'textfield',
        '#title' => t('Bundle CMDI profile IDs'),
        '#description' => t('CMDI profiles that should be treated as bundles'),
        '#default_value' => $profiles['bundle_profiles'],
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
        '#description' => t('Host name or IP of the http/php server'),
        '#default_value' => $service['host_name'],
        '#required' => TRUE,
    );

    $form['ingest_service']['host_scheme'] = array (
        '#type' => 'textfield',
        '#title' => t('Host Scheme '),
        '#description' => t('HTTP or HTTPS'),
        '#default_value' => $service['host_scheme'],
        '#required' => TRUE,
    );

    $form['ingest_service']['java_home'] = array(
        '#type' => 'textfield',
        '#title' => t('Java home'),
        '#description' => t('Specific Java home path'),
        '#default_value' => $service['java_home'],
        '#required' => TRUE,
    );

    $form['ingest_service']['bag_exe'] = array (
        '#type' => 'textfield',
        '#title' => t('Bagit executable'),
        '#description' => t('Specific path to bagit executable'),
        '#default_value' => $service['bag_exe'],
        '#required' => TRUE,
    );

    $form['ingest_service']['bag_dir'] = array (
        '#type' => 'textfield',
        '#title' => t('Bag folder'),
        '#description' => t('Backend directory where bags will be placed'),
        '#default_value' => $service ['bag_dir'],
        '#required' => TRUE,
    );

    $form['ingest_service']['sword_tmp_dir'] = array (
        '#type' => 'textfield',
        '#title' => t('Sword temporary folder'),
        '#description' => t('Directory sword uses to temporarily save bags'),
        '#default_value' => $service ['sword_tmp_dir'],
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
        '#title' => t('Path to error log directory'),
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
    $form['ingest_service']['max_ingest_files'] = array (
        '#type' => 'textfield',
        '#title' => t('Maximal number of files per bundle'),
        '#description' => t('Validation parameter for bundles before ingest'),
        '#default_value' => $service['max_ingest_files'],
        '#required' => TRUE,
    );
    $form['ingest_service']['max_file_size'] = array (
        '#type' => 'textfield',
        '#title' => t('Maximum size per ingested file (GB)'),
        '#description' => t('Validation parameter for bundles before ingest'),
        '#default_value' => $service['max_file_size'],
        '#required' => TRUE,
    );

    $form['ingest_service']['allowed_extensions'] = array (
        '#type' => 'textarea',
        '#title' => t('Allowed file extensions (comma separated list)'),
        '#description' => t('Validation parameter for bundles before ingest'),
        '#default_value' => $service['allowed_extensions'],
        '#required' => TRUE,
    );

    // FITS
    $form['fits'] = array(
        '#type' => 'fieldset',
        '#title' => t('FITS'),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
        '#tree' => TRUE,
    );

    $fits = variable_get('flat_deposit_fits');

    $form['fits']['url'] = array (
        '#type' => 'textfield',
        '#title' => t('URL of FITS service'),
        '#default_value' => $fits['url'],
        '#required' => TRUE,
    );

    $form['fits']['port'] = array (
        '#type' => 'textfield',
        '#title' => t('Port of FITS service'),
        '#default_value' => $fits['port'],
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
        '#validate' => array('flat_deposit_admin_form_save_validate',),
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
function keep_default_password($form, &$form_state)
{
    if (empty($form_state['values']['sword']['password'])) {
        $form_state['values']['sword']['password'] = variable_get('flat_deposit_sword')['password'];
    }

    if (empty($form_state['values']['fedora']['password'])) {
        $form_state['values']['fedora']['password'] = variable_get('flat_deposit_fedora')['password'];
    }
}

function flat_deposit_admin_form_save_validate($form, &$form_state)
{
    $excluded = ['namespace']; #fill with values if general get extra values that are not paths to check
    foreach ($form_state['values']['general'] as $key => $path) {
        if (!is_numeric(array_search($key, $excluded))) {
            if (!file_exists($path)) {
                drupal_mkdir($path, NULL, TRUE);
            }

            if (!file_exists($path)) {
                form_set_error($key, 'Directory ('. $path .') does not exist and cannot be created');
            }
        }
    }
}


function flat_deposit_admin_form_save_submit($form, &$form_state)
{
    module_load_include('inc', 'flat_deposit', 'inc/config');

    flat_deposit_set_general($form_state['values']['general']);
    flat_deposit_set_cmdi_profiles($form_state['values']['cmdi_profiles']);
    flat_deposit_set_fits($form_state['values']['fits']);
    flat_deposit_set_ingest_service($form_state['values']['ingest_service']);
    flat_deposit_set_sword($form_state['values']['sword']);
    flat_deposit_set_doorkeeper($form_state['values']['doorkeeper']);
    flat_deposit_set_fedora($form_state['values']['fedora']);
    flat_deposit_set_solr($form_state['values']['solr']);
    drupal_set_message(t('Changed values have been saved'));
}



function flat_deposit_admin_form_reset_submit($form, &$form_state)
{
    module_load_include('inc', 'flat_deposit', 'inc/config');

    flat_deposit_set_general();
    flat_deposit_set_cmdi_profiles();
    flat_deposit_set_fits();
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
        'general' =>  variable_get('flat_deposit_general'),
        'cmdi_profiles' => variable_get('flat_deposit_cmdi_profiles'),
        'fits' => variable_get('flat_deposit_fits'),
        'ingest_service' => variable_get('flat_deposit_ingest_service'),
        'sword' => variable_get('flat_deposit_sword'),
        'doorkeeper' => variable_get('flat_deposit_doorkeeper'),
        'fedora' => variable_get('flat_deposit_fedora'),
        'solr' => variable_get('flat_deposit_solr'),
    );

    $URI = "temporary://flat_deposit_settings.json";

    file_unmanaged_save_data(json_encode($config), $URI, FILE_EXISTS_REPLACE);

    $header =  array('Content-type' => 'application/json',
        'Content-Disposition' => 'attachment; filename=flat_deposit_settings.json');

    file_transfer($URI, $header);
}


function flat_deposit_admin_form_import_validate($form, &$form_state)
{

    // Check if file is specified
    if (empty($_FILES['File_selector']['name'])) {
        form_set_error('import', t('File needs to be specified for import'));

        return $form;
    }

    // Check mime type of file
    $tmp = $_FILES['File_selector']['tmp_name'];


    /* check fails, apparently FITS gives it another mime type than application/json
    module_load_include('inc', 'flat_deposit', '/Helpers/CMDI/class.CmdiHandler');
    $type = CmdiHandler::fits_mimetype_check($tmp);

    if ($type != 'application/json') {

        form_set_error('import', t('Settings file needs to be a json file '));
    }
    */

    // Check if file can be read in
    $content = file_get_contents($tmp);

    if ($content === FALSE) {
        form_set_error('import', t('Settings file cannot be processed'));
        return $form;
    }


    // Check allowed characters
    if (preg_match('/[^a-zA-Z0-9_.,;:"\s\-\{\}\/\\\\]/', $content) !== 0) {

        form_set_error('import', t('Settings file contains forbidden characters. Only allowed characters are numbers, letters, underscores, hyphens, (semi-)colons, curly brackets, dot, comma, slash, backslash.'));
        return $form;
    }

    // Check if string can be unserialized
    $imported_values = json_decode($content, true);

    if ($imported_values === false) {
        form_set_error('import', t('Unable to decode the settings file'));
        return $form;
    }

    // validate that unserialization of imported string results in arrays corresponding to the drupal 'flat_deposit'-variables
    $checks = array('general', 'fits', 'ingest_service', 'sword', 'doorkeeper', 'fedora', 'solr');

    foreach ($checks as $check) {
        if (!empty(array_diff_key($imported_values[$check], $form_state['values'][$check]))) {

            form_set_error('imported variable keys do not match with flat_deposit variable keys');
        };
    }


    $form_state['values']['imported_values'] = $imported_values;

    return $form;
}


function flat_deposit_admin_form_import_submit($form, &$form_state)
{
    variable_set('flat_deposit_general', $form_state['values']['imported_values']['general']);
    variable_set('flat_deposit_cmdi_profiles', $form_state['values']['imported_values']['cmdi_profiles']);
    variable_set('flat_deposit_fits', $form_state['values']['imported_values']['fits']);
    variable_set('flat_deposit_ingest_service', $form_state['values']['imported_values']['ingest_service']);
    variable_set('flat_deposit_sword', $form_state['values']['imported_values']['sword']);
    variable_set('flat_deposit_doorkeeper', $form_state['values']['imported_values']['doorkeeper']);
    variable_set('flat_deposit_fedora', $form_state['values']['imported_values']['fedora']);
    variable_set('flat_deposit_solr', $form_state['values']['imported_values']['solr']);

    drupal_set_message(t('Setting import completed'));
}
