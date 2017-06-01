#!/usr/bin/env drush
<?php

 // Delete all flat_bundle and flat_collection content
    $nids = db_select('node','n')
        ->fields('n', array('nid'))
        ->condition('type', array('flat_collection', 'flat_bundle', 'blog'), 'IN')
        ->execute()
        ->fetchCol();

    if (!empty ($nids)){
        node_delete_multiple($nids);
        drupal_set_message('Nodes containing custom content types have been removed');
    }
