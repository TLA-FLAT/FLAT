<?php

/**
 * CMDI selector form
 */
function flat_deposit_cmdi_select_form($form, &$form_state)
{

    global $user;
    $link_filebrowser = 'user/' . $user->uid . '/imce';

    $node = menu_get_object() ? menu_get_object() : $form_state['values']['node'];
    $bundle = $node->title;
    $wrapper = entity_metadata_wrapper('node', $node);
    $collection = $wrapper->upload_collection->value();

    $path = t('!name/!collection/!bundle', array('!name'=>$user->name,'!collection'=>$collection,'!bundle'=>$bundle));


    $form = array();

    $form['link'] = array(
        '#theme' => 'link',
        '#text' => 'Return to metadata info',
        '#path' => 'node/' . $node->nid . '/cmdi_info',
        '#options' => array(
            'attributes' => array(),
            'html' => FALSE,
        ),
        '#prefix' => '<p>',
        '#suffix' => '</p>',
    );



    $form['select'] = array(
        '#title' => t('METADATA File Creator'),
        '#type' => 'fieldset',
        '#attributes' => array(
            'id' => 'select-cmdi',)
    );

    $list_allowed_profiles = list_allowed_values(field_info_field('upload_cmdi'));

    $form['select']['cmdi_select'] = [
        '#type' => 'select',
        '#title' => t('Please select metadata profile'),
        '#options' => $list_allowed_profiles,
        '#default_value' => array_keys($list_allowed_profiles, $wrapper->upload_cmdi->value()),
    ];
/*
    $form['select']['checkbox_ajax_trigger'] = array(
        '#title' => t('Fill template with saved meta data'),
        '#type' => 'checkbox',
        '#ajax' => array(
            'callback' => 'flat_deposit_cmdi_select_ajax_callback',
            'wrapper' => 'select-cmdi',
            'method' => 'replace',
            'effect' => 'fade',
        ),
    );
*/
    $form['select']['link'] = array(
        '#markup' => t('Note: Manually uploaded metadata files should be !link manually to the metadata directory within the folder \'%path\'. 
        Naming convention of that file is %filename. Resource related fields will be formatted!! ', array('!link' => l(t('added'),$link_filebrowser), '%path' => $path, '%filename' => 'record.cmdi' )),
        '#weight' => 99,
    );

    $form['select']['node'] = [
        '#type' => 'value',
        '#value' => $node,
    ];

    $form['submit'] = array (
        '#type' => 'submit',
        '#value' => t('Save'),
        '#validate' => array('flat_deposit_cmdi_select_form_validate_final'),
    );

    /*
        // In case AJAX is triggered
        $checked = isset($form_state['values']['checkbox_ajax_trigger']) ? $form_state['values']['checkbox_ajax_trigger'] : 0 ;
        if ($checked) {

            // Search for available template data files in a certain data directory (e.g. Templates). // In case data files are available
            // a checkbox is added allowing to fill
            $template_dir = 'private://flat_deposit/' . $user->name . '/Templates/';
            $listing = array_diff(scandir($template_dir), array('..', '.'));
            $templates = [];

            foreach ($listing as $f) {
                $sub = substr($f, 0, -4);
                $templates[$sub] = $sub;
            }
            if (!$templates) {
                drupal_set_message(t('No saved meta data are available'), 'warning');

            } else {

                $form['select']['select_data_file'] = array(
                    '#type' => 'select',
                    '#title' => t('Select a file containing meta data'),
                    '#options' => $templates);
            }
        }
    */
    return $form;

}



/**
 * ajax callback for updating the select form field
 */
/*
function flat_deposit_cmdi_select_ajax_callback ($form, &$form_state)
{

    return $form['select'];
}
*/

/**
 *
 */

function flat_deposit_cmdi_select_form_validate_final($form, &$form_state){

//todo validate that saved meta data fit with profile

    return $form;
}


function flat_deposit_cmdi_select_form_submit($form, &$form_state){

    $node = $form_state['values']['node'];
    $wrapper = entity_metadata_wrapper('node', $node);

    $list_allowed_profiles = array('AAAA','BBBB','CCCC');
    $profile_key = $form_state['values']['cmdi_select'];
    $profile_value = $list_allowed_profiles[$form_state['values']['cmdi_select']];

    // Set new upload_cmdi value
    $wrapper->upload_cmdi->set($profile_value);
    $wrapper->save();
    drupal_set_message(t('Metadata profile has been saved'));

    // Redirect to creation form if no record.cmdi exists
    if (!$wrapper->upload_cmdi->value()){

        if (isset($form_state['values']['select_data_file'])){
            $md = '/' . $form_state['values']['select_data_file'];
            $form_state ['redirect'] = "node/$node->nid/cmdi/create" . $md;
        } else {
            $form_state ['redirect'] = "node/$node->nid/cmdi/create";
        }




    }




}

