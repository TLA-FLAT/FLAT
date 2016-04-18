<?php
/**
 * Created by PhpStorm.
 * User: danrhe
 * Date: 18/04/16
 * Time: 17:10
 */
function flat_deposit_set_paths()
{
    $freeze_path = variable_get('flat_deposit_freeze_path', array());

    //already installed
    if (!empty($freeze_path[0])) {
        return TRUE;
    }
    variable_set('flat_deposit_freeze_path', $freeze_path);

    return TRUE;
}