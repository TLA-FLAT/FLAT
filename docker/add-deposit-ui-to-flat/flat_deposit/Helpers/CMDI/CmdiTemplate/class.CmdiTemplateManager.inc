<?php

class CmdiTemplateManager
{
    public static function alter(&$form, &$form_state)
    {
        // if (isset($form_state['pressedButtons'])) {
        //     dfb('------- PRESSED BUTTONS ------ ');
        //     dfb($form_state['pressedButtons']);
        //     dfb('------- PRESSED BUTTONS ------ ');
        // }
        $form['flat_modal'] = CmdiModalBuilder::build();

        if (!isset($form_state['loaded_cmdi_templates'])) {
            $form_state['loaded_cmdi_templates'] = [];
        }

        // saving template blocks
        $saved = static::save($form_state);

        if (true === $saved) {

            // template was saved, 'load_template' element needs to be rebuilt
            $cmdi_tree = $form_state['triggering_element']['#cmdi_tree'];
            $profile = isset($form_state['complete form']['trigger']['select_profile_name']['#value']) ? $form_state['complete form']['trigger']['select_profile_name']['#value'] : (isset($form_state['complete form']['select_profile_name']['#value']) ? $form_state['complete form']['select_profile_name']['#value'] : null);
            $component_id = $form_state['triggering_element']['#component_id'];

            // rebuild 'load_template' element
            static::rebuild($form['template_container']['elements'], $cmdi_tree, $profile, $component_id);
        }

        $saved  = static::saveLoaded($form_state);
        $loaded = static::load($form['template_container']['elements'], $form_state);
    }

    public static function build($component_id, $profile, $cmdi_tree)
    {
        return CmdiTemplateBuilder::build($component_id, $profile, $cmdi_tree);
    }

    public static function buildActions($profile, $component_id, $cmdi_tree)
    {
        return CmdiActionsBuilder::build($profile, $component_id, $cmdi_tree);
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

    public static function load(&$elements, &$form_state)
    {
        return CmdiTemplateLoader::load($elements, $form_state);
    }
}

function flat_cmdi_templates_ajax_save_callback($form, &$form_state) {
    return $form['template_container'];
}

function flat_cmdi_templates_ajax_load_callback($form, &$form_state) {
    return $form['template_container'];
}
