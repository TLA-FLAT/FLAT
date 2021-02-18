<?php

class CmdiActionsBuilder
{
  public static function build($available_templates, $component_id)
  {
    return theme('flat_actions', [

      'component_id' => $component_id,
      'available_templates' => $available_templates,
    ]);
  }
}
