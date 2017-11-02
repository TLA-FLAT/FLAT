<?php

/**
 * @file
 * Views hooks and callback registries.
 */

function flat_views_views_data_alter(&$data)
{
 #   var_dump('flat_views_views_data_alter');
    // This example alters the title of the node:nid field in the Views UI.
    $data['node']['nid']['title'] = t('Node-Nide');
}

function flat_views_views_pre_render(&$view) {
    if ($view->name == 'FLAT My Collections View') {
#        krumo($view->result);
        foreach ($view->result as $key => $result) {

            $val = $result->field_data_flat_fid_flat_fid_value;
            $link = l('link to archived collection', 'islandora/object/' . rawurlencode($val));
            $result->field_flat_flat_fid[0]['rendered']['#markup'] = $link;
        }
    }
}
