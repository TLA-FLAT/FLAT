<?php

class CmdiModalBuilder
{
    public static function build()
    {
      static::registerJs();

      return [
        '#markup' => static::modal(),
      ];
    }

    public static function registerJs()
    {
      // adding modal support
      $module_path = drupal_get_path('module', 'flat_deposit');

      drupal_add_js($module_path . '/js/flat-modal.js');
      drupal_add_css($module_path . '/css/flat-modal.css');

      // adding templates
      drupal_add_js([

        'flat_modal_blank' => static::blank(),
        'flat_modal_loader' => static::loader(),
        'flat_modal_confirm_delete' => static::confirmDelete(),

      ], 'setting');
    }

    public static function modal()
    {
      return theme('flat_modal');
    }

    public static function blank()
    {
      return theme('flat_modal_blank');
    }

    public static function loader()
    {
      return theme('flat_modal_loader');
    }

    public static function error()
    {
      return theme('flat_modal_error');
    }

    public static function confirm($cmdi_id)
    {
      return theme('flat_modal_confirm', [

        'cmdi_id' => $cmdi_id,
        'title' => 'Provided label already exists',
        'description' => 'A preset with this label already exists. Would you like to replace it?',
      ]);
    }

    public static function success()
    {
      return theme('flat_modal_success', [

        'title' => 'Success',
        'description' => 'Your preset was successfully saved.',
      ]);
    }

    public static function confirmDelete()
    {
      return theme('flat_modal_confirm_delete', [

        'title' => 'Are you sure?',
        'description' => 'Are you sure you want to delete this preset? This action is permanent!',
        'url' => url('flat_deposit/delete_cmdi_template'),
      ]);
    }

    public static function successDelete()
    {
      return theme('flat_modal_success', [

        'title' => 'Success',
        'description' => 'Your preset was successfully deleted.',
      ]);
    }
}
