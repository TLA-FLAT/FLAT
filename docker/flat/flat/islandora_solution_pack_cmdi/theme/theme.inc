<?php
/**
 * @file
 * This file contains all theme and preprocess functions for cmdi.
 */


/**
 * Implements module_preprocess_hook().
 */
function islandora_cmdi_preprocess_islandora_cmdi_display(array &$variables) {
  $object = $variables['islandora_object'];
  $cmd = FALSE;
  if (isset($object['CMD']) && islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $object['CMD'])) {
    try {
      $cmd = $object['CMD']->content;
    }
    catch (Exception $e) {
      drupal_set_message(t('Error retrieving object %s %t', array('%s' => $object->id, '%t' => $e->getMessage())), 'error', FALSE);
    }
  }
  // Using the Islandora 6 object details XSLT let's transform the CMD datastream
  // to HTML output.
  if ($cmd) {
    $input = new DOMDocument();
    $did_load_ok = $input->loadXML($cmd);
    if ($did_load_ok) {
      $xpath = new DOMXPath($input);
      $xpath->registerNamespace('cmd', "http://www.clarin.eu/cmd/");
      $profile_id = $xpath->query('/cmd:CMD/cmd:Header/cmd:MdProfile');
      $profile_str = $profile_id->item(0)->nodeValue;
      $profile_str = preg_replace("/[^a-zA-Z0-9]/","_",$profile_str, -1,$count);
    }

    $file = drupal_get_path('module', 'islandora_cmdi') . '/xsl/' . $profile_str . '.xsl';
    if (!file_exists($file)) {
        $file = drupal_get_path('module', 'islandora_cmdi') . '/xsl/browser_cmdi2html.xsl';
    }
    $xslt = new XSLTProcessor();
    $xslt_dom = new DOMDocument();
    $xslt_dom->load($file);
    if ($did_load_ok) {
      $xslt->importStylesheet($xslt_dom);
      $variables['metadata'] = $xslt->transformToXml($input);
    }
  }
}
