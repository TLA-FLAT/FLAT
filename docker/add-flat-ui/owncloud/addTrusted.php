<?php

include('/var/www/owncloud/config/config.backup');
$CONFIG ['trusted_domains'][1] = '192.168.99.100';
$v = var_export($CONFIG, true);
echo $v;
?>

