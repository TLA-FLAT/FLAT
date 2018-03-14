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

echo 'hello';
$node = node_load(496);
$wrapper = entity_metadata_wrapper('node', $node);

$title = $node->title;
$collection = $wrapper->flat_parent_title->value();

$owner = 'danrhe@mpi.nl';



module_load_include('inc','flat_deposit','/Helpers/CMDI/class.CmdiHandler');
/*
$cmdi = simplexml_load_string($recordCmdi, 'CmdiHandler');
$str = $cmdi->asXML();
exit;

*/

$export_dir = 'metadata://' . '/' . str_replace('@', '_at_' , $owner) . "/$collection/$title/";
$export_dir = 'metadata://' . '/' . $owner . "/$collection/$title/";



$fName = $export_dir . 'record.cmdi';


if (!file_exists($fName)){

    print_r('File not found');
    exit();

}


$cmdi_str = file_get_contents(drupal_realpath($fName));

$cmdi = CmdiHandler::loadCleanedCmdi($cmdi_str);


$fileDir = $wrapper->flat_location->value();
#$cmdi->removeMdSelfLink();

$fid = $wrapper->flat_fid->value();
$str = $cmdi->asXML();
$cmdi->addResources($fileDir, $fid);
$str = $cmdi->asXML();
echo 'done';
exit;






