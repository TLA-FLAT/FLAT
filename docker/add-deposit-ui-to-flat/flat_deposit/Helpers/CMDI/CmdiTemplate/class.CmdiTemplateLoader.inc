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
                    static::loadElement($elements, $loadable['cmdi_tree'], $block);
                }
            }
        }

        $form_state['loaded_cmdi_templates'] = [];
    }
}
