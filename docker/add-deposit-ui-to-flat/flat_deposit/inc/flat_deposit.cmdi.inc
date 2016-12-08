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

    $profile = $wrapper->upload_cmdi->value();

    $content = array();

    $content['cmdi']['profile']['chosen'] = array(
        '#type' => 'markup',
        '#markup' => t('The chosen profile is %profile. ', array('%profile' => $profile)),
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

    // check existence info file
    module_load_include('php', 'flat_deposit', 'Helpers/CMDI_Handler');
    $cmd = new CMDI_Handler($node);

    if (!$cmd->projectCmdiFileExists()) {
        $message = 'No metadata file found in bundle metadata directory. Please upload or create a file. ';
    } else {
        $message = 'Metadata file found in bundle metadata directory. ';
    }

    $content['cmdi']['file']['exist'] = array(
        '#type' => 'markup',
        '#markup' => t($message),
        '#prefix' => '<p>',
    );

    if ($cmd->projectCmdiFileExists()) {
        if (!file_exists(drupal_realpath('public://tmp'))) drupal_mkdir('public://tmp');

        $URI = 'public://tmp/' .  $node->title . '_' . $node->nid . '_record.cmdi';
        file_unmanaged_copy($cmd->export_file, $URI, FILE_EXISTS_REPLACE);
/*
        if (file_exists($URI)) {
            // Serve file download.
            drupal_add_http_header('Pragma', 'public');
            drupal_add_http_header('Expires', '0');
            drupal_add_http_header('Cache-Control', 'must-revalidate, post-check=0, pre-check=0');
            drupal_add_http_header('Content-Type', 'application/xml');
            drupal_add_http_header('Content-Disposition', 'attachment; filename=' . basename($URL) . ';');
            drupal_add_http_header('Content-Transfer-Encoding', 'binary');
            drupal_add_http_header('Content-Length', filesize($URL));
            readfile($URL);
            unlink($URL);
            drupal_exit();
        }


*/


        $content['cmdi']['file']['link'] = array(
            '#theme' => 'link',
            '#text' => 'Link to metadata. ',
            '#path' => file_create_url($URI),
            '#options' => array(
                'attributes' => array(),
                'html' => FALSE,
            ),
        );



    }

    return $content;


}
