<?php

class CmdiTemplateBuilder
{
    public static function build($component_id, $profile, $cmdi_tree)
    {
        $id = implode('_', $cmdi_tree);
        $label_field_name = 'cmdi_new_template_label_' . $id;

        $fields = [
        ];

        $fields['load_template'] = static::buildLoadTemplateElement($profile, $component_id);
        $fields['load_template']['#cmdi_tree'] = $cmdi_tree;

        $fields['new_template'] = [

            'label' => [

                '#type' => 'textfield',
                '#name' => $label_field_name,
                '#default_value' => '',
                '#attributes' => [
                    'class' => ['hide'],
                    'placeholder' => 'Label of new template' . $id,
                ],
            ],

            'save' => [

                '#type' => 'button',
                '#name' => 'save_cmdi_template_' . $id,
                '#value' => 'Save as template',
                '#flat_ignore_button' => true,
                '#cmdi_tree' => $cmdi_tree,
                '#component_id' => $component_id,
                '#label_field_name' => $label_field_name,
                '#limit_validation_errors' => [],
                '#attributes' => [
                    'class' => ['hide'],
                ],
                '#ajax' => [

                    'callback' => 'flat_cmdi_templates_ajax_save_callback',
                    'method' => 'replace',
                    'effect' => 'fade',
                    'wrapper' => 'template-form',
                    'event' => 'saving_' . $id,
                ],
            ],
        ];

        return $fields;
    }

    public static function buildLoadTemplateElement($profile, $component_id)
    {
        $available_templates = CmdiTemplateData::getAvailableTemplates($profile, $component_id);

        if (count($available_templates) === 0) {
            return static::noTemplatesFound();
        }

        return [

          '#type' => 'select',
          '#options' => $available_templates,
          '#attributes' => [
              'class' => ['hide'],
            'data-role' => 'flat-cmdi-template-loadable-' . $component_id,
          ],
          '#component_id' => $component_id,
          '#ajax' => [

              'callback' => 'flat_cmdi_templates_ajax_load_callback',
              'method' => 'replace',
              'effect' => 'fade',
              'wrapper' => 'template-form',
          ],
        ];
    }

    public static function buildTemplateBlock($element)
    {
        if (!isset($element['#cmdi_template_fields']) || count($element['#cmdi_template_fields']) === 0) {
            return null;
        }

        $fields = $element['#cmdi_template_fields'];
        $block = [];

        if (isset($element['#value']) && $element['#value'] !== null) {
            $block['#value'] = $element['#value'];
        }

        foreach ($fields as $field) {

            if (isset($element[$field]['#cmdi_template_fields'])) {

                if (count($element[$field]['#cmdi_template_fields']) > 0) {
                    $block[$field] = static::buildTemplateBlock($element[$field]);
                }

            } else {
                $block[$field] = $element[$field];
            }
        }

        $block['#cmdi_template_fields'] = $fields;
        return $block;
    }

    private static function noTemplatesFound()
    {
        return [
            '#markup' => '<strong>No templates available for this block</strong><br /><br />',
        ];
    }
}
