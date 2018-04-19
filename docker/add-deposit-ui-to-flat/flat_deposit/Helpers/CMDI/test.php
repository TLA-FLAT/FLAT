<?php
/*
phpinfo();
exit;
*/
/**
 * Created by PhpStorm.
 * User: danrhe
 * Date: 09/05/2017
 * Time: 11:42
 */

$drupal_path = '/easylat/www/htdocs/drupal';
chdir($drupal_path);
define('DRUPAL_ROOT', getcwd()); //the most important line
require_once './includes/bootstrap.inc';
drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);


/*
$fields['flat_deleted_resources'] = array(
    'field_name' => 'flat_deleted_resources',
    'type' => 'text',
    'cardinality' => 1,
    'settings' => array(
        'max_length' => 5000)
);
field_create_field($fields['flat_deleted_resources'] );
*/
$instances['flat_deleted_resources'] = array(
    'field_name' => 'flat_deleted_resources',
    'bundle' => 'flat_bundle',
    'label' => 'Deleted Resources',
    'description' => 'Resources of existing bundle that will be deleted',
    'widget' => array(
        'type' => 'text_textfield'
    ),
    'required' => FALSE,
    'settings' => array('text_processing' => 0),
    'display' => array(
        'default' => array(
            'type' => 'hidden',
            'label' => 'inline',
        ),
        'error' => array(
            'label' => 'inline',
        ),
    )
);

field_create_instance($instances['flat_deleted_resources']);


/*
$id = 'WrittenResource';
$sourceResource = simplexml_load_string("<cmd:$id xmlns:cmd=\"http://lat.mpi.nl/\"></cmd:$id>");
echo "hello";
echo $sourceResource->asXML();
//exit;

$nid = '774';
$node = node_load($nid);
var_dump($node);
$wrapper = entity_metadata_wrapper('node', $node);

module_load_include('inc','flat_deposit','/Helpers/CMDI/class.CmdiHandler');



$cmdiName = drupal_realpath('metadata://danrhe@mpi.nl/Collection01/My Name is Daniel/record.cmdi');
#$cmdiName = drupal_realpath('metadata://danrhe_at_mpi.nl/Collection01/LatSessionImport/record.cmdi');

if (!file_exists($cmdiName)){

    print_r('File not found');
    exit();

}



$cmdi = CmdiHandler::simplexml_load_cmdi_file($cmdiName);
$check = $cmdi->asXML();


$md_type = $wrapper->flat_cmdi_option->value();
$fileDir = $wrapper->flat_location->value();
$fid = $wrapper->flat_fid->value();

try{
    $cmdi->addResources($md_type, $fileDir, $fid);

    $cmdi->cleanMdSelfLink();


} catch (CmdiHandlerException $cmdiHandlerException){

    echo $cmdiHandlerException->getMessage();
    exit;
}
$check = $cmdi->asXML();
*/
echo 'well done';
exit;
