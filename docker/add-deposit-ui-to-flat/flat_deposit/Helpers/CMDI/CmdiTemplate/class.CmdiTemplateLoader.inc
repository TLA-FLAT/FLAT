<?php

class CmdiTemplateLoader
{
    public static function getLoadedBlock($id)
    {
        global $user;
        $row = CmdiTemplateDb::fetch($id, $user->uid);

        if (false === $row) {

            // template not found
            return false;
        }

        $block = json_decode($row['block'], true);

        if (null === $block) {

            // json could not be decoded
            return false;
        }

        return $block;
    }

    public static function load(&$form_state)
    {
        $isSubmitted = isset($form_state['triggering_element']['#ajax']['callback']) && $form_state['triggering_element']['#ajax']['callback'] === 'flat_cmdi_templates_ajax_load_callback';

        if (false === $isSubmitted) {
            return false;
        }

        $id = (int)$form_state['triggering_element']['#value'];
        $target_tree = $form_state['triggering_element']['#cmdi_tree'];
        $block = static::getLoadedBlock($id);

        dfb($block);
        if (false !== $block) {

            // loading block into form_state input array will trigger the formbuilder
            // to create the necessary elements
            drupal_array_set_nested_value($form_state['input']['template_container']['elements'], $target_tree, $block['state']);

            if (null !== $block['pressed']) {

                // create pressedButtons form state if it doesn't exist
                if (!isset($form_state['pressedButtons']['template_container']['elements'])) {
                    $form_state['pressedButtons']['template_container']['elements'] = [];
                }

                dfb('before');
                dfb($form_state['pressedButtons']);
                // create pressedButtons tree in form state pressedButtons for saved block
                drupal_array_set_nested_value($form_state['pressedButtons']['template_container']['elements'], $target_tree, $block['pressed']);
                dfb($form_state['pressedButtons']);
                dfb('after');
            }
            // if (!isset($form_state['pressedButtons']))

            // set pressedButtons for loaded element
            // drupal_set_nested_value($form_state['pressedButtons']);



            // loading pressedButtons state into target tree
            // if (!isset($form_state['pressedButtons']['template_container']['elements'])) {
            //     $form_state['pressedButtons']['template_container']['elements'] = [];
            // }

            // drupal_array_set_nested_value($form_state['pressedButtons']['template_container']['elements'], $block['tree'], $block['pressed']);
            // dfb($block['pressed']);
            // dfb($form_state['pressedButtons']);

            // dfb($form_state['pressedButtons']);
            // if (isset($form_state['pressedButtons'])) {

                // dfb($tree);
                // dfb($form_state['input']['template_container']);
                // dfb($form_state['input']['template_container']['elements']['Contributors']);
                // dfb($form_state['values']['template_container']);
                // dfb($form_state['values']['template_container']['elements']['Contributors']);
                // drupal_array_set_nested_value($form_state['pressedButtons'], $tree, null);
                // dfb($form_state['pressedButtons']);
                // drupal_array_set_nested_value($form_state['pressedButtons'], $tree, []);
            // }
        }
    }
}
