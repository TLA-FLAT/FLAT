<?php
/**
 * @file
 * views related code.
 *
 *
 * It implements :
 *   hook_views_default_views()
 *
 */
$view = new view();
$view->name = 'FLAT My Collections View';
$view->description = 'View of an individuals collections';
$view->tag = 'default';
$view->base_table = 'node';
$view->human_name = 'My active collections';
$view->core = 7;
$view->api_version = '3.0';
$view->disabled = FALSE; /* Edit this to true to make a default view disabled initially */

/* Display: Master */
$handler = $view->new_display('default', 'Master', 'default');
$handler->display->display_options['title'] = 'My active collections';
$handler->display->display_options['use_more_always'] = FALSE;
$handler->display->display_options['access']['type'] = 'perm';
$handler->display->display_options['access']['perm'] = 'use deposit module';
$handler->display->display_options['cache']['type'] = 'none';
$handler->display->display_options['query']['type'] = 'views_query';
$handler->display->display_options['exposed_form']['type'] = 'basic';
$handler->display->display_options['exposed_form']['options']['autosubmit'] = TRUE;
$handler->display->display_options['pager']['type'] = 'full';
$handler->display->display_options['pager']['options']['items_per_page'] = '10';
$handler->display->display_options['style_plugin'] = 'table';

/* Field: Content: Title */
$handler->display->display_options['fields']['title']['id'] = 'title';
$handler->display->display_options['fields']['title']['table'] = 'node';
$handler->display->display_options['fields']['title']['field'] = 'title';
$handler->display->display_options['fields']['title']['label'] = 'Collection name';
$handler->display->display_options['fields']['title']['alter']['word_boundary'] = FALSE;
$handler->display->display_options['fields']['title']['alter']['ellipsis'] = FALSE;

/* Field: Content: FID of the collection flat_fid*/
$handler->display->display_options['fields']['flat_flat_fid']['id'] = 'flat_fid';
$handler->display->display_options['fields']['flat_flat_fid']['table'] = 'field_data_flat_fid';
$handler->display->display_options['fields']['flat_flat_fid']['field'] = 'flat_fid';
$handler->display->display_options['fields']['flat_flat_fid']['label'] = 'Link name in archive';

/* Field: Content: Status of the upload */
$handler->display->display_options['fields']['flat_collection_status']['id'] = 'flat_collection_status';
$handler->display->display_options['fields']['flat_collection_status']['table'] = 'field_data_flat_collection_status';
$handler->display->display_options['fields']['flat_collection_status']['field'] = 'flat_collection_status';
$handler->display->display_options['fields']['flat_collection_status']['label'] = 'Collection status';

/* Field: Content: Post date */
$handler->display->display_options['fields']['created']['id'] = 'created';
$handler->display->display_options['fields']['created']['table'] = 'node';
$handler->display->display_options['fields']['created']['field'] = 'created';
$handler->display->display_options['fields']['created']['label'] = 'Initiated on';
$handler->display->display_options['fields']['created']['date_format'] = 'long';
$handler->display->display_options['fields']['created']['second_date_format'] = 'long';

/* Field: Content: Edit link */
$handler->display->display_options['fields']['edit_node']['id'] = 'edit_node';
$handler->display->display_options['fields']['edit_node']['table'] = 'views_entity_node';
$handler->display->display_options['fields']['edit_node']['field'] = 'edit_node';
$handler->display->display_options['fields']['edit_node']['label'] = 'Edit';

/* Field: Content: Delete link */
$handler->display->display_options['fields']['delete_node']['id'] = 'delete_node';
$handler->display->display_options['fields']['delete_node']['table'] = 'views_entity_node';
$handler->display->display_options['fields']['delete_node']['field'] = 'delete_node';
$handler->display->display_options['fields']['delete_node']['label'] = 'Delete';

/* Sort criterion: Content: Post date */
$handler->display->display_options['sorts']['created']['id'] = 'created';
$handler->display->display_options['sorts']['created']['table'] = 'node';
$handler->display->display_options['sorts']['created']['field'] = 'created';
$handler->display->display_options['sorts']['created']['expose']['label'] = 'Initiation date';
$handler->display->display_options['sorts']['created']['order'] = 'DESC';
$handler->display->display_options['sorts']['created']['exposed'] = TRUE;

/* Sort criterion: Content: Name of the collection (flat_parent_title) */
$handler->display->display_options['sorts']['flat_fid_value']['id'] = 'flat_fid_value';
$handler->display->display_options['sorts']['flat_fid_value']['table'] = 'field_data_flat_fid';
$handler->display->display_options['sorts']['flat_fid_value']['field'] = 'flat_fid_value';
$handler->display->display_options['sorts']['flat_fid_value']['exposed'] = TRUE;
$handler->display->display_options['sorts']['flat_fid_value']['expose']['label'] = 'FID';

/* Sort criterion: Content: Status of the upload (flat_bundle_status) */
$handler->display->display_options['sorts']['flat_collection_status_value']['id'] = 'flat_collection_status_value';
$handler->display->display_options['sorts']['flat_collection_status_value']['table'] = 'field_data_flat_collection_status';
$handler->display->display_options['sorts']['flat_collection_status_value']['field'] = 'flat_collection_status_value';
$handler->display->display_options['sorts']['flat_collection_status_value']['exposed'] = TRUE;
$handler->display->display_options['sorts']['flat_collection_status_value']['expose']['label'] = 'Upload status';

/* Filter criterion: Content: Type */
$handler->display->display_options['filters']['type']['id'] = 'type';
$handler->display->display_options['filters']['type']['table'] = 'node';
$handler->display->display_options['filters']['type']['field'] = 'type';
$handler->display->display_options['filters']['type']['value'] = array(
    'flat_collection' => 'flat_collection',
);

/* Filter criterion: User: Current */
$handler->display->display_options['filters']['uid_current']['id'] = 'uid_current';
$handler->display->display_options['filters']['uid_current']['table'] = 'users';
$handler->display->display_options['filters']['uid_current']['field'] = 'uid_current';
$handler->display->display_options['filters']['uid_current']['relationship'] = 'uid';
$handler->display->display_options['filters']['uid_current']['value'] = '1';

/* Display: Personal upload view page */
/*$handler = $view->new_display('page', 'Personal Collections view page', 'page');
$handler->display->display_options['display_description'] = 'View of all personal collections';
$handler->display->display_options['path'] = 'my_collections';
*/

/* Display: Block */
/* Display: Personal upload view page */
$handler = $view->new_display('block', 'Block', 'flat_my_collections_block');
$handler->display->display_options['block_description'] = 'FLAT my collections';
$handler->display->display_options['menu']['title'] = 'User Collections';
$handler->display->display_options['defaults']['pager'] = TRUE;
$handler->display->display_options['pager']['type'] = 'some';
$handler->display->display_options['pager']['options']['items_per_page'] = '5';

$views[$view->name] = $view;





