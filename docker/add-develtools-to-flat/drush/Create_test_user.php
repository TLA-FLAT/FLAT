#!/usr/bin/env drush
<?php

//This will generate a random password, you could set your own here
  $password = 'test1234';

  //set up the user fields
  $fields = array(
    'name' => 'test',
    'mail' => 'test@example.com',
    'pass' => $password,
    'status' => 1,
    'init' => 'email address',
    'roles' => array(
      DRUPAL_AUTHENTICATED_RID => 'authenticated user',
    ),
  );

  //the first parameter is left blank so a new user is created
  $account = user_save('', $fields);

