<?php
/**
 * @file
 * The primary PHP file for this theme.
 */

/**
 * Theme function for the compound navigation block parts.
 */
function flat_bootstrap_theme_preprocess_islandora_compound_prev_next(array &$variables) {
  error_log("!MENZO: flat_bootstrap_theme_preprocess_islandora_compound_prev_next");
  if (($variables['child_count'] > 0 && !$variables['parent_tn']) || ($variables['child_count'] > 0 && $variables['parent_tn'])) {
    for ($i = 0; $i < count($variables['siblings']); $i += 1) {
      $sibling = array();
      $sibling['pid'] = $variables['siblings'][$i];
      $sibling['class'] = array();
      if ($sibling['pid'] === $variables['pid']) {
        $sibling['class'][] = 'active';
      }
      $sibling_object = islandora_object_load($sibling['pid']);
      if (isset($sibling_object['TN']) && islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $sibling_object['TN'])) {
        $sibling['TN'] = 'islandora/object/' . $sibling['pid'] . '/datastream/TN/view';
      }
      else {
        // Object either does not have a thumbnail or it's restricted show a
        // default in its place.
        $sibling['TN'] = $folder_image_path;
      }
      $sibling['label'] = $sibling_object->label;
      $themed_siblings[] = $sibling;
    }
  }
  $variables['themed_siblings'] = $themed_siblings;
}
