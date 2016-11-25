<?php


/**
 * Template session form. Can be populated by saved meta data (md). This is done by filling form default_values with data from a saved php-array.

 * @param $extra string path variable extending the ordinary registered path which will be used as additional variable in functions' logic
 * @return array
 */
function flat_deposit_cmdi(){

    $node = menu_get_object();
    $wrapper = entity_metadata_wrapper('node',$node);

    $profile = $wrapper->upload_cmdi->value();

    // check existence info file
    module_load_include('php','flat_deposit','Helpers/CMDI_Handler');
    $cmd = new CMDI_Handler($node);

    if(!$cmd->projectCmdiFileExists()) {

        drupal_set_message(t('No info file found. Please upload a file or create a new one'));
        drupal_goto('node/' . $node->nid . '/cmdi/select');

    } elseif(!$profile) {

        drupal_set_message(t('Profile has not been set please set.'));
        drupal_goto('node/' . $node->nid . '/cmdi/select');


    } else{

    //TODO add edit option for existing cmdi's
    return 'hello';
    }

}
