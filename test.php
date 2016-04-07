#!/usr/bin/env drush
<?php

$self = drush_sitealias_get_record('@self');
if (empty($self)) {
    drush_die("I can't bootstrap from the current location.", 0);
}

drush_print("Time to prepare the working environment.");

// let's jump to our site directory before we do anything else
drush_op('chdir', $self['root']);

define('DRUPAL_ROOT', getcwd());

require_once DRUPAL_ROOT . '/includes/bootstrap.inc';
drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);
#informs drupal about entity
wysiwyg_entity_info();

#dependencies
require_once DRUPAL_ROOT . '/'. drupal_get_path('module', 'wysiwyg') . '/wysiwyg.admin.inc' ;


