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
            ->propertyCondition('flat_bundle_status_value', 'processing');

        $processing_bundles = $query->execute();

        // get cmdi ancestors of the current node
        $connection = islandora_get_tuque_connection();
        $ri = $connection->repository->ri;

        $ancestors_query = get_cmdi_ancestors($fid);

        $ancestors = $ri->sparqlQuery($ancestors_query);

        dd($processing_bundles);
        dd($ancestors);

    }
}
