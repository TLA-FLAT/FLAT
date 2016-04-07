<?php
define('OC_ADMIN', 'admin');
define('OC_PASS', 'admin');
define('OC_ROOT', '/var/www/owncloud/');

class OwnCloudConnectionException extends Exception {}

function owncloud_check_connection($result) {

    if ($result->code != "200") {
        throw new OwnCloudConnectionException('Owncloud server error code (' . $result->code  . ')');
    }
}


function owncloud_account_exists($account)
{
    $return_val = false;
    $oc_account = $account->name;
    $oc_url = 'http://' . OC_ADMIN . ':' . OC_PASS . '@' . $_SERVER['HTTP_HOST'] . '/owncloud/ocs/v1.php/cloud/users?search=' . $oc_account;

    $options = array(
        'max_redirects' => 3,);

    $result = drupal_http_request($oc_url, $options);

    try {

        owncloud_check_connection($result);

        $xml = new SimpleXMLElement(file_get_contents($oc_url));

        if ($xml->data->users->element['0'] == $oc_account) {
            $return_val = true;
        } else {
            drupal_set_message(t('Drupal account without owncloud account. Please contact adminstrator', array()),'error');
            watchdog('Owncloud account', t('Owncloud account not available'), array(), WATCHDOG_ERROR);
        }
    }

    catch (OwnCloudConnectionException $e) {
        drupal_set_message(t('Check for owncloud account failed. Error message: %message', array('%message' => $e->getMessage())),'error');
        watchdog('Owncloud connection', $e->getMessage(), array(), WATCHDOG_ERROR);
    }
    return $return_val;
}




