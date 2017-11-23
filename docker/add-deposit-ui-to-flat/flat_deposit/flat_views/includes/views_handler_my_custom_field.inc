<?php
/**
 * @file
 * Custom views handler definition.
 *
 * Place this code in
 * /sites/all/[custom_module_name]/includes/views_handler_my_custom_field.inc
 */

/**
 * Custom handler class.
 *
 * @ingroup views_field_handlers
 */
class views_handler_my_custom_field extends views_handler_field {
    /**
     * {@inheritdoc}
     *
     * Perform any database or cache data retrieval here. In this example there is
     * none.
     */
    function query() {

    }

    /**
     * {@inheritdoc}
     *
     * Modify any end user views settings here. Debug $options to view the field
     * settings you can change.
     */
    function option_definition() {
        $options = parent::option_definition();
        return $options;
    }

    /**
     * {@inheritdoc}
     *
     * Make changes to the field settings form seen by the end user when adding
     * your field.
     */
    function options_form(&$form, &$form_state) {
        parent::options_form($form, $form_state);
    }

    /**
     * Render callback handler.
     *
     * Return the markup that will appear in the rendered field.
     */
    function render($values) {

        return l('Edit', 'node/' . $values->nid);
    }
}