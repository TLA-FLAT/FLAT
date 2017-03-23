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
     * FlatBundle constructor.
     */
    public function __construct($node)
    {
        $this->node = $node;
        $this->wrapper = entity_metadata_wrapper('node', $node);
    }


    /**
     * Creates metadata directories to prepare the storage of cmdi files in drupal (private) and freeze location
     *
     * @param bool $collection_name (optional) name of collection different than the one set to which a bundle belongs
     * @return bool
     */
    public function createBundleDirectories($collection_name = false){

        $object = user_load($this->node->uid);
        $user_name = $object->name;

        $collection_name = $collection_name ? $collection_name : $this->wrapper->flat_parent_title->value();
        $bundle_name = $this->node->title;

        $resource_dir = 'private://flat_deposit/data/' . $user_name . "/$collection_name";
        $meta_dir = 'private://flat_deposit/metadata/' . $user_name . "/$collection_name/$bundle_name";
        $freeze_dir = 'freeze://' . $user_name .  "/$collection_name/$bundle_name/metadata";

        $dirs = array($resource_dir, $meta_dir, $freeze_dir);

        foreach ($dirs as $dir){

            if (!file_exists($dir)) {
                $check = drupal_mkdir($dir, NULL, TRUE);
                if (!$check) return FALSE;
            }

        }

        return TRUE;

    }

    public function moveBundleToCollection($collection_name){

        $bundle_name = $this->node->title;

        $old_collection = $this->wrapper->flat_parent_title->value();
        $new_collection = $collection_name;


        $object = user_load($this->node->uid);

        $base = 'private://flat_deposit/';

        $resource_source = drupal_realpath($base) . '/data/' . $object->name . "/$old_collection/$bundle_name";
        $resource_target = drupal_realpath($base) . '/data/' . $object->name . "/$new_collection/$bundle_name";
        $meta_target = $base . '/metadata/' . $object->name . "/$new_collection/$bundle_name";

        // Check if target directory of resources already exists
        if (file_exists($resource_target)){
            return FALSE;
        }


        // move data
        $data_is_moved = rename($resource_source, $resource_target);

        if (!$data_is_moved) return FALSE;

        // remove source collection directory if empty
        if (!(new FilesystemIterator(dirname($resource_source)))->valid()) rmdir (dirname($resource_source));


        //if metadata exists move to new location
        if ($this->wrapper->flat_cmdi_file){

            $metadata_file_info =$this->wrapper->flat_cmdi_file->value();
            $file= file_load($metadata_file_info['fid']);
            $new_file = file_move($file, $meta_target, FILE_EXISTS_REPLACE);

            if (!$new_file) return FALSE;

        }


        return TRUE;

    }

}