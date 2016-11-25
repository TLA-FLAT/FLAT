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
            'flat_deposit_admin_form_submit',

            )
    );

    $form['buttons']['restore'] = array (
        '#type' => 'submit',
        '#value' => t('Reset to defaults'),
        '#submit' => array('flat_deposit_admin_form_reset_submit'),
        '#limit_validation_errors' => array(),
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

function flat_deposit_admin_form_submit ($form, &$form_state)
{
    variable_set('flat_deposit_paths', $form_state['values']['paths']);
    variable_set('flat_deposit_names', $form_state['values']['names']);
    variable_set('flat_deposit_metadata', $form_state['values']['metadata']);
    variable_set('flat_deposit_ingest_service', $form_state['values']['ingest_service']);
    variable_set('flat_deposit_sword', $form_state['values']['sword']);
    variable_set('flat_deposit_doorkeeper', $form_state['values']['doorkeeper']);
    variable_set('flat_deposit_fedora', $form_state['values']['fedora']);
    variable_set('flat_deposit_solr', $form_state['values']['solr']);
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
