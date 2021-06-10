<?php

/**
 * This class will sync template data stored in drupal's form_state input array
 * towards values array
 *
 * Best guess is that because templates are loaded using *_alter hook (which isn't called when form is submitted)
 * form_state input array contains the actual data but isn't moved to form_state values array because of the use of *_alter hook
 * so this hack is necessary
 *
 * @author  Ibrahim Abdullah <ibrahim.abdullah@mpi.nl>
 * @package FLAT CMDI Templates
 */
class CmdiValueExtractor
{
    /**
     * Syncing "input" of "template_container[elements]" to "values"
     *
     * @param array $form_state
     *
     * @return array
     */
    public static function extract($form_state)
    {
        $values = $form_state['values']['template_container']['elements'];
        $input = $form_state['input']['template_container']['elements'];

        return static::recursiveExtraction($values, $input);
    }

    /**
     * Recursively walk through values array to sync drupal form "input" values with "values"
     *
     * @param array $values
     * @param array $input
     *
     * @return array
     */
    public static function recursiveExtraction($values, $input)
    {
        $extracted = [];

        foreach ($values as $field => $innervalues) {

            if (is_array($innervalues)) {

                $innerinput = isset($input[$field]) ? $input[$field] : [];
                $extracted[$field] = static::recursiveExtraction($innervalues, $innerinput);

            } else {
                $extracted[$field] = isset($input[$field]) ? $input[$field] : $innervalues;
            }
        }

        return $extracted;
    }
}
