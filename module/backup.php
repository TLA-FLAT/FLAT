<?php
/**
 * Created by PhpStorm.
 * User: danrhe
 * Date: 08/02/16
 * Time: 12:21
 */


/**
 * Implements hook_theme().
 */
function flat_deposit_ui_theme($existing, $type, $theme, $path)
{
    return array(
        'flat_deposit_ui_piclink' => array(
            'variables' => array(
                'input1' => '',
            ),
        ),
    );
}