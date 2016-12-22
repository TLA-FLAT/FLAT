<?php
/**
 * Created by PhpStorm.
 * User: danrhe
 * Date: 02/11/16
 * Time: 17:15
 */




/**
 * Implements hook_form().
 *
 * flat_owncloud admin menu which can be used to adapt the default settings.
 *
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
 * keeps the default password in case no password has been entered
 */
function keep_default_password($form, &$form_state) {


    if (empty($form_state['owncloud']['admin_pass'])) {
        $form_state['values']['owncloud']['admin_pass'] = variable_get('flat_owncloud')['admin_pass'];
    }

}

/**
 * function call to check if owncloud is reachable using the owncloud command line tool (occ).
 *
 *
 * @return array|bool if successfully status of occ otherwise FALSE
 */
function check_owncloud_status()
{
    $status = FALSE;

    $config = variable_get('flat_owncloud');

    $cmd = $config['root_dir'] . '/occ status --output=json';
    exec($cmd, $output, $return_val);

    if (!$return_val) {
        $formatted = (array)json_decode($output[0]);

        if ($formatted['installed']){


            return $formatted;
        }

    }

    return $status;
}


/**
 * function call to check if owncloud external files extension (files_external app) is enabled using the owncloud command line tool (occ).
 *
 * @param $enable bool If true function tries to enable the extension
 *
 * @return bool
 */
function owncloud_external_files_plugin_enabled($enable=FALSE)
{
    $enabled = FALSE;


    // occ needs to be called from owncloud root directory as apache user
    $config = variable_get('flat_owncloud');

    $cmd = $config['root_dir'] . '/occ app:list --output=json';
    exec ( $cmd , $output, $return_val);

    // In case of positive return value (0 = no errors), check on shell output the status of the plugin
    if (!$return_val) {

        $formatted = (array)json_decode($output[0]);

        // in case bool is set and plugin is disabled, enable the plugin and rerun listing of plugin status
        if (array_key_exists('files_external', $formatted['disabled']) AND $enable) {

            $cmd = $config['root_dir'] . '/occ app:enable files_external';
            exec($cmd, $output, $return_val);

            $cmd = $config['root_dir'] . '/occ app:list --output=json';
            exec ( $cmd , $output, $return_val);

            if ($output) {

                $formatted = (array)json_decode($output[0]);

            }

        }

        if (array_key_exists('files_external', $formatted['enabled'])) {

            $enabled = TRUE;
        }
    }

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

                $activated = owncloud_external_files_plugin_enabled(TRUE);

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

            $activated = owncloud_external_files_plugin_enabled();

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
