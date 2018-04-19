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

$id = 'WrittenResource';
$sourceResource = simplexml_load_string("<cmd:$id xmlns:cmd=\"http://lat.mpi.nl/\"></cmd:$id>");
echo "hello";
echo $sourceResource->asXML();
//exit;

$nid = '525';
$node = node_load($nid);
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
echo 'well done';
exit;
