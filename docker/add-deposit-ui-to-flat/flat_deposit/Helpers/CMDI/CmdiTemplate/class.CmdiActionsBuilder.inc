<?php

class CmdiActionsBuilder
{
  public static function build($profile, $component_id)
  {
    $available_templates = CmdiTemplateData::getAvailableTemplates($profile, $component_id);

    return theme('flat_actions', [

      'component_id' => $component_id,
      'available_templates' => $available_templates,
    ]);
  }
}
