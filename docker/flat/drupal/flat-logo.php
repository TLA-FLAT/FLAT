#!/usr/bin/env drush
<?php

// Get the settings
$theme_settings = variable_get('theme_settings', array());

// Set the variable
$theme_settings ['default_logo'] = 0;
$theme_settings ['logo_path'] = 'public://flat-logo.png';

// Save our settings
variable_set('theme_settings', $theme_settings);
