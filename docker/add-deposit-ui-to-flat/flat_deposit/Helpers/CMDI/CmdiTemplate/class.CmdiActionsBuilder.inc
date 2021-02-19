<?php

class CmdiActionsBuilder
{
  public static function build($profile, $component_id, $cmdi_tree)
  {
    $cmdi_id = implode('_', $cmdi_tree);
    $available_templates = CmdiTemplateData::getAvailableTemplates($profile, $component_id);

    return theme('flat_actions', [

      'cmdi_id' => $cmdi_id,
      'component_id' => $component_id,
      'available_templates' => $available_templates,
      'cmdi_data' => json_encode([

        'cmdi_id' => $cmdi_id,
        'profile' => $profile,
        'component_id' => $component_id,
        'url' => url('flat_deposit/save_check'),
        'label_name' => 'cmdi_new_template_label_' . $cmdi_id,
        'save_name' => 'save_cmdi_template_' . $cmdi_id,
      ]),
    ]);
  }
}
