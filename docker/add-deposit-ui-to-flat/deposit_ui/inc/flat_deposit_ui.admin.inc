<?php


function flat_deposit_ui_admin_form($form, &$form_state)
{
    $form = array();

    $form['overview'] = array(
        '#markup' => t('This interface allows administrators to customize the deposit\'s settings'),
        '#prefix' => '<p>',
        '#suffix' => '</p>',);

    $form['paths'] = array(
        '#type' => 'fieldset',
        '#title' => t('Paths'));

    $form['paths']['freeze'] = array (
        '#type' => 'textfield',
        '#title' => 'Freeze',
        '#description' => t('Absolute path of the location where frozen data will be stored'),
        '#default_value' => FREEZE_DIR,
        '#required' => TRUE,);

    $form['paths']['bags'] = array (
        '#type' => 'textfield',
        '#title' => 'Bags',
        '#description' => t('Absolute path of the location where Fedora can needs to search for complete SIP\'s that will be ingested'),
        '#default_value' => BAG_DIR,
        '#required' => TRUE,);

    $form['names']['apache_user'] = array (
        '#type' => 'textfield',
        '#title' => 'Apache User',
        '#description' => t('Name of the apache user (e.g. www-data, www-)'),
        '#default_value' => APACHE_USER,
        '#required' => TRUE,);

    $form['submit'] = array (
        '#type' => 'submit',
        '#value' => t('Save'));

    $form['restore'] = array (
        '#type' => 'submit',
        '#value' => t('Reset to defaults'),
        '#submit' => array('flat_deposit_ui_admin_form_reset_submit'));

    return $form;
}

function flat_deposit_ui_admin_form_submit ($form, &$form_state)
{
    variable_set('flat_deposit_paths', array(
        'freeze' => $form_state['values']['freeze'],
        'bag'=> $form_state['values']['bags']));

    variable_set('flat_deposit_names', array(
        'apache_user' => $form_state['values']['apache_user'],));

    drupal_set_message(t('Changed values have been saved'));
}

function flat_deposit_ui_admin_form_reset_submit ($form, &$form_state){
    module_load_include('inc', 'flat_deposit_ui', 'inc/config');
    flat_deposit_set_paths();
    flat_deposit_set_names();

    drupal_set_message(t('All settings have been reset to default'));
}
