<?php

class CmdiDropdownBuilder
{
  public static function build($available_templates, $component_id)
  {
    return theme('flat_dropdown', [

      'component_id' => $component_id,
      'available_templates' => $available_templates,
    ]);
  }
}
