<?php
/**
 * @file
 * meertens_flat.features.inc
 */

/**
 * Implements hook_islandora_solr_fields_features_default_fields().
 */
function meertens_flat_islandora_solr_fields_features_default_fields() {
  return array(
  0 => array(
    'solr_field' => 'title',
    'field_type' => 'result_fields',
    'weight' => -10,
    'solr_field_settings' => array(
      'label' => 'Title',
      'snippet' => NULL,
      'date_format' => '',
      'maximum_length' => 0,
      'add_ellipsis' => FALSE,
      'wordsafe' => FALSE,
      'wordsafe_length' => 1,
      'permissions' => array(
        3 => 3,
      ),
      'link_to_object' => TRUE,
      'link_to_search' => FALSE,
    ),
  ),
  1 => array(
    'solr_field' => 'meertensCollection',
    'field_type' => 'facet_fields',
    'weight' => -10,
    'solr_field_settings' => array(
      'label' => 'Type',
      'pid_object_label' => NULL,
      'permissions' => array(
        3 => 3,
      ),
    ),
  ),
  2 => array(
    'solr_field' => 'clarin.eu_cr1_c_1440426460261_description',
    'field_type' => 'result_fields',
    'weight' => -9,
    'solr_field_settings' => array(
      'label' => 'Description',
      'snippet' => NULL,
      'date_format' => '',
      'maximum_length' => 0,
      'add_ellipsis' => FALSE,
      'wordsafe' => FALSE,
      'wordsafe_length' => 1,
      'permissions' => array(
        3 => 3,
      ),
      'link_to_object' => FALSE,
      'link_to_search' => FALSE,
    ),
  ),
  3 => array(
    'solr_field' => 'CCR_C-3646_60ef52ab-b400-cb07-7cc2-bda80ec72001',
    'field_type' => 'facet_fields',
    'weight' => -9,
    'solr_field_settings' => array(
      'label' => 'Category',
      'pid_object_label' => NULL,
      'permissions' => array(
        3 => 3,
      ),
    ),
  ),
  4 => array(
    'solr_field' => 'meertensCollection',
    'field_type' => 'result_fields',
    'weight' => -8,
    'solr_field_settings' => array(
      'label' => 'Type',
      'snippet' => NULL,
      'date_format' => '',
      'maximum_length' => 0,
      'add_ellipsis' => FALSE,
      'wordsafe' => FALSE,
      'wordsafe_length' => 1,
      'permissions' => array(
        3 => 3,
      ),
      'link_to_object' => FALSE,
      'link_to_search' => FALSE,
    ),
  ),
  5 => array(
    'solr_field' => 'CCR_C-5311_dd01255f-ebbf-20dc-f519-a17c90a59400',
    'field_type' => 'facet_fields',
    'weight' => -8,
    'solr_field_settings' => array(
      'label' => 'Subcategory',
      'pid_object_label' => NULL,
      'permissions' => array(
        3 => 3,
      ),
    ),
  ),
);
}
