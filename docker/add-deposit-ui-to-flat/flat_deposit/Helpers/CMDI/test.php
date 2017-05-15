<?php
/**
 * Created by PhpStorm.
 * User: danrhe
 * Date: 09/05/2017
 * Time: 11:42
 */

$drupal_path = '/var/www/html/flat';
chdir($drupal_path);
define('DRUPAL_ROOT', getcwd()); //the most important line
require_once './includes/bootstrap.inc';
drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);

module_load_include('php','flat_deposit','Helpers/CMDI/CmdiHandler');

$file = '/app/flat/backend/Ingest_service_error_log/Collection_59087f9530fcc_admin_2017-05-02T14-46-13.log';
$type = CmdiHandler::fits_mimetype_check($file);


echo $type . ' hello';


