<?php


/**
 * Template session form. Can be populated by saved meta data (md). This is done by filling form default_values with data from a saved php-array.

 * @param $extra string path variable extending the ordinary registered path which will be used as additional variable in functions' logic
 * @return array
 */
function flat_deposit_cmdi_create_form($form, &$form_state){

    $node = menu_get_object() ? menu_get_object() : $form_state['values']['node'];
    $wrapper = entity_metadata_wrapper('node',$node);

    $profile = $wrapper->flat_cmdi_file->value();


    // load form corresponding to profile
    module_load_include('inc','flat_deposit','Helpers/CMDI_profiles');

    $profile_handler = new CMDI_profiles($profile);
    $profile_handler->getForm();
    $form = $profile_handler->fields;

    //add keys of form fields to form
    $form['form_fields_keys'] = array(
        '#type' => 'value',
        '#value' => array_keys($form));


    $form['ajax_trigger'] = array(
        '#type' => 'checkbox',
        '#title' => t('Save input as template'),
        '#ajax' => array(
            'callback' => 'flat_deposit_cmdi_save_form_ajax_callback',
            'wrapper' => 'input_field',
            'effect' => 'fade',
        ),
        '#disabled' =>TRUE,
    );

    $form['input_field'] = array(
        '#type' => 'markup',
        '#prefix' => '<div id="input_field">',
        '#suffix' => '</div>',
    );

    // Since checkboxes return TRUE or FALSE, we have to check that
    // $form_state has been filled as well as what it contains.
    if (!empty($form_state['values']['ajax_trigger']) && $form_state['values']['ajax_trigger']) {
        $form['input_field']['template_name'] = array(
            '#type' => 'textfield',
            '#title' => t('Name of template'),
        );

    }

    $form['select']['node'] = [
        '#type' => 'value',
        '#value' => $node,
    ];


    $form['submit'] = array (
        '#type' => 'submit',
        '#value' => t('Save'),
    );

    // if template file has been selected load it into memory and put values as default values in form
    if (isset($form_state['build_info']['args'][1])) {

        $template = $form_state['build_info']['args'][1];

        if ($template == 'debug'){

            $md = get_example_md($profile); #uncomment to get example data

        } else {

            global $user;
            $file = $form_state['build_info']['args'][1];
            $md = include 'private://flat_deposit/' . $user->name . '/Templates/' . $file . '.php';
        }


    // validation that all fields of the loaded data are also in the form
        // TODO validate merge cmdi profile and template
        $intersecting_fields = recursive_array_intersect_key($md, $form);
        $n_intersect = count($intersecting_fields, COUNT_RECURSIVE);
        $n_default_values = recursiveFind ($md, '#default_value');

        // compare
        if ($n_intersect > 0){

            $form = array_merge_recursive($md, $form);

        } else {

            drupal_set_message('No matched fields found');

        }
    }

    return $form;

}


function flat_deposit_cmdi_save_form_ajax_callback($form, $form_state)
{
    return $form['input_field'];
}


function flat_deposit_cmdi_create_form_validate($form, &$form_state){


    // Checks for saving the templates
    if ($form_state['values']['ajax_trigger']){

        // Can directory be created?
        global $user;
        $template_dir = 'private://flat_deposit/' . $user->name . '/Templates';

        if (!file_exists($template_dir)) {drupal_mkdir($template_dir);}

        if (!file_exists($template_dir)) {

            form_set_error ('cmdi_error', "Couldn't create directory for saving templates. Please contact administator");
            return $form;
        }

        // Is template directory owned by apache_user and writable
        $owner = posix_getpwuid(fileowner($template_dir));
        $config = variable_get('flat_deposit_names');

        if ($owner['name'] != $config['apache_user']) {
            form_set_error ('input_field', "Template directory is not writable by application. Please contact administator");
            return $form;
        }

        // Is template directory name a valid name ?
        $template_name = $form_state['values']['template_name'];
        if (!preg_match('/^[a-zA-Z0-9\_]{2,40}+$/', $template_name)) {
            form_set_error ('input_field', "The name of template directory may contain letters, numbers and underscores (max length 40 characters)");
        }

        if (file_exists( $template_dir . '/' . $template_name . '.php')) {
            form_set_error ('input_field', "A template file with entered name already exists");
        }

    }




    #form_set_error('debug','debug');
    return $form;

}

/**
 * On submission the form is optionally saved as template
 *
 */
function flat_deposit_cmdi_create_form_submit($form, &$form_state){
    $node = $form_state['values']['node'];
    $wrapper = entity_metadata_wrapper('node', $node);

    $bundle = $node->title;
    $collection = $wrapper->flat_parent_title->value();
    $template = $wrapper->flat_cmdi_file->value();

    /*
    // Save data as template (if checked)
    $data = array();
    foreach ($form_state['values']['form_fields_keys'] as $field) {

        // append array of values
        $data[$field] = array();

        foreach ($field as $key => $value){

        }

        $data[$field] = array('#default_value' => $form_state['values'][$field]);

    }
    krumo($data);
    if ($form_state['values']['save_template']) {
        $template_dir = USER_DRUPAL_DATA_DIR . '/Templates';
        $template_name = $form_state['values']['template_name'];

        file_put_contents( "$template_dir/$template_name.php", "<?php return ". var_export( $data, true ).";\n" );

    }
    */

    // generate CMDI xml and save to disc
    module_load_include('php','flat_deposit','Helpers/CMDI_Handler');
    /*$cmd = new CMDI_Handler($node);

    # prepare xml
    $cmd->getXML();

    # fill/change xml object
    $cmd->changeHeader();
    $cmd->processFormData($form_state['values']);
    $cmd->addComponentInfoToXml();

    $cmd->xml->asXML($cmd->export_file);
    drupal_set_message(t('Created metadata file from template ' . $template));
*/
    $form_state['redirect'] ="node/$node->nid";

}
