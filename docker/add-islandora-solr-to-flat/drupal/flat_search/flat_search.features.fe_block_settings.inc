<?php
/**
 * @file
 * flat_search.features.fe_block_settings.inc
 */

/**
 * Implements hook_default_fe_block_settings().
 */
function flat_search_default_fe_block_settings() {
  $export = array();

  $export['version'] = '2.0';

  $export['islandora_solr-basic_facets'] = array(
    'cache' => -1,
    'custom' => 0,
    'delta' => 'basic_facets',
    'module' => 'islandora_solr',
    'node_types' => array(),
    'pages' => '',
    'roles' => array(),
    'themes' => array(
      'bartik' => array(
        'region' => '',
        'status' => 0,
        'theme' => 'bartik',
        'weight' => 0,
      ),
      'bootstrap' => array(
        'region' => '',
        'status' => 0,
        'theme' => 'bootstrap',
        'weight' => 0,
      ),
      'flat_bootstrap_theme' => array(
        'region' => 'sidebar_first',
        'status' => 1,
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

  $export['islandora_solr-simple'] = array(
    'cache' => -1,
    'custom' => 0,
    'delta' => 'simple',
    'module' => 'islandora_solr',
    'node_types' => array(),
    'pages' => '',
    'roles' => array(),
    'themes' => array(
      'bartik' => array(
        'region' => '',
        'status' => 0,
        'theme' => 'bartik',
        'weight' => 0,
      ),
      'bootstrap' => array(
        'region' => '',
        'status' => 0,
        'theme' => 'bootstrap',
        'weight' => 0,
      ),
      'flat_bootstrap_theme' => array(
        'region' => 'sidebar_first',
        'status' => 1,
        'theme' => 'flat_bootstrap_theme',
        'weight' => -15,
      ),
      'seven' => array(
        'region' => '',
        'status' => 0,
        'theme' => 'seven',
        'weight' => 0,
      ),
    ),
    'title' => '<none>',
    'visibility' => 0,
  );

  $export['search-form'] = array(
    'cache' => -1,
    'custom' => 0,
    'delta' => 'form',
    'module' => 'search',
    'node_types' => array(),
    'pages' => '',
    'roles' => array(),
    'themes' => array(
      'bartik' => array(
        'region' => 'sidebar_first',
        'status' => 1,
        'theme' => 'bartik',
        'weight' => -1,
      ),
      'bootstrap' => array(
        'region' => 'sidebar_first',
        'status' => 1,
        'theme' => 'bootstrap',
        'weight' => -1,
      ),
      'flat_bootstrap_theme' => array(
        'region' => '',
        'status' => 0,
        'theme' => 'flat_bootstrap_theme',
        'weight' => -12,
      ),
      'seven' => array(
        'region' => 'dashboard_sidebar',
        'status' => 1,
        'theme' => 'seven',
        'weight' => -10,
      ),
    ),
    'title' => '',
    'visibility' => 0,
  );

  return $export;
}
