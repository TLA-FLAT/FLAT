<?php

/**
 * This class will sync modified form data when template has been loaded in form, even after
 * form generates form error
 *
 * Best guess is that because templates are loaded using *_alter hook (which isn't called when form is validated)
 * but ajax_handler *is* called, so we can sync the form data
 *
 * @author  Ibrahim Abdullah <ibrahim.abdullah@mpi.nl>
 * @package FLAT CMDI Templates
 */
class CmdiValueSyncer
{
    public static function sync(&$form, &$form_state)
    {
        if (!isset($form['template_container'])) {
            return;
        }

        $elements = &$form['template_container']['elements'];
        $input = &$form_state['input']['template_container']['elements'];

        static::recursiveExtraction($input, $elements);
    }

    /**
     * Recursively walk through input array to sync drupal form "input" values with "form[#value]"
     *
     * @param array $input
     * @param array $elements
     *
     * @return void
     */
    public static function recursiveExtraction($input, &$elements)
    {
        foreach ($input as $field => $innerinput) {

            if (is_array($innerinput)) {

                $innerelements = &$elements[$field];
                static::recursiveExtraction($innerinput, $innerelements);

            } else {

                // found value of field
                $elements[$field]['#value'] = $innerinput;
            }
        }
    }
}
