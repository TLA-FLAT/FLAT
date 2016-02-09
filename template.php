<?php
/**
 * @file
 * template.php
 */

/**
 * Allows for images as menu items.
 * Just supply the an image path in the title. The image path will be replaced
 * with an img tag. The description is used as alt text and title.
 * Implements theme_menu_link().
 * Reference: http://chrisshattuck.com/blog/how-use-images-menu-items-drupal-simple-preprocessing-function
 **/


function flat_deposit_ui_theme_menu_link($link) {


    $element = $link['element'];
    // Allows for images as menu items. Just supply the path to the image as the title
    if ( strpos($element ['#title'], '.png') !== false || strpos($element ['#title'], '.jpg') !== false || strpos($element ['#title'], '.gif') !== false) {
        $link['element']['#title'] =  '<img title="'. $element['#original_link']['description'] .'" alt="'. $element['#original_link']['description'].'" src="'. url($link['element']['#title']) .'"/>';
        $link['element']['#localized_options']['html'] = TRUE;
}

    return theme_menu_link($link);
}




?>



