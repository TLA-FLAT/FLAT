<?php

class CmdiTemplateRebuilder
{
    public static function rebuild(&$element, $cmdi_tree, $profile, $component_id)
    {
        if (count($cmdi_tree) > 0) {

            $nextKey = array_shift($cmdi_tree);

            if (isset($element[$nextKey])) {
                static::rebuild($element[$nextKey], $cmdi_tree, $profile, $component_id);
            }

        } else {

            if (isset($element['flat_cmdi_templates']['load_template'])) {

                $currentCmdiTree = $element['flat_cmdi_templates']['load_template']['#cmdi_tree'];
                $element['flat_cmdi_templates']['load_template'] = CmdiTemplateBuilder::buildLoadTemplateElement($profile, $component_id);
                $element['flat_cmdi_templates']['load_template']['#cmdi_tree'] = $currentCmdiTree;
                $element['#panel_actions'] = CmdiTemplateManager::buildActions($profile, $component_id, $currentCmdiTree);
            }
        }
    }
}
