<?php

/**
 *
 */

class FlatBundleException extends Exception
{
}

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
    public static function checkUniquenessNaming($user_id, $collection, $bundle_name)
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
     * @param string $move_direction Direction in which data will be moved. Allowed values are: 'freeze' and 'unfreeze'
     *
     * @return bool|string new location of the bundle data or false
     */
    public static function moveBundleData($node, $data_type, $move_direction)
    {

        $wrapper = entity_metadata_wrapper('node', $node);

        try {
            self::validate_node($node);
            $source_location = self::set_original_location($wrapper, $data_type);
            self::validate_source($wrapper, $source_location, $data_type);
            $target_location = self::set_target_location($node, $wrapper, $data_type, $move_direction);
            self::validate_target($wrapper, $data_type, $source_location, $target_location);
            self::validate_naming($node);
            $new_location = self::move_data($wrapper, $data_type, $source_location, $target_location);
        } catch (FlatBundleException $exception) {
            trigger_error($exception->getMessage(), E_USER_WARNING);
            return FALSE;
        }

        return $new_location;
    }

    /**
     * Validates if new collection already has a bundle or collection with the same name (i.e. fedora object label).
     *
     * @param $node
     */
    public static function validate_naming($node)
    {
        $title = $node->title;
        $wrapper = entity_metadata_wrapper('node', $node);
        $fid = @$wrapper->flat_fid->value();
        if (isset($fid)) {
            return;
        }

        $parent_nid = $wrapper->flat_parent_nid->value();
        $parent_node = node_load($parent_nid);
        $parent_wrapper = entity_metadata_wrapper('node', $parent_node);
        $parent_fid = $parent_wrapper->flat_fid->value();

        module_load_include('inc', 'flat_deposit', '/inc/class.FlatTuque');
        $values = FlatTuque::getChildrenLabels($parent_fid);

        if (in_array(strtoupper($title), array_unique(array_map('strtoupper', $values)))) {
            throw new FlatBundleException("A collection or bundle with almost identical name already exists in archive");
        }
    }

    public static function recursiveRmDir($dir)
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


    public static function validate_node($node)
    {
        if ($node->type != 'flat_bundle') {
            throw new FlatBundleException("Wrong node type for data move");
        }
    }

    public static function set_original_location($wrapper, $data_type)
    {
        if ($data_type == 'metadata') {
            $metadata_file_info = $wrapper->flat_cmdi_file->value();
            $original_location = $metadata_file_info['uri'];
        } else {
            $original_location = $wrapper->flat_location->value();
        }
        return $original_location;
    }

    // generate new target path
    public static function set_target_location($node, $wrapper, $data_type, $move_direction)
    {
        $bundle = $node->title;
        $collection_name = $wrapper->flat_parent_title->value();

        $usr = user_load($node->uid);
        $tail = str_replace('@', '_at_', $usr->name) . "/$collection_name/$bundle";


        if ($data_type == 'metadata') {
            $base = ($move_direction == 'freeze') ? 'freeze://metadata' : "metadata://";
/*
            $new_location = "$base/$tail/";

            $new_location .= 'record.cmdi';
*/
            $new_location = "$base/$node->nid/record.cmdi";

            // data handling
        } else {
            if ($move_direction == 'freeze') {

                // freeze location is standard the same for all
                $base = 'freeze://data';
                /*
                $new_location = "$base/$tail/";
                */
                $new_location = "$base/$node->nid";
            } else {
                $source = $wrapper->flat_source->value();
                // flat_bundle nodes will be put back to original_path location
                if (($source == 'local') || ($source == 'cloud')) {
                    $original_location = $wrapper->flat_original_path->value();
                    $new_location = $original_location;
                } else {
                    // if unfrozen flat_bundle with local MPI_workspace resources is frozen move to freeze location
                    $base =  'external://';
                    /*
                    $new_location = "$base/$tail/";
                    */
                    $new_location = "$base/$node->nid";
                }
            }
        }
        return $new_location;
    }

    public static function validate_source($wrapper, $dir, $data_type)
    {
        $source = $wrapper->flat_source->value();

        if ($source == 'local' AND $data_type == 'data') {
            #module_load_include('inc','flat_deposit','inc/class.Workspaces');
            // todo implement validation
        } else {
            // Validate that the files' original location is correct
            if (!file_exists($dir)) {
                throw new FlatBundleException("Source directory does not exist");
            }
        }
    }

    public static function validate_target($wrapper, $data_type, $source_location, $target_location)
    {
        $source_path_elements = array_filter(explode('/', $source_location));
        $target_path_elements = array_filter(explode('/', $target_location));

        // do nothing if data stays at same location (e.g. MPI_workspace data when collection is changed)
        if (empty(array_diff($source_path_elements, $target_path_elements))) {

        } else {
            // check that target path/file is not existing
            if (file_exists($target_location)) {
                throw new FlatBundleException("Target location already exists");
            }

            // check that target path/file can be placed within parent directory
            $collection_dir = dirname($target_location);

            if (!file_exists($collection_dir)) {
                $check = drupal_mkdir($collection_dir, NULL, TRUE);

                if (!$check) {
                    throw new FlatBundleException("Unable to create folder at target original_location");
                }
            }
            if ($wrapper->flat_source->value() != 'local') {

                if (!is_writable($collection_dir)) {
                    throw new FlatBundleException("Unable to write at target location");
                }
            }
        }
    }

    public static function move_data($wrapper, $data_type, $source_location, $target_location)
    {
        //bypass move if source and target are identical
        $source_path_elements = array_filter(explode('/', $source_location));
        $target_path_elements = array_filter(explode('/', $target_location));

        // do nothing if data stays at same location (e.g. MPI_workspace data when collection is changed)
        if (empty(array_diff($source_path_elements, $target_path_elements))) {
            return $target_location;
        }

        if ($data_type == 'metadata') {
            $metadata_file_info = $wrapper->flat_cmdi_file->value();

            $original_metadata_uri = $metadata_file_info['uri'];

            $file = file_load($metadata_file_info['fid']);

            $new_file = file_move($file, $target_location, FILE_EXISTS_REPLACE);

            if (!$new_file) {
                throw new FlatBundleException("Unable to move metadata to new target original_location");
            }

            // for some unknown reason flat_location and flat_original_path are messed up by attaching the newly created cmdi file, so we need to restore it
            $flat_location_original = $wrapper->flat_location->value();
            $flat_original_path_original = $wrapper->flat_original_path->value();


            // Change node information
            $metadata_file_info ['uri'] = $target_location;
            $wrapper->flat_cmdi_file->set($metadata_file_info);
            $wrapper->save();

            $node = node_load($wrapper->getIdentifier());
            $wrapper = entity_metadata_wrapper('node', $node);
            $wrapper->flat_location->set($flat_location_original);
            $wrapper->flat_original_path->set($flat_original_path_original);
            $wrapper->save();

            if (self::is_dir_empty($original_metadata_uri)) {
                drupal_rmdir($original_metadata_uri);
            }
        } else {
            $moved = rename(drupal_realpath($source_location), drupal_realpath($target_location));
            #$moved = rename(drupal_realpath($source_location), drupal_realpath($target_location)); // Unfortunately, rename does not return true on success

            // hook for additional actions after move
            $actions = module_invoke_all('data_move_actions', $source_location, $target_location);

            if (!file_exists($target_location)) {
                throw new FlatBundleException("Unable to move data to freeze original_location");
            }

            // Change node information
            $wrapper->flat_location->set($target_location);
            $wrapper->save();
        }
            return $target_location;
    }


    public static function is_dir_empty($dir)
    {
        if (!is_readable($dir)) {
            return NULL;
        }
        if (!is_dir($dir)) {
            return NULL;
        }
        return (count(scandir($dir)) == 2);
    }
}
