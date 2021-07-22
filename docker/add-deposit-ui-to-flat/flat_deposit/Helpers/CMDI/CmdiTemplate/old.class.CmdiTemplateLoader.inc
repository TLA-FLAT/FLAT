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

    public static function loadElement(&$element, $cmdi_tree, &$block)
    {
        if (count($cmdi_tree) > 0) {

            $nextKey = array_shift($cmdi_tree);

            if (isset($element[$nextKey])) {
                static::loadElement($element[$nextKey], $cmdi_tree, $block);
            }

        } else {

            $copy = $block['element'];
            $metadata = $copy['#flat_cmdi_metadata'];

            $copy['flat_cmdi_templates'] = CmdiTemplateManager::build($metadata['component_id'], $metadata['profile'], $metadata['tree']);
            $copy['#panel_actions'] = CmdiTemplateManager::buildActions($metadata['profile'], $metadata['component_id'], $metadata['tree']);

            $element = $copy;
        }
    }

    public static function load(&$elements, &$form_state)
    {
        if (count($form_state['loaded_cmdi_templates']) > 0) {

            foreach ($form_state['loaded_cmdi_templates'] as $loadable) {

                $block = static::getLoadedBlock($loadable['id']);

                if (false !== $block) {
                    // $tree = array_merge(['template_container', 'elements'], $loadable['cmdi_tree']);
                    // dfb($loadable);
                    // dfb($tree);
                    // dfb($form_state['pressedButtons']);
                    if (isset($form_state['pressedButtons'])) {

                        # when block gets loaded, we need to reset the pressedButtons state
                        # back to null to prevent FormBuilder from rebuilding previous form states
                        // $tree = array_merge(['template_container', 'elements'], $loadable['cmdi_tree']);
                        // dfb($tree);
                        // dfb($form_state['input']['template_container']);
                        // dfb($form_state['input']['template_container']['elements']['Contributors']);
                        // dfb($form_state['values']['template_container']);
                        // dfb($form_state['values']['template_container']['elements']['Contributors']);
                        // drupal_array_set_nested_value($form_state['pressedButtons'], $tree, null);
                        // dfb($form_state['pressedButtons']);
                        // drupal_array_set_nested_value($form_state['pressedButtons'], $tree, []);
                    }

                    static::loadElement($elements, $loadable['cmdi_tree'], $block);
                }
            }

            $form_state['loaded_cmdi_templates'] = [];
        }
    }
}
