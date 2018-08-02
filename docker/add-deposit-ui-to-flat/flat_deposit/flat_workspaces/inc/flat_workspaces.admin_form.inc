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
        '#markup' => t('Enable the use of local network share workspaces for data upload'),
        '#prefix' => '<p>',
        '#suffix' => '</p>',);

    $form['workspace_settings'] = array(
        '#type' => 'fieldset',
        '#title' => t('Workspace settings'),
        '#tree' => TRUE,
        );

    $form['workspace_settings']['activated'] = array (
        '#title' => t('Use local workspaces'),
        '#description' => t('Enable the use of local network share workspaces for data upload'),
        '#type' => 'checkbox',
        '#default_value' => $config['activated'],
        '#required' => FALSE);

    $form['workspace_settings']['mount_folder'] = array(
        '#type' => 'textfield',
        '#title' => t('Workspaces root'),
        '#description' => t('Path of the workspaces root directory'),
        '#default_value' => $config['mount_folder'],
    );

    $form['workspace_settings']['department_mapping'] = array(
        '#type' => 'textarea',
        '#title' => t('Department directory mapping'),
        '#description' => t('Directory name to department name mapping (one mapping per line in the form: dirname = "department name"'),
        '#default_value' => $config['department_mapping'],
    );

    $form['workspace_settings']['workspace_folder'] = array (
        '#type' => 'textfield',
        '#title' => t('Workspaces folder name'),
        '#description' => t('Name of the folder that contains the workspaces within each department folder'),
        '#default_value' => $config['workspace_folder'],
        '#required' => TRUE,
    );

    $form['workspace_settings']['archive_folder'] = array (
        '#type' => 'textfield',
        '#title' => t('Archive Deposit folder name'),
        '#description' => t('Name of archive folder within each workspace, to be used for deposits'),
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
