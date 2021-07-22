<?php

class CmdiTemplateManager
{
    public static function build($component_id, $profile, $cmdi_tree)
    {
        return CmdiTemplateBuilder::build($component_id, $profile, $cmdi_tree);
    }

    public static function buildActions($profile, $component_id, $cmdi_tree)
    {
        return CmdiActionsBuilder::build($profile, $component_id, $cmdi_tree);
    }

    public static function modal()
    {
        return CmdiModalBuilder::build();
    }

    public static function rebuild(&$elements, $cmdi_tree, $profile, $component_id)
    {
        // clearing cache
        CmdiTemplateData::clear();

        // and rebuilding 'load_template' element
        CmdiTemplateRebuilder::rebuild($elements, $cmdi_tree, $profile, $component_id);
    }

    public static function save(&$form_state): bool
    {
        $saver = new CmdiTemplateSaver($form_state);
        return $saver->save();
    }

    public static function saveLoaded(&$form_state)
    {
        return CmdiTemplateSaveLoaded::saveLoaded($form_state);
    }

    public static function load(&$form_state)
    {
        return CmdiTemplateLoader::load($form_state);
    }
}

function flat_cmdi_templates_ajax_save_callback($form, &$form_state) {
    return $form['template_container'];
}

function flat_cmdi_templates_ajax_load_callback($form, &$form_state) {
    return $form['template_container'];
}
