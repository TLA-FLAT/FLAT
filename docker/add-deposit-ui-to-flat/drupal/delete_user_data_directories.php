#!/usr/bin/env drush
<?php

    // Delete all user data from drupal user data folder and freeze folder
    module_load_include('inc', 'flat_deposit', 'inc/class.FlatBundle');

    $users = entity_load('user');

    foreach ($users as $user){

        if(strlen($user->name) > 0){

            $user_data_directory =  drupal_realpath('external://' . str_replace('@' ,'_at_' , $user->name));
            $user_freeze_directory =  drupal_realpath('freeze://' . str_replace('@' ,'_at_' , $user->name));

            if (file_exists($user_data_directory) && $user_data_directory){

                FlatBundle::recursiveRmDir($user_data_directory);
                rmdir ($user_data_directory);}

            if (file_exists($user_freeze_directory) && $user_freeze_directory){

                FlatBundle::recursiveRmDir($user_freeze_directory);
                rmdir ($user_freeze_directory);}
        }
    }

    drupal_set_message('All non-archived user project data files in drupal data folder and freeze folder have been removed');
