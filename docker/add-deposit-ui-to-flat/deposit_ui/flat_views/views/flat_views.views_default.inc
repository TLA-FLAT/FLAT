<?php
/**
 * @file
 * views related code.
 *
 *
 * It implements :
 *   hook_views_default_views()
 *
 */


/**
 * Implements hook_views_default_views().
 * load the views exported in code
 *  */


function flat_views_views_default_views() {
  $path = './' . drupal_get_path('module', 'flat_views') . '/views/flat_views.views*.inc';
  $views = array();
  foreach(glob($path) as $views_filename) {
    require_once($views_filename);
  }
  return  $views;
}



