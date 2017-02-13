<?php

/**
 * @file
 * Views hooks and callback registries.
 */

/**
 * Implements hook_views_data().
 */


/**
 * implement hook_views_data_alter
 * */
function flat_views_views_data_alter(&$data) {

  // changes the data that will be rendered
  $data['flat_supercollection']['actions'] = array(
      'title' => t('Action buttons'),
      'help' => t('Produces links to actions a user may perform on a collection entity.'),
      'field' => array(
          'handler' => 'flat_views_views_handler_field_actions',
          'group' => 'Collection',
          'click sortable' => FALSE,
      ),
  );
  $data['flat_supercollection']['date_created'] = array(
      'title' => t('Created date'),
      'help' => t('The date the relationship was created.'),
      'field' => array(
          'handler' => 'views_handler_field_date',
          'click sortable' => TRUE,
      ),
      'sort' => array(
          'handler' => 'views_handler_sort_date',
      ),
      'filter' => array(
          'handler' => 'views_handler_filter_date',
      ),
  );
}

