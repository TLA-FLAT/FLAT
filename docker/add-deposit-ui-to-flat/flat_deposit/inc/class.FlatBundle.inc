<?php

/**
 *
 */
class FlatBundle
{

    /**
     * Checks that combination of collection name and bundle is unique for a certain user.
     *
     * @param $user_id integer drupal id of user for which nodes are queried
     * @param $collection String title of the parent
     * @param $bundle_name String title of the node
     *
     * @return bool TRUE if combination of collection and bundle name is unique
     */
    static public function checkUniquenessNaming($user_id, $collection, $bundle_name)
    {

        $query = new EntityFieldQuery();
        $query->entityCondition('entity_type', 'node')
            ->entityCondition('bundle', 'flat_bundle')
            ->propertyCondition('uid', $user_id)
            ->propertyCondition('title', $bundle_name)
            ->fieldCondition('flat_parent_title', 'value', "$collection", '=');

        $user_node_nids_in_collection = $query->execute();

        if (!empty($user_node_nids_in_collection)) {

            return FALSE;
        }
        return TRUE;
    }





    /**
     * Moves bundle data and meta data to new location. This can be a different collection or the freeze location
     *
     * @param stdClass $node drupal flat_bundle node.
     *
     * @param string $data_type type of data to be moved: 'metadata' or 'data'.
     *
     * @param string $target_stream Target location. Allowed values are: 'freeze' and 'private'
     *
     * @param string $toNewCollection [optional] Name of a new collection
     *
     * @return bool|string new location of the bundle data or false
     */
    static public function moveBundleData($node, $data_type, $target_stream, $toNewCollection = ''){

        $wrapper = entity_metadata_wrapper('node', $node);

        // validate node
        if ($node->type != 'flat_bundle'){
            trigger_error("Wrong node type for data move",E_USER_WARNING);
            return FALSE;

        }


        if ($data_type == 'metadata'){

            $metadata_file_info = $wrapper->flat_cmdi_file->value();
            $file= file_load($metadata_file_info['fid']);

            $original_location = $metadata_file_info['uri'];

        } else {

            $original_location = $wrapper->flat_location->value();
        }

        // Validate that the files' original location is correct
        if (!file_exists($original_location)){
            trigger_error("Source directory does not exist",E_USER_ERROR);
            return FALSE;

        }


        // generate new target path
        $bundle = $node->title;
        $collection_name = ($toNewCollection != '') ? $toNewCollection : $wrapper->flat_parent_title->value();

        $usr = user_load($node->uid);
        $tail = $usr->name . "/$collection_name/$bundle";



        if ($data_type == 'metadata'){

            $base = ($target_stream == 'freeze') ? 'freeze://metadata' : "metadata://";

            $new_location = "$base/$tail/";

            $new_location .= 'record.cmdi';

        } else{
            $base = ($target_stream == 'freeze') ? 'freeze://data' : 'external://';

            $new_location = "$base/$tail/";

        }



        // validate that new location does not exist
        if(file_exists($new_location)) {
            trigger_error("Target location already exists", E_USER_ERROR);
            return false;
        }

        $collection_dir = dirname($new_location);

        if (!file_exists($collection_dir)){
            $check = drupal_mkdir($collection_dir, NULL, TRUE);

            if(!$check) {
                trigger_error("Unable to create folder at target original_location", E_USER_ERROR);
                return false;
            }
        }

        if ($data_type == 'metadata'){

            $new_file = file_move($file, $new_location, FILE_EXISTS_REPLACE);

            if (!$new_file) {
                trigger_error("Unable to move metadata to new target original_location", E_USER_ERROR);
                return FALSE;
            }

            /*
            // Get rid of cached bundle file info
            $cid = "field:node:" . $node->nid;
            $cid2 = "field:node:" . $file->fid;
            $num_deleted = db_delete('cache_field')
                ->condition('cid', array($cid, $cid2), 'in')
                ->execute();
*/
            // Change node information
            $metadata_file_info ['uri'] = $new_location;
            $wrapper->flat_cmdi_file->set($metadata_file_info);
            $wrapper->save();


            if (self::is_dir_empty($original_location)){

                drupal_rmdir($original_location);
            }

            return $new_location;

        } else {


            $moved = rename(drupal_realpath($original_location), drupal_realpath($new_location)); // Unfortunately, rename does not return true on success

            if (!file_exists($new_location)) {
                trigger_error("Unable to move data to freeze original_location", E_USER_ERROR);
                return false;
            }

            // Change node information
            $wrapper->flat_location->set($new_location);
            $wrapper->save();

            return $new_location;
        }


    }



    static public function recursiveRmDir($dir)
    {
        $iterator = new RecursiveIteratorIterator(new \RecursiveDirectoryIterator($dir, \FilesystemIterator::SKIP_DOTS), \RecursiveIteratorIterator::CHILD_FIRST);
        foreach ($iterator as $filename => $fileInfo) {
            if ($fileInfo->isDir()) {
                rmdir($filename);
            } else {
                unlink($filename);
            }
        }
    }

    static public function is_dir_empty($dir) {
        if (!is_readable($dir)) return NULL;
        if (!is_dir($dir)) return NULL;
        return (count(scandir($dir)) == 2);
    }


}