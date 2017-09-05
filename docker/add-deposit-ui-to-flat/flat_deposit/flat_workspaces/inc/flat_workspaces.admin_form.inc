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
function flat_workspaces_admin_form($form, &$form_state)
{
    $config = variable_get('flat_workspaces',array());

    $form = array();

    $form['overview'] = array(
        '#markup' => t('Administrators can configure local storages that provide data for ingestion'),
        '#prefix' => '<p>',
        '#suffix' => '</p>',);

    $form['workspace_settings'] = array(
        '#type' => 'fieldset',
        '#title' => t('Workspace settings'),
        '#tree' => TRUE,
        );

    $form['workspace_settings']['mount_folder'] = array(
        '#type' => 'textfield',
        '#title' => t('Path of mounted directory'),
        '#description' => t('Mounted drive of file server'),
        '#default_value' => $config['mount_folder'],
    );

    $form['workspace_settings']['workspace_folder'] = array (
        '#type' => 'textfield',
        '#title' => t('Workspace folder name'),
        '#description' => t('Folder name of the workspaces available within each department folder'),
        '#default_value' => $config['workspace_folder'],
        '#required' => TRUE,
    );

    $form['workspace_settings']['archive_folder'] = array (
        '#type' => 'textfield',
        '#title' => t('Name of accessible archive folder'),
        '#default_value' => $config['archive_folder'],
        '#required' => TRUE,
    );
// BUTTONS
    $form['buttons']['submit'] = array (
        '#type' => 'submit',
        '#value' => t('Save'),
        #'#validate' => array('flat_workspace_admin_form_save_validate',),
        '#submit' => array('flat_workspace_admin_form_save_submit',
        )
    );
    return $form;
}


function flat_workspace_admin_form_save_submit ($form, &$form_state)
{

    module_load_include('inc', 'flat_workspaces', 'inc/config');

    flat_workspaces_set_variables($form_state['values']['workspace_settings']);

    drupal_set_message(t('Changed values have been saved'));

}
