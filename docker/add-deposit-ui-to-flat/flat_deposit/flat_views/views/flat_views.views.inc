<?php

/**
 * @file
 * Views hooks and callback registries.
 */

/**
* Implements hook_views_data().
 */
function flat_views_views_data() {
    $data['flat_views']['table']['group'] = t('Flat Views');
    $data['flat_views']['table']['join'] = array(
        // Exist in all views.
        '#global' => array(),
    );

    $data['flat_views']['Edit bundle'] = array(
        'title' => t('Edit bundle'),
        'help' => t('Redirects to node view.'),
        'field' => array(
            'handler' => 'views_handler_my_custom_field',
        ),
    );

    return $data;
}

function flat_views_views_data_alter(&$data){

 #   var_dump($data['field_data_flat_cmdi_file']['flat_cmdi_file']);
#    $data['field_data_flat_cmdi_file']['flat_cmdi_file']['field']['handler'] = 'flat_views_views_handler_cmdi_file_field';

    return $data;
}


function flat_views_views_pre_render(&$view) {
    if ($view->name == 'FLAT My Collections View') {
#        krumo($view->result);
        foreach ($view->result as $key => $result) {

            $val = $result->field_data_flat_fid_flat_fid_value;
            $link = l('link', 'islandora/object/' . rawurlencode($val));
            $result->field_flat_flat_fid[0]['rendered']['#markup'] = $link;
        }
    }
}
