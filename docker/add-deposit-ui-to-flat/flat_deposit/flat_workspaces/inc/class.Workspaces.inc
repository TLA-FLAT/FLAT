<?php
module_load_include('inc', 'node', 'node.pages');
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
                '#description' => 'Select the workspace folder within the departments\' project folder containing the data for this bundle',
                '#states' => array(
                    // Hide this select menu if source of resources in not local.
                    'visible' => array(
                        ':input[name="flat_source[und]"]' => array('value' => 'local'),
                    ),
                ),

                'department' => array(
                    '#type' => 'radios',
                    '#title' => t('Department'),
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
                    '#ajax' => array(
                        'callback' => 'reload_workspace_navigation_container_ajax_callback',
                        'wrapper' => 'workspace_navigation',
                        'effect' => 'fade',
                    ),
                ),

                'project' => array(
                    '#title' => 'Project',
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
                'next_directories' => array(
                    '#title' => 'Navigate to',
                    '#type' => 'select',
                    '#empty_option' => '-- nothing selected --',
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

    /**
     * Get the path of a bundles" resource files. If value of node field 'original_path' is not set yet, path is set to project root.
     * Otherwise, if project root folder has changed set 'original path' to root folder. Otherwise take saved value

     * @param $form_state
     * @param $project_root
     * @return string
     */
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

     static function getSubdirectoryNames($directory, $filters=[]){

        $subdirectories = [];
        if (!file_exists($directory)){
            return $subdirectories;
        }

        $results = scandir($directory);

        foreach ($results as $result) {

            $full_path = $directory . '/' . $result;

            $check = self::passedAllChecks($full_path, $filters);

            if (!$check) {

                    continue;

            } else {

                $subdirectories[] = $result;

            }

        }

        return $subdirectories;

    }

    /**
     * Checks if a certain directoru
     * @param $directory
     * @param array $filters
     * @return bool
     */
    static function passedAllChecks($directory, $filters=[]){

        $checkPassed = [];

        foreach ($filters as $filter) {

            if (!self::$filter($directory)) {

                $checkPassed[] = FALSE;

            } else {

                $checkPassed[] = TRUE;

            }
        }

        return !in_array(FALSE, $checkPassed, TRUE);

    }

    static function isAccessibleDir($directory){
        return is_dir($directory) AND is_readable($directory) ? TRUE: FALSE;
    }

    static function isAccessible($directory){
        return is_readable($directory);
    }

    static function isDir($directory){
        return is_dir($directory) ;
    }

    static function isNotCurrentDir($directory){


        return basename($directory) != '.';
    }
    static function isNotProjectParentDir($directory)
    {

        if (basename(dirname($directory)) == variable_get('flat_workspaces')['archive_folder'] AND basename($directory) == '..') {

            return false;

        } else {

            return true;

        }
    }


    static function userHasAclPermissions($directory){
        module_load_include('inc', 'flat_workspaces','inc/class.AclHandler');
        $acl = new AclHandler(drupal_realpath($directory));

        global $user;
        $id = $user->name == 'admin' ? exec('whoami') : $user->name;
        $id = str_replace('@mpi.nl', '', $id);

        return $acl->checkWriteAccess($id) ;
    }


}

function reload_workspace_navigation_container_ajax_callback ($form, $form_state)
{

    return $form['workspace_navigation'];
}









