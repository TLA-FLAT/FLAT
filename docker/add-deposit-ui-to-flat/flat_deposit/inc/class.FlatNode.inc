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
}
