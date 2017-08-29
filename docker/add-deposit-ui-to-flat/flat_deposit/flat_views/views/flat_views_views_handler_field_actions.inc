<?php

/**
 * @file
 * handler class for the new field 'actions'
 */

/**
 * class definition
 */
class flat_views_views_handler_field_actions extends views_handler_field {
  /**
   * Render function: return html output
   * Including edit and delete actions
   */
  function render($values) {
    // debug ($values);
    $actions = array();

    if (user_access('admin collection')) {
      $alt = t('Edit node');
      $actions[] = array(
        'name' => t('Edit'),
        'url' => 'admin/collections/manage/' . $values->collection_id ,
        'icon' => theme('image', array('path' => drupal_get_path('module', 'flat_views') . '/images/edit.gif', 'alt' => $alt)),
        'title' => $alt,
      );
    }
    if (user_access('admin collection')) {
      $alt = t('Delete collection - member relation');
      $actions[] = array(
        'name' => t('Delete'),
        'url' => 'admin/collections/manage/'  . $values->collection_id . '/delete',
        'icon' => theme('image', array('path' => drupal_get_path('module', 'flat_views') . '/images/delete.gif', 'alt' => $alt)),
        'title' => $alt,
      );
    }

    $output = '';
    foreach ($actions as $action) {
      $output .= l($action['icon'], $action['url'], array('attributes' => array('title' => $action['title']), 'html' => TRUE));
    }
    return $output;

  }
  /**
   *
   */
  function query() {
    // Do nothing, leave query blank, we render the contents
  }
}

