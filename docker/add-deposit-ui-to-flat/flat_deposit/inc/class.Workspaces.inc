<?php

/**
 * Created by PhpStorm.
 * User: danrhe
 * Date: 04/05/2017
 * Time: 11:57
 */
class Workspaces
{


    static function get_container(){

        return array(
            '#weight' => 11,
            '#type' => 'container',
            '#tree' => FALSE,
            '#attributes' => array(
                'id' => array('workspace_navigation'),
            ),
            'field' => array(
                '#type' => 'fieldset',
                '#title' => 'Local Path Selector',
                '#description' => 'Select the folder to be archived',
                '#states' => array(
                    // Hide this select menu if source of resources in not local.
                    'visible' => array(
                        ':input[name="flat_source[und]"]' => array('value' => 'local'),
                    ),
                ),

                'department' => array(
                    '#type' => 'radios',
                    '#options' => array(
                        'lac' => 'Language and Cognition',
                        'ladd' => 'Language Development',
                        'lag' => 'Language and Genetics',
                        'nbl' => 'Neurobiology of Language',
                        'nvc' => 'Neurogenetics of Vocal Communication',
                        'pol' => 'Psychology of Language',
                        'tg' => 'Technical group',
                    ),
                    '#empty_option' => '-- Select Department --',
                    '#empty_value' => 'unselected',
                    '#title' => t('Department for which data will be archived'),
                    '#ajax' => array(
                        'callback' => 'reload_workspace_navigation_container_ajax_callback',
                        'wrapper' => 'workspace_navigation',
                        'effect' => 'fade',
                    ),
                ),

                'project' => array(
                    '#title' => 'Project name',
                    '#type' => 'select',
                    '#options' => array(),
                    '#empty_option' => '-- Select Project --',
                    '#empty_value' => 'none',
                    '#access' => FALSE,
                    '#ajax' => array(
                        'callback' => 'reload_workspace_navigation_container_ajax_callback',
                        'wrapper' => 'workspace_navigation',
                        'effect' => 'fade',
                    ),

                ),
                'previous_directory' => array(
                    '#type' => 'submit',
                    '#value' => t('Back one level'),
                    '#submit' => array('workspace_navigation_previous_directory_submit'),
                    '#ajax' => array(
                        'callback' => 'reload_workspace_navigation_container_ajax_callback',
                        'wrapper' => 'workspace_navigation',
                        'effect' => 'fade',
                    ),

                    '#limit_validation_errors' => array(
                        array('data', 'current_directory'),
                        array('project'),
                        array('department'),
                        array('next_directories'),
                        array('flat_original_path'),
                        array('flat_source'),
                    ),

                    '#access' => FALSE,
                ),


                'next_directories' => array(
                    '#title' => 'Traverse into subdirectory',
                    '#type' => 'select',
                    '#empty_option' => '-- not selected --',
                    '#empty_value' => '',
                    '#access' => FALSE,
                    '#options' => array(),
                    '#ajax' => array(
                        'callback' => 'reload_workspace_navigation_container_ajax_callback',
                        'wrapper' => 'workspace_navigation',
                        'effect' => 'fade',
                    ),

                ),

            )
        );

    }

    static function get_flat_original_path($form_state, $project_root){
        if (isset($form_state['values']['flat_source']['und'][0]['value'])) {
            if ($form_state['values']['flat_source']['und'][0]['value'] == 'external') {

                return '';
            }
        }
        // 1) If not set yet, set to project root. Otherwise,
        // 2) if project root folder has changed set to root folder.
        // 3) Otherwise take saved value
        $value = isset($form_state['values']['flat_original_path']['und'][0]['value']) ? $form_state['values']['flat_original_path']['und'][0]['value'] : $project_root;


        if (strpos ($value, $project_root ) === FALSE){

            $current = $project_root;

        } else {

            $current = $value ;

        }

        return $current;

    }
}


function getSubdirectoryNames($directory, $filters=[]){

    $subdirectories = [];
    if (!file_exists($directory)){return $subdirectories;}

    $results = scandir($directory);

    foreach ($results as $result) {
        if ($result === '.' or $result === '..') continue;

        $dir = $directory . '/' . $result;

        if (in_array('access', $filters)){

            if (!is_dir($dir) OR !is_readable($dir)) {
                continue;
            }

        }

        if (in_array('directories', $filters)){

            if (!is_dir($dir) ) {
                continue;
            }

        }


        $subdirectories[] = $result;
    }


    return $subdirectories;

}


function reload_workspace_navigation_container_ajax_callback ($form, $form_state)
{

    return $form['workspace_navigation'];
}









