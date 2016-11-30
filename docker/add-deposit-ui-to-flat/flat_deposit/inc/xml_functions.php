<?php

/**
 * Class SimpleXMLElement_Plus is an extended simpleXML class that  allows to include processing instructions
 */
class SimpleXMLElement_Plus extends SimpleXMLElement {

    /**
     * Function that allows including processing instructions into exportable xml object.
     * @param $name
     * @param $value
     */
    public function addProcessingInstruction( $name, $value )
    {
        // Create a DomElement from this simpleXML object
        $dom_sxe = dom_import_simplexml($this);

        // Create a handle to the owner doc of this xml
        $dom_parent = $dom_sxe->ownerDocument;

        // Find the topmost element of the domDocument
        $xpath = new DOMXPath($dom_parent);
        $first_element = $xpath->evaluate('/*[1]')->item(0);

        // Add the processing instruction before the topmost element
        $pi = $dom_parent->createProcessingInstruction($name, $value);
        $dom_parent->insertBefore($pi, $first_element);
    }
}

function add_attribute_tree_to_xml($data, &$xml_data ){

    foreach( $data as $key => $value ) {
        if( is_array($value) ){
            $subnode = $xml_data->$key;
            add_attribute_tree_to_xml($value, $subnode );
        } else {
            $xml_data->addAttribute($key ,$value);
        }
    }
}


/**
 * function definition to convert an array to xml. Don't use for attributes, use add_attribute_tree_to_xml instead
 *
 * @param $data php array
 * @param $xml_data simplexml object for which new child branches are created
 */
function array_to_xml( $data, &$xml_data ) {
    foreach( $data as $key => $value ) {
        if( is_array($value) ) {
            if( is_numeric($key) ){
                $key = 'item'. $key; //dealing with <0/>..<n/> issues
            }
            $subnode = $xml_data->addChild($key);
            array_to_xml($value, $subnode);
        } else {
            $xml_data->addChild("$key",htmlspecialchars("$value"));
        }
    }
}

// function definition to convert array to xml
function array_to_xml_original ( $data, &$xml_data ) {
    foreach( $data as $key => $value ) {
        if( is_array($value) ) {
            if( is_numeric($key) ){
                $key = 'item'. $key; //dealing with <0/>..<n/> issues
            }
            $subnode = $xml_data->addChild($key);
            array_to_xml($value, $subnode);
        } else {
            $xml_data->addChild("$key",htmlspecialchars("$value"));
        }
    }
}
