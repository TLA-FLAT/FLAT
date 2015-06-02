<?php

/**
 * @file
 * Implements a simple class for working with Cmdi data.
 *
 * Inspiration and design shamelessly stolen from the pyfedora
 * project at http://pypi.python.org/pypi/pyfedora/0.1.0
 */

/**
 * Cmdi Class
 */
class Cmdi {

  public $cmdi = array(
    'cmdi:header' => array(),
    'cmdi:Resources' => array(),
    'cmdi:Components' => array(),
  );
  public $owner;

  /**
   * Constructor.
   *
   * @param string $cmdi_xml
   *   The Cmdi XML.
   */
  public function __construct($cmdi_xml = NULL) {
    if (!empty($cmdi_xml)) {
      $this->cmdi = self::importFromXMLString($cmdi_xml);
    }
  }

  /**
   * Add an element.
   *
   * @param string $element_name
   *   The name of the element to add.
   * @param string $value
   *   The value of the element to add.
   */
  public function addElement($element_name, $value) {
    if (is_string($value) && is_array($this->cmdi[$element_name])) {
      $this->cmdi[$element_name][] = $value;
    }
  }

  /**
   * Replace the given Cmdi element with the given values.
   *
   * @param string $element_name
   *   The name of the elements to set.
   * @param mixed $values
   *   The values of the set the elements too.
   */
  public function setElement($element_name, $values) {
    if (is_array($values)) {
      $this->cmdi[$element_name] = $values;
    }
    elseif (is_string($values)) {
      $this->cmdi[$element_name] = array($values);
    }
  }

  /**
   * Serialize this object as an XML string.
   *
   * @return string
   *   The serialized XML.
   */
  public function asXML() {
    $cmdi_xml = new DomDocument();
  /**
    * $oai_cmdi = $cmdi_xml->createElementNS('http://www.openarchives.org/OAI/2.0/oai_cmdi/', 'oai_cmdi:cmdi');
    * $oai_cmdi->setAttribute('xmlns:cmdi', 'http://purl.org/cmdi/elements/1.1/');
    * foreach ($this->cmdi as $cmdi_element => $values) {
    *   if (is_array($values) && !empty($values)) {
    *     foreach ($values as $value) {
    *       $new_item = $cmdi_xml->createElement($cmdi_element, $value);
    *       $oai_cmdi->appendchild($new_item);
    *    }
    *  }
    *  else {
    *    $new_item = $cmdi_xml->createElement($cmdi_element);
    *    $oai_cmdi->appendchild($new_item);
    *  }
    *}
    *$cmdi_xml->appendchild($oai_cmdi);
   */
    return $cmdi_xml->saveXML();
  }

  /**
   * Serializes this object as an array.
   *
   * @return array
   *   The serialized object.
   */
  public function asArray() {
    $cmdi_array = array();
    foreach ($this as $element) {
      if (!empty($element)) {
        foreach ($element as $field => $values) {
          // Split value if the result value is an array.
          if (is_array($values)) {
            $value = '';
            $i = 0;
            foreach ($values as $piece) {
              if (!empty($piece)) {
                if ($i++) {
                  $value .= ", ";
                }
                $value .= $piece;
              }
            }
          }
          else {
            $value = $values;
          }
          $cmdi_label = explode(':', $field);
          $element_label = drupal_ucfirst($cmdi_label[1]);
          $i18n_object_id = drupal_strtolower($element_label);
          $cmdi_array[$field]['label'] = function_exists('i18n_string') ?
            i18n_string("islandora:cmdi:{$i18n_object_id}:label", $element_label) :
            $element_label;
          $cmdi_array[$field]['value'] = filter_xss($value);
          $cmdi_array[$field]['class'] = drupal_strtolower(preg_replace('/[^A-Za-z0-9]/', '-', $field));
          $cmdi_array[$field]['cmditerms'] = preg_replace('/^cmdi/', 'cmditerms', $field);
        }
      }
    }
    return $cmdi_array;
  }

  /**
   * Creates a new instance of the class by parsing cmdi_xml.
   *
   * @param string $cmdi_xml
   *   Cmdi XML.
   *
   * @return Cmdi
   *   The instantiated object.
   */
  public static function importFromXMLString($cmdi_xml) {
    $cmdi_doc = new DomDocument();
    if (!empty($cmdi_xml) && $cmdi_doc->loadXML($cmdi_xml)) {
   //   $oai_cmdi = $cmdi_doc->getElementsByTagNameNS('http://purl.org/cmdi/elements/1.1/', '*');
      $new_cmdi = new Cmdi();
      foreach ($oai_cmdi as $child) {
        if (isset($new_cmdi->cmdi[$child->nodeName])) {
          array_push($new_cmdi->cmdi[$child->nodeName], $child->nodeValue);
        }
      }
      return $new_cmdi;
    }
    return NULL;
  }

}