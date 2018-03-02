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

module_load_include('php','flat_deposit','Helpers/CMDI/CmdiHandler');

$fName = drupal_get_path('module', 'flat_deposit') . '/Helpers/CMDI/test.cmdi';
$cmdi = simplexml_load_file($fName, 'CmdiHandler');

$fid = 'lat:12345_5ac1b857_06cb_460a_858e_72e1cb8bcab8';

$cmdi->addAttribute('key','value');
#$parser = new Template2FormParser($template);
#$form = $parser->buildDrupalForm();

$str = $cmdi->asXML();
#print drupal_render($form);
echo "done";

#return $form;




