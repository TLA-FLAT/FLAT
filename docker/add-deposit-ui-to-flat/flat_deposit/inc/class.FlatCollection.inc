<?php

/**
 *
 */

class FlatCollectiomException extends Exception
{
}

class FlatCollection
{

    /**
     * Query flat collection nodes of a specific user.
     *
     * @param integer $user_id user id for which nodes are queried.
     * @param String $fid (optional) filters for collections with a certain flat_fid field value.
     *
     * @return array results of the query
     */
    public static function getUserCollectionNodes($user_id, $fid = null)
    {
        $query = new EntityFieldQuery();
        $query->entityCondition('entity_type', 'node')
            ->entityCondition('bundle', 'flat_collection')
            ->propertyCondition('uid', $user_id);
        if ($fid) {
            $query->fieldCondition('flat_fid', 'value', "$fid", '=');
        }

        $collection_nodes = $query->execute();

        return $collection_nodes;
    }
}
