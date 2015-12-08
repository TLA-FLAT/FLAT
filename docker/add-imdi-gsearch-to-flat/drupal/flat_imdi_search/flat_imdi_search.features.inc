<?php
/**
 * @file
 * flat_imdi_search.features.inc
 */

/**
 * Implements hook_islandora_solr_fields_features_default_fields().
 */
function flat_imdi_search_islandora_solr_fields_features_default_fields() {
  return array(
  0 => array(
    'solr_field' => 'cmd.title',
    'field_type' => 'facet_fields',
    'weight' => -10,
    'solr_field_settings' => array(
      'label' => 'Title',
      'pid_object_label' => NULL,
      'permissions' => array(
        3 => 3,
      ),
    ),
  ),
  1 => array(
    'solr_field' => 'cmd.title',
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
      'link_to_object' => FALSE,
      'link_to_search' => FALSE,
    ),
  ),
  2 => array(
    'solr_field' => 'cmd.Subject',
    'field_type' => 'result_fields',
    'weight' => -9,
    'solr_field_settings' => array(
      'label' => 'Subject',
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
    'solr_field' => 'cmd.Subject',
    'field_type' => 'facet_fields',
    'weight' => -9,
    'solr_field_settings' => array(
      'label' => 'Subject',
      'pid_object_label' => NULL,
      'permissions' => array(
        3 => 3,
      ),
    ),
  ),
  4 => array(
    'solr_field' => 'cmd.Language',
    'field_type' => 'result_fields',
    'weight' => -8,
    'solr_field_settings' => array(
      'label' => 'Language',
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
    'solr_field' => 'cmd.Language',
    'field_type' => 'facet_fields',
    'weight' => -8,
    'solr_field_settings' => array(
      'label' => 'Language',
      'pid_object_label' => NULL,
      'permissions' => array(
        3 => 3,
      ),
    ),
  ),
  6 => array(
    'solr_field' => 'cmd.notes',
    'field_type' => 'facet_fields',
    'weight' => -7,
    'solr_field_settings' => array(
      'label' => 'Notes',
      'pid_object_label' => NULL,
      'permissions' => array(
        3 => 3,
      ),
    ),
  ),
  7 => array(
    'solr_field' => 'cmd.Country',
    'field_type' => 'result_fields',
    'weight' => -7,
    'solr_field_settings' => array(
      'label' => 'Country',
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
  8 => array(
    'solr_field' => 'cmd.notes',
    'field_type' => 'result_fields',
    'weight' => -6,
    'solr_field_settings' => array(
      'label' => 'Notes',
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
  9 => array(
    'solr_field' => 'cmd.Format',
    'field_type' => 'facet_fields',
    'weight' => -6,
    'solr_field_settings' => array(
      'label' => 'Format',
      'pid_object_label' => NULL,
      'permissions' => array(
        3 => 3,
      ),
    ),
  ),
  10 => array(
    'solr_field' => 'cmd.Country',
    'field_type' => 'facet_fields',
    'weight' => -5,
    'solr_field_settings' => array(
      'label' => 'Country',
      'pid_object_label' => NULL,
      'permissions' => array(
        3 => 3,
      ),
    ),
  ),
  11 => array(
    'solr_field' => 'cmd.url',
    'field_type' => 'result_fields',
    'weight' => -5,
    'solr_field_settings' => array(
      'label' => 'URL',
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
  12 => array(
    'solr_field' => 'cmd.Format',
    'field_type' => 'result_fields',
    'weight' => -4,
    'solr_field_settings' => array(
      'label' => 'Format',
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
  13 => array(
    'solr_field' => 'cmd.DataProvider',
    'field_type' => 'facet_fields',
    'weight' => -4,
    'solr_field_settings' => array(
      'label' => 'Data Provider',
      'pid_object_label' => NULL,
      'permissions' => array(
        3 => 3,
      ),
    ),
  ),
  14 => array(
    'solr_field' => 'cmd.CreationDate',
    'field_type' => 'result_fields',
    'weight' => -3,
    'solr_field_settings' => array(
      'label' => 'Creation Date',
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
  15 => array(
    'solr_field' => 'cmd.CreationDate',
    'field_type' => 'facet_fields',
    'weight' => -3,
    'solr_field_settings' => array(
      'label' => 'Creation Date',
      'pid_object_label' => NULL,
      'permissions' => array(
        3 => 3,
      ),
    ),
  ),
  16 => array(
    'solr_field' => 'cmd.PublicationDate',
    'field_type' => 'facet_fields',
    'weight' => -2,
    'solr_field_settings' => array(
      'label' => 'Publication Date',
      'pid_object_label' => NULL,
      'permissions' => array(
        3 => 3,
      ),
    ),
  ),
  17 => array(
    'solr_field' => 'cmd.PublicationDate',
    'field_type' => 'result_fields',
    'weight' => -2,
    'solr_field_settings' => array(
      'label' => 'Publication Date',
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
  18 => array(
    'solr_field' => 'cmd.DataProvider',
    'field_type' => 'result_fields',
    'weight' => -1,
    'solr_field_settings' => array(
      'label' => 'Data Provider',
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
);
}
