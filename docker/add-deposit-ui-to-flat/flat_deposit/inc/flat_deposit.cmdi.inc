<?php


/**
 * Template session form. Can be populated by saved meta data (md). This is done by filling form default_values with data from a saved php-array.

 * @param $extra string path variable extending the ordinary registered path which will be used as additional variable in functions' logic
 * @return array
 */
function flat_deposit_cmdi()
{

    $node = menu_get_object();
    $wrapper = entity_metadata_wrapper('node', $node);

    $file_info = $wrapper->upload_cmdi->value();
    if ($file_info) {

        module_load_include('php', 'flat_deposit', 'Helpers/CMDI_Handler');
        $cmd = new CMDI_Handler($this->node);

        $message_chosen = t('The chosen profile is %profile. ', array('%profile' => $cmd->profile));

    } else {

        $message_chosen = 'No metadata file available for this bundle';
    }
    $content = array();

    $content['cmdi']['profile']['chosen'] = array(
        '#type' => 'markup',
        '#markup' => $message_chosen,
        '#prefix' => '<p>',
    );

    $content['cmdi']['profile']['change'] = array(
        '#theme' => 'link',
        '#text' => 'Change metadata profile',
        '#path' => 'node/' . $node->nid . '/cmdi/select',
        '#options' => array(
            'attributes' => array(),
            'html' => FALSE,
        ),
    );


    return $content;


}
