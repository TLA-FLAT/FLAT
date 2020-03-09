<?php

/**
 * Created by PhpStorm.
 * User: danrhe
 * Date: 08/06/2017
 * Time: 15:13
 */
class FlatNode
{
    public static function transliteration_clean_filename($filename, $source_langcode = NULL)
    {
        if (is_array($filename)) {
            foreach ($filename as $key => $value) {
                $filename[$key] = self::transliteration_clean_filename($value, $source_langcode);
            }
            return $filename;
        }
        $filename = self::transliteration_get($filename, '', $source_langcode);
        // Replace whitespace.
        $filename = str_replace(' ', '_', $filename);
        // Remove remaining unsafe characters.
        $filename = preg_replace('![^0-9A-Za-z_.-]!', '', $filename);
        // Remove multiple consecutive non-alphabetical characters.
        $filename = preg_replace('/(_)_+|(\.)\.+|(-)-+/', '\\1\\2\\3', $filename);
        // Force lowercase to prevent issues on case-insensitive file systems.
        if (variable_get('transliteration_file_lowercase', TRUE)) {
            $filename = strtolower($filename);
        }
        return $filename;
    }
    public static function transliteration_get($text, $unknown = '?', $source_langcode = NULL)
    {
        static $loaded = FALSE;
        if (!$loaded) {
            module_load_include('inc', 'transliteration');
            $loaded = TRUE;
        }
        return transliteration_process($text, $unknown, $source_langcode);
    }

    /**
     * Check whether any bundles are currently being processed in the CMDI hierarchy of the current node
     *
     * @param string $fid Fedora ID without prefix
     * @return bool True or False whether any bundles in the hierarchy are being processed
     */
    public static function processing_nodes_exist_in_hierarchy($fid)
    {
        module_load_include('php', 'flat_deposit','/inc/fedora_queries');

        // query bundles that are currently being processed
        $query = new EntityFieldQuery();
        $query->entityCondition('entity_type', 'node')
            ->entityCondition('bundle', 'flat_bundle')
            ->fieldCondition('flat_bundle_status', 'value', 'processing');

        $result = $query->execute();
        if (isset($result['node'])) {
            $processing_bundles_nids = array_keys($result['node']);
            $processing_bundles = entity_load('node', $processing_bundles_nids);
        }
        if ($processing_bundles) {
            $processing_bundles_parents_nids = [];
            foreach ($processing_bundles as $processing_bundle) {
                $processing_bundles_parents_nids[] = $processing_bundle->flat_parent_nid['und'][0]['value'];
            }
        }

        $processing_bundles_parents = entity_load('node', $processing_bundles_parents_nids);

        $processing_bundles_parents_fids = [];
        if ($processing_bundles_parents) {
                    foreach ($processing_bundles_parents as $processing_bundle_parent) {
                            $processing_bundles_parents_fids[] = $processing_bundle_parent->flat_fid['und'][0]['value'];
                    }
            }

        // establish connection to Fedora RI for SPARQL queries
        $connection = islandora_get_tuque_connection();
        $ri = $connection->repository->ri;

        // get cmdi ancestors of the parents of all currently processing bundle nodes
        $processing_bundles_parents_ancestors = [];
        foreach ($processing_bundles_parents_fids as $processing_bundles_parents_fid) {
            $ancestors_query = get_cmdi_ancestors($processing_bundles_parents_fid);
            $ancestors = $ri->sparqlQuery($ancestors_query);
            $processing_bundles_parents_ancestors = array_merge($processing_bundles_parents_ancestors, $ancestors);
        }

        $processing_bundles_parents_ancestor_fids = [];
        foreach ($processing_bundles_parents_ancestors as $ancestor) {
                    $processing_bundles_parents_ancestor_fids[] = $ancestor['pid']['value'];
        }

        // all fids together (parents + ancestors of parents)
        $all_processing_bundles_ancestors = array_merge($processing_bundles_parents_fids, $processing_bundles_parents_ancestor_fids);

        // get cmdi ancestors of the current node
            $current_ancestors_query = get_cmdi_ancestors($fid);
            $current_ancestors = $ri->sparqlQuery($ancestors_query);
            $current_ancestor_fids = [];
            foreach ($current_ancestors as $ancestor) {
            $current_ancestor_fids[] = $ancestor['pid']['value'];
        }

        // see wether there is an overlap between the ancestors of the current node and the ancestors of all processing bundles
        $overlap = array_intersect($current_ancestor_fids, $all_processing_bundles_ancestors);
        if ($overlap) {
            return TRUE;
        }
        else {
            return FALSE;
        }

    }
}
