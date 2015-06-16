<?php
/**
 * @file
 * easylat.features.fe_block_boxes.inc
 */

/**
 * Implements hook_default_fe_block_boxes().
 */
function easylat_default_fe_block_boxes() {
  $export = array();

  $fe_block_boxes = new stdClass();
  $fe_block_boxes->info = 'Developed by TLA';
  $fe_block_boxes->format = 'filtered_html';
  $fe_block_boxes->machine_name = 'easylat';
  $fe_block_boxes->body = 'Developed by The Language Archive (a cooperation between KNAW and MPI).';

  $export['easylat'] = $fe_block_boxes;

  return $export;
}
