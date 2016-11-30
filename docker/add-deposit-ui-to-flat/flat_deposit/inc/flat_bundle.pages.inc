<?php
/**
 * Created by PhpStorm.
 * User: danrhe
 * Date: 03/08/16
 * Time: 16:14
 */

/**
 * Menu callback; displays a Drupal page containing recent upload bundles of a given user.
 */
function flat_bundle_page_user($account) {
    global $user;

    drupal_set_title($title = t("@name's upload bundle", array('@name' => format_username($account))), PASS_THROUGH);

    $build = array();

    $query = db_select('node', 'n')->extend('PagerDefault');
    $nids = $query
        ->fields('n', array('nid', 'sticky', 'created'))
        ->condition('type', 'flat_bundle')
        ->condition('uid', $account->uid)
        ->condition('status', 1)
        ->orderBy('sticky', 'DESC')
        ->orderBy('created', 'DESC')
        ->limit(variable_get('default_nodes_main', 10))
        ->addTag('node_access')
        ->execute()
        ->fetchCol();

    if (!empty($nids)) {
        $nodes = node_load_multiple($nids);
        $build += node_view_multiple($nodes);
        $build['pager'] = array(
            '#theme' => 'pager',
            '#weight' => 5,
        );
    }
    else {
        if ($account->uid == $user->uid) {
            drupal_set_message(t('You have not created any upload bundle.'));
        }
        else {
            drupal_set_message(t('!author has not created any upload bundle entries.', array('!author' => theme('username', array('account' => $account)))));
        }
    }

    return $build;
}
