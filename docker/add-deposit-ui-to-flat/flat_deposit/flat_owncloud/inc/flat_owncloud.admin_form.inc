<?php
/**
 * Created by PhpStorm.
 * User: danrhe
 * Date: 02/11/16
 * Time: 17:15
 */




/**
 * Implements hook_form().
 */
function flat_owncloud_admin_form($form, &$form_state)
{
    $config = variable_get('flat_owncloud',array());

    $form = array();

    $form['overview'] = array(
        '#markup' => t('Administrators can configure here owncloud settings. It is required that owncloud runs on the same server as this drupal installation'),
        '#prefix' => '<p>',
        '#suffix' => '</p>',);

    $form['owncloud'] = array(
        '#type' => 'fieldset',
        '#title' => t('Owncloud settings'),
        '#tree' => TRUE,
        );


    $form['owncloud']['activated'] = array (
        '#title' => t('Use Owncloud'),
        '#description' => t('Allow incorporation of owncloud in module'),
        '#type' => 'checkbox',
        '#default_value' => $config['activated'],
        '#required' => FALSE);


    $form['owncloud']['admin_name'] = array (
        '#type' => 'textfield',
        '#title' => 'Owncloud admin name',
        '#description' => t('The owncloud admin user name'),
        '#default_value' => $config['admin_name'],
        '#required' => TRUE,);

    $form['owncloud']['admin_pass'] = array (
        '#type' => 'password',
        '#title' => 'Owncloud admin password',
        '#description' => t('The owncloud admin user password'),
        '#default_value' => $config['admin_pass'],
        '#required' => FALSE,);

    $form['owncloud']['schema'] = array (
        '#type' => 'textfield',
        '#title' => 'Host schema',
        '#description' => t('HTTP or HTTPS'),
        '#default_value' => $config['schema'],
        '#required' => TRUE,);

    $form['owncloud']['host'] = array (
        '#type' => 'textfield',
        '#title' => 'Host name',
        '#description' => t('IP address or hostname'),
        '#default_value' => $config['host'],
        '#required' => TRUE,);

    $form['owncloud']['root_dir'] = array (
        '#type' => 'textfield',
        '#title' => 'Owncloud root directory',
        '#description' => t('The owncloud installation path'),
        '#default_value' => $config['root_dir'],
        '#required' => TRUE,);

    $form['owncloud']['temp_dir'] = array (
        '#type' => 'textfield',
        '#title' => 'Owncloud temp directory',
        '#description' => t('Directory where owncloud can read/write temporary files (e.g. mount_user.json)'),
        '#default_value' => $config['temp_dir'],
        '#required' => TRUE,);

    $form['button']['submit'] = array (
        '#type' => 'submit',
        '#value' => t('Save'),
        '#submit' => array(
            'keep_default_password',
            'flat_owncloud_admin_form_submit',
        )
    );


    $form['button']['restore'] = array (
        '#type' => 'submit',
        '#value' => t('Reset to defaults'),
    );

    $form['button']['check'] = array (
        '#type' => 'submit',
        '#value' => t('Check owncloud connection'),);

    return $form;
}

/**
 * keeps the default password in case no
 */
function keep_default_password($form, &$form_state) {


    if (empty($form_state['owncloud']['admin_pass'])) {
        $form_state['values']['owncloud']['admin_pass'] = variable_get('flat_owncloud')['admin_pass'];
    }

}


function check_owncloud_status()
{
    $status = FALSE;

    $drupal_dir = getcwd();
    $config = variable_get('flat_owncloud');
    chdir($config['root_dir']);

    $cmd = 'php occ status --output=json';
    exec($cmd, $output, $return_val);

    if (!$return_val) {
        $formatted = (array)json_decode($output[0]);

        if ($formatted['installed']){

            chdir($drupal_dir);
            return $formatted;
        }

    }
    chdir($drupal_dir);
    return $status;
}

function check_owncloud_external_files_plugin($enable=FALSE)
{
    $enabled = FALSE;

    $drupal_dir = getcwd();
    $config = variable_get('flat_owncloud');
    chdir($config['root_dir']);

    $cmd = 'php occ app:list --output=json';
    exec ( $cmd , $output, $return_val);

    // Activate files_external plugin if status of plugin is disabled
    if (!$return_val AND $enable){
      $formatted = (array)json_decode($output[0]);
        if (array_key_exists('files_external',$formatted['disabled'])){

            $cmd = 'php occ app:enable files_external';
            exec ( $cmd , $output, $return_val);

        }
    }

    // Check if files_external plugin is enabled
    $cmd = 'php occ app:list --output=json';
    exec ( $cmd , $output, $return_val);

    // check status of files_external plugin
    if ($output){

        $formatted = (array)json_decode($output[0]);

        if (array_key_exists('files_external',$formatted['enabled'])){

            $enabled = TRUE;

        }
    }

    chdir($drupal_dir);
    return $enabled;
}


function flat_owncloud_admin_form_validate ($form, &$form_state)
{
    $button = $form_state['values']['op'];

    switch ($button) {
        case 'Save': {
            $status = check_owncloud_status();
            if (!$status) {
                form_set_error('activated', 'Owncloud status failed');
            }

            if ($form_state['values']['owncloud']['activated']){
                $activated = check_owncloud_external_files_plugin(TRUE);
                if (!$activated){
                    form_set_error('activated','Owncloud external plugin is not / could not be activated');
                }
            }

            break;
        }
        case 'Reset to defaults': {

            break;
        }
        case 'Check owncloud connection': {
            $status = check_owncloud_status();
            if (!$status) {
                form_set_error('activated', 'Owncloud status failed');
            } else {

                $form_state['values']['status'] = $status;
            }

            $activated = check_owncloud_external_files_plugin();
            if (!$activated){
                form_set_error('activated','Owncloud external plugin is not / could not be activated');
            }
            break;
        }

    }
    return $form;
}


function flat_owncloud_admin_form_submit ($form, &$form_state)
{
    $button = $form_state['values']['op'];

    switch ($button) {
        case 'Save': {
            module_load_include('inc', 'flat_owncloud', 'inc/config');

            flat_owncloud_set_owncloud($form_state['values']['owncloud']);


            drupal_set_message(t('Changed values have been saved'));

            break;
        }
        case 'Reset to defaults': {

            module_load_include('inc', 'flat_owncloud', 'inc/config');

            flat_owncloud_set_owncloud();
            drupal_set_message(t('Owncloud settings have been reset to default'));

            break;
        }
        case 'Check owncloud connection': {

            drupal_set_message(print_r($form_state['values']['status'],TRUE));
            drupal_set_message(t('Owncloud plugin is enabled'));
            try {

                module_load_include('inc', 'flat_owncloud', 'Helpers/owncloud_api');
                $oc = new OwnCloud();
                if (!$oc->noErrors){
                    throw new OwnCloudConnectionException($oc->error_message);
                }
                drupal_set_message(t('User account found on Owncloud server'));
                $oc->ListAllDrupalMounts(TRUE);

                // Remove existing mount
                $oc->GetUserMount(TRUE);
                if ($oc->archive_mount)
                $oc->UnmountUserMount(NULL, TRUE);

                // Create and remove test mount
                $oc->MountDataFolder(TRUE);
                $oc->GetUserMount(TRUE);
                $oc->UnmountUserMount(NULL, TRUE);

            } catch (OwnCloudConnectionException $e) {

                $oc->error_message = $e->getMessage();
                drupal_set_message($oc->error_message, 'warning');

                break;
            }
        }

    }


}
