<?php
/**
 * @file
 * meertens_flat.features.fe_block_settings.inc
 */

/**
 * Implements hook_default_fe_block_settings().
 */
function meertens_flat_default_fe_block_settings() {
  $export = array();

  $export['version'] = '2.0';

  $export['block-flat'] = array(
    'cache' => -1,
    'custom' => 0,
    'machine_name' => 'flat',
    'module' => 'block',
    'node_types' => array(),
    'pages' => '',
    'roles' => array(),
    'themes' => array(
      'bartik' => array(
        'region' => 'footer',
        'status' => 1,
        'theme' => 'bartik',
        'weight' => 0,
      ),
      'bootstrap' => array(
        'region' => 'footer',
        'status' => 1,
        'theme' => 'bootstrap',
        'weight' => 0,
      ),
      'flat_bootstrap_theme' => array(
        'region' => '',
        'status' => 0,
        'theme' => 'flat_bootstrap_theme',
        'weight' => 0,
      ),
      'seven' => array(
        'region' => '',
        'status' => 0,
        'theme' => 'seven',
        'weight' => 0,
      ),
    ),
    'title' => '',
    'visibility' => 0,
  );

  $export['system-navigation'] = array(
    'cache' => -1,
    'custom' => 0,
    'delta' => 'navigation',
    'module' => 'system',
    'node_types' => array(),
    'pages' => '',
    'roles' => array(),
    'themes' => array(
      'bartik' => array(
        'region' => 'sidebar_first',
        'status' => 1,
        'theme' => 'bartik',
        'weight' => 0,
      ),
      'bootstrap' => array(
        'region' => 'sidebar_first',
        'status' => 1,
        'theme' => 'bootstrap',
        'weight' => 0,
      ),
      'flat_bootstrap_theme' => array(
        'region' => '',
        'status' => 0,
        'theme' => 'flat_bootstrap_theme',
        'weight' => 0,
      ),
      'seven' => array(
        'region' => '',
        'status' => 0,
        'theme' => 'seven',
        'weight' => 0,
      ),
    ),
    'title' => '',
    'visibility' => 0,
  );

  return $export;
}
