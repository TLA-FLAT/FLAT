<?php


function flat_reports_action_form($form, &$form_state)
{

    global $user;
    $form = array();
    $form['actions']['container'] = array(
        '#type' => 'container',
        '#attributes' => array('class' => array('container-inline')),
    );

    $form['actions']['delete_all_reports'] = array(
        '#type' => 'submit',
        '#value' => t('Delete all reports'),
        '#submit' => array('flat_reports_action_form_delete_all_submit'),
    );

    $form['actions']['delete_all_reports_2_weeks'] = array(
        '#type' => 'submit',
        '#value' => t('Delete all reports older than 2 weeks'),
        '#submit' => array('flat_reports_action_form_delete_2_weeks_submit'),
    );

    return $form;

}

function flat_reports_action_form_delete_2_weeks_submit($form, &$form_state){
    global $user;

    $twoWeeksAgo = time() - (14*24*60*60) ;
    $query = db_delete('node')
        ->condition('type', 'blog', '=')
        ->condition('uid', $user->uid, '=')
        ->condition('changed', $twoWeeksAgo, '<=');
    $num_deleted = $query->execute();


    drupal_set_message('Deleted all reports older than 2 weeks');
}


function flat_reports_action_form_delete_all_submit($form, &$form_state){
    global $user;


    $query = db_delete('node')
        ->condition('type', 'blog', '=')
        ->condition('uid', $user->uid, '=');
    $num_deleted = $query->execute();


drupal_set_message('Deleted all reports');
}


