<?php
/**
 * Created by PhpStorm.
 * User: danrhe
 * Date: 02/11/16
 * Time: 17:17
 */

/*
 * Implements hook_form_alter().
 */
function demo_form_alter(&$form, &$form_state, $form_id) {
    #krumo($form_id);
    switch ($form_id) {
        case 'user_profile_form' :

            $form['account']['pass']['#description'] = t('To change the current user password, enter the new password in both fields. Please do not use common passwords like "password", "12345", "love".');
            #krumo($form);

            break;
    }
}

function flat_deposit_ui_cmdi_select_form($form, &$form_state){

    $form = array();

    $form['select_template_fieldset'] = array(
        '#title' => t('Form Template Selection'),
        '#type' => 'fieldset',
        '#prefix' => '<div id="select-div">',
        '#suffix' => '</div>',
    );

    $form['select_template_fieldset']['cmdi_select'] = [
        '#type' => 'select',
        '#title' => t('Please select meta data template'),
        '#options' => [
            'session' => t('session'),
            'experiment' => t('experiment'),
            'minimal' => t('minimal'),
        ],
    ];

    $form['select_template_fieldset']['checkbox_ajax_trigger'] = array(
        '#title' => t('Fill template with saved meta data'),
        '#type' => 'checkbox',
        '#ajax' => array(
            'callback' => 'flat_deposit_ui_cmdi_select_ajax_callback',
            'wrapper' => 'select-div',
            'method' => 'replace',
            'effect' => 'fade',
        ),
    );

// The select_data_file field is only shown when checkbox_ajax_trigger is checked
    $checked = isset($form_state['values']['checkbox_ajax_trigger']) ? $form_state['values']['checkbox_ajax_trigger'] : 0 ;
    if ($checked) {


        // Search for available tempalte data files in a certain data directory (e.g. Templates). // In case data files are
        // a checkbox is added allowing to fill
        $template_dir = USER_DRUPAL_DATA_DIR . '/Templates/';
        $listing = array_diff(scandir($template_dir), array('..', '.'));
        $templates = [];

        #foreach ($listing as $f) {$templates[(string)$i] = substr($f, 0, -4);$i++;} #make list of de-suffixed names
        foreach ($listing as $f) {
            $sub = substr($f, 0, -4);
            $templates[$sub] = $sub;
        } #make list of de-suffixed names

        $form['select_template_fieldset']['select_data_file'] = array(
            '#type' => 'select',
            '#title' => t('Select a file containing meta data'),
            '#options' => $templates);

        if (!$templates) {
            drupal_set_message('No saved meta data are available', 'warning');
        }

    }

    $form['submit'] = array (
        '#type' => 'submit',
        '#value' => t('Select'),
        '#submit' => array('flat_deposit_ui_cmdi_select_form_submit'),
        '#validate' => array('flat_deposit_ui_cmdi_select_form_validate'));

    return $form;
}

function flat_deposit_ui_cmdi_select_ajax_callback ($form, &$form_state)
{

    return $form['select_template_fieldset'];
}


function flat_deposit_ui_cmdi_select_form_validate($form, &$form_state){
    $node = $form_state['build_info']['args'][0];
    $template = $form_state['values']['cmdi_select'];

    module_load_include('php','flat_deposit_ui','Helpers/CMDI_Handler');
    $cmd = new CMDI_Handler($node, $template);

    if ($cmd->projectCmdiFileExists()) form_set_error ('ingest_error', t('CMDI file already exists at target location'));
    return $form;
}


function flat_deposit_ui_cmdi_select_form_submit($form, &$form_state){
    $template = $form_state['values']['cmdi_select'];
    $node = $form_state['build_info']['args'][0];
    $action = ($form_state['build_info']['args'][1] == 'createandfreeze') ? $form_state['build_info']['args'][1] : 'createcmdi';

    if (isset($form_state['values']['select_data_file'])){
        $md = '/' . $form_state['values']['select_data_file'];
        $form_state ['redirect'] = "node/$node->nid/cmdi/$template/$action" . $md;
    } else {
        $form_state ['redirect'] = "node/$node->nid/cmdi/$template/$action";
    }


}


/**
 * Template session form. Can be populated by saved meta data (md). This is done by filling form default_values with data from a saved php-array.

 * @param $extra string path variable extending the ordinary registered path which will be used as additional variable in functions' logic
 * @return array
 */
function flat_deposit_ui_cmdi_create_form($form, &$form_state, $extra){
    $node = $form_state['build_info']['args'][0];
    $template = $form_state['build_info']['args'][1];

    $bundle = $node->title;
    $collection = field_get_items('node', $node, 'upload_collection')[0]['value'];

    // Validation of meta data file might be handy and could be included here
    // todo check passing extra variable
    $extra = NULL;
    if (isset($extra)){
        $file = $form_state['build_info']['args'][4];
        $md = include USER_DRUPAL_DATA_DIR . '/Templates/'. $file . '.php';

    } else {$md = NULL;
    };

    module_load_include('php','flat_deposit_ui','Helpers/CMDI_Handler');

    $md = get_example_md($template); #uncomment to get example data

    // remove md for unfilled form
    $form = get_template_form($template, $bundle,$md);


    //add names of form fields to form
    $form['form_fields_template'] = array(
        '#type' => 'value',
        '#value' => array_keys($form));


    // add data fields and submit-button to template form
    $form ['data_template']= array(
        '#type' => 'value',
        '#value' => array('template' => $template,
            'bundle' => $bundle,
            'collection' =>$collection));


    $form['save_template'] = array(
        '#type' => 'checkbox',
        '#title' => t('Save input as template'),
        '#ajax' => array(
            'callback' => 'flat_deposit_ui_cmdi_save_form_ajax_callback',
            'wrapper' => 'input_field',
            'effect' => 'fade',
        ),
    );

    $form['input_field'] = array(
        '#type' => 'markup',
        '#prefix' => '<div id="input_field">',
        '#suffix' => '</div>',
    );

    // Since checkboxes return TRUE or FALSE, we have to check that
    // $form_state has been filled as well as what it contains.
    if (!empty($form_state['values']['save_template']) && $form_state['values']['save_template']) {
        $form['input_field']['template_name'] = array(
            '#type' => 'textfield',
            '#title' => t('Name of template'),
        );

    }
    $form['submit'] = array (
        '#type' => 'submit',
        '#value' => t('Save'),
        '#submit' => array('flat_deposit_ui_cmdi_create_form_submit'),
        '#validate' => array('flat_deposit_ui_cmdi_create_form_validate'));

    return $form;
}


function flat_deposit_ui_cmdi_save_form_ajax_callback($form, $form_state)
{
    return $form['input_field'];
}


function flat_deposit_ui_cmdi_create_form_validate($form, &$form_state){

// Checks for saving the templates
    if ($form_state['values']['save_template']){

        // Can directory be created?
        $template_dir = USER_DRUPAL_DATA_DIR . '/Templates';
        if (!file_exists($template_dir)) { drupal_mkdir($template_dir);}

        if (!file_exists($template_dir)) {
            form_set_error ('cmdi_error', "Couldn't create directory for saving templates. Please contact administator");
            return $form;
        }

        // Is template directory owned by apache_user and writable
        $owner = posix_getpwuid(fileowner($template_dir));
        if ($owner['name'] != APACHE_USER) {
            form_set_error ('cmdi_error', "Template directory is not writable by application. Please contact administator");
            return $form;
        }

        // Is template directory name a valid name ?
        $template_name = $form_state['values']['template_name'];
        if (!preg_match('/^[a-zA-Z0-9\_]{2,40}+$/', $template_name)) {
            form_set_error ('cmdi_error', "The name of template directory may contain letters, numbers and underscores (max length 40 characters)");
            return $form;
        }
        if (file_exists( $template_dir . '/' . $template_name . '.php')) {
            form_set_error ('cmdi_error', "A template file with entered name already exists");
            return $form;
        }

    }

    return $form;

}

/**
 * On submission the form is optionally saved as template
 *
 */
function flat_deposit_ui_cmdi_create_form_submit($form, &$form_state){
    $node = $form_state['build_info']['args'][0];
    $template = $form_state['build_info']['args'][1];
    $action = $form_state['build_info']['args'][2];

    $bundle = $node->title;
    $collection = field_get_items('node', $node, 'upload_collection')[0]['value'];


    $data = array();
    foreach ($form_state['values']['form_fields_template'] as $field) {$data[$field] = $form_state['values'][$field];}

    // Save data as template (if checked)
    if ($form_state['values']['save_template']) {
        $template_dir = USER_DRUPAL_DATA_DIR . '/Templates';
        $template_name = $form_state['values']['template_name'];

        file_put_contents( "$template_dir/$template_name.php", "<?php return ". var_export( $data, true ).";\n" );

    }

    // generate CMDI xml and save to disc
    module_load_include('php','flat_deposit_ui','Helpers/CMDI_Handler');
    $cmd = new CMDI_Handler($node, $template);

    # prepare xml
    $cmd->getXML();

    # fill/change xml object
    $cmd->changeHeader();
    $cmd->processFormData($form_state['values']);
    $cmd->addComponentInfoToXml();
    #$cmd->resources = $cmd->searchDir(USER_DRUPAL_DATA_DIR . "/$collection/$bundle");
    #$cmd->addResourcesToXml();


    $cmd->xml->asXML($cmd->export_file);

    $form_state['redirect'] ="node/$node->nid";
    drupal_set_message(t('Created metadata file from template ' . $template));


    // freeze directory in case this option is chosen
    if ($action == 'createandfreeze') {
        flat_bundle_freeze_bundle($node);
    }
}
