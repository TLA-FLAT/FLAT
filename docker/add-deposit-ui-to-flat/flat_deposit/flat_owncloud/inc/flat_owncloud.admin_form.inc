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
        '#value' => t('Save'));

    $form['button']['restore'] = array (
        '#type' => 'submit',
        '#value' => t('Reset to defaults'),
    );

    $form['button']['check'] = array (
        '#type' => 'submit',
        '#value' => t('Check owncloud connection'),);

    return $form;
}



function flat_owncloud_admin_form_validate ($form, &$form_state)
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

            flat_set_owncloud();
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
