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
function flat_deposit_ui_owncloud_form($form, &$form_state)
{
    $form = array();

    $form['overview'] = array(
        '#markup' => t('Administrators can configure here owncloud settings. It is required that owncloud runs on the same server as this drupal installation'),
        '#prefix' => '<p>',
        '#suffix' => '</p>',);

    $form['settings'] = array(
        '#type' => 'fieldset',
        '#title' => t('Owncloud settings'));


    $form['settings']['activated'] = array (
        '#title' => t('Use Owncloud'),
        '#description' => t('Allow incorporation of owncloud in module'),
        '#type' => 'checkbox',
        '#default_value' => variable_get('owncloud_settings',array())['activated'],
        '#required' => FALSE);


    $form['settings']['admin_name'] = array (
        '#type' => 'textfield',
        '#title' => 'Owncloud admin name',
        '#description' => t('The owncloud admin user name'),
        '#default_value' => variable_get('owncloud_settings',array())['admin_name'],
        '#required' => TRUE,);

    $form['settings']['admin_pass'] = array (
        '#type' => 'password',
        '#title' => 'Owncloud admin password',
        '#description' => t('The owncloud admin user password'),
        '#default_value' => variable_get('owncloud_settings',array())['admin_pass'],
        '#required' => FALSE,);

    $form['settings']['root_dir'] = array (
        '#type' => 'textfield',
        '#title' => 'Owncloud root directory',
        '#description' => t('The owncloud installation path'),
        '#default_value' => variable_get('owncloud_settings',array())['root_dir'],
        '#required' => TRUE,);

    $form['settings']['temp_dir'] = array (
        '#type' => 'textfield',
        '#title' => 'Owncloud temp directory',
        '#description' => t('Directory where owncloud can read/write temporary files (e.g. mount_user.json)'),
        '#default_value' => variable_get('owncloud_settings',array())['temp_dir'],
        '#required' => TRUE,);

    $form['submit'] = array (
        '#type' => 'submit',
        '#value' => t('Save'));

    $form['restore'] = array (
        '#type' => 'submit',
        '#value' => t('Reset to defaults'),
    );

    $form['check'] = array (
        '#type' => 'submit',
        '#value' => t('Check owncloud connection'),);

    return $form;
}



function flat_deposit_ui_owncloud_form_validate ($form, &$form_state)
{
    $button = $form_state['values']['op'];

    switch ($button) {
        case 'Save': {

            break;
        }
        case 'Reset to defaults': {

            break;
        }
        case 'Check owncloud connection': {

            break;
        }

    }

}


function flat_deposit_ui_owncloud_form_submit ($form, &$form_state)
{
    $button = $form_state['values']['op'];

    switch ($button) {
        case 'Save': {

            variable_set('owncloud_settings', array(
                'activated' => $form_state['values']['activated'],
                'admin_name' => $form_state['values']['admin_name'],
                'admin_pass' => $form_state['values']['admin_pass'],
                'root_dir' => $form_state['values']['root_dir'],
                'temp_dir' => $form_state['values']['temp_dir']));

            drupal_set_message(t('Changed values have been saved'));

            break;
        }
        case 'Reset to defaults': {

            module_load_include('inc', 'flat_deposit_ui', 'inc/config');

            flat_deposit_set_owncloud();
            drupal_set_message(t('Owncloud settings have been reset to default'));

            break;
        }
        case 'Check owncloud connection': {


            try {

                $oc = new OwnCloud();
                if (!$oc->noErrors){
                    throw new OwnCloudConnectionException($oc->error_message);
                }
                $oc->ListAllDrupalMounts(TRUE);

                // Remove existing mount
                $oc->GetUserMount(TRUE);
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
