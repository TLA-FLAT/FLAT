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
$view->name = 'flat_workspace';
$view->description = 'View of an individuals uploads and notifications ';
$view->tag = 'default';
$view->base_table = 'node';
$view->human_name = 'FLAT workspace';
$view->core = 7;
$view->api_version = '3.0';
$view->disabled = FALSE; /* Edit this to true to make a default view disabled initially */

/* Display: Master */
$handler = $view->new_display('default', 'Master', 'default');
$handler->display->display_options['title'] = 'My workspace';
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
/* Header: Global: View area */
$handler->display->display_options['header']['view_1']['id'] = 'view_1';
$handler->display->display_options['header']['view_1']['table'] = 'views';
$handler->display->display_options['header']['view_1']['field'] = 'view';
$handler->display->display_options['header']['view_1']['label'] = 'Upload bundle view area';
$handler->display->display_options['header']['view_1']['empty'] = TRUE;
/* Footer: Global: View area */
$handler->display->display_options['footer']['view']['id'] = 'view';
$handler->display->display_options['footer']['view']['table'] = 'views';
$handler->display->display_options['footer']['view']['field'] = 'view';
$handler->display->display_options['footer']['view']['label'] = 'Notification block';
$handler->display->display_options['footer']['view']['view_to_insert'] = 'notifications:block';
/* Relationship: Content: Author */
$handler->display->display_options['relationships']['uid']['id'] = 'uid';
$handler->display->display_options['relationships']['uid']['table'] = 'node';
$handler->display->display_options['relationships']['uid']['field'] = 'uid';
$handler->display->display_options['relationships']['uid']['required'] = TRUE;
/* Field: Content: Title */
$handler->display->display_options['fields']['title']['id'] = 'title';
$handler->display->display_options['fields']['title']['table'] = 'node';
$handler->display->display_options['fields']['title']['field'] = 'title';
$handler->display->display_options['fields']['title']['label'] = 'Upload bundle name';
$handler->display->display_options['fields']['title']['alter']['word_boundary'] = FALSE;
$handler->display->display_options['fields']['title']['alter']['ellipsis'] = FALSE;
/* Field: Content: Name of the collection */
$handler->display->display_options['fields']['upload_collection']['id'] = 'upload_collection';
$handler->display->display_options['fields']['upload_collection']['table'] = 'field_data_upload_collection';
$handler->display->display_options['fields']['upload_collection']['field'] = 'upload_collection';
$handler->display->display_options['fields']['upload_collection']['label'] = 'Belongs to';
/* Field: Content: Status of the upload */
$handler->display->display_options['fields']['upload_status']['id'] = 'upload_status';
$handler->display->display_options['fields']['upload_status']['table'] = 'field_data_upload_status';
$handler->display->display_options['fields']['upload_status']['field'] = 'upload_status';
$handler->display->display_options['fields']['upload_status']['label'] = 'Upload status';

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
/* Sort criterion: Content: Name of the collection (upload_collection) */
$handler->display->display_options['sorts']['upload_collection_value']['id'] = 'upload_collection_value';
$handler->display->display_options['sorts']['upload_collection_value']['table'] = 'field_data_upload_collection';
$handler->display->display_options['sorts']['upload_collection_value']['field'] = 'upload_collection_value';
$handler->display->display_options['sorts']['upload_collection_value']['exposed'] = TRUE;
$handler->display->display_options['sorts']['upload_collection_value']['expose']['label'] = 'Collection Name)';
/* Sort criterion: Content: Status of the upload (upload_status) */
$handler->display->display_options['sorts']['upload_status_value']['id'] = 'upload_status_value';
$handler->display->display_options['sorts']['upload_status_value']['table'] = 'field_data_upload_status';
$handler->display->display_options['sorts']['upload_status_value']['field'] = 'upload_status_value';
$handler->display->display_options['sorts']['upload_status_value']['exposed'] = TRUE;
$handler->display->display_options['sorts']['upload_status_value']['expose']['label'] = 'Upload status';
/* Filter criterion: Content: Type */
$handler->display->display_options['filters']['type']['id'] = 'type';
$handler->display->display_options['filters']['type']['table'] = 'node';
$handler->display->display_options['filters']['type']['field'] = 'type';
$handler->display->display_options['filters']['type']['value'] = array(
    'flat_bundle' => 'flat_bundle',
);
/* Filter criterion: User: Current */
$handler->display->display_options['filters']['uid_current']['id'] = 'uid_current';
$handler->display->display_options['filters']['uid_current']['table'] = 'users';
$handler->display->display_options['filters']['uid_current']['field'] = 'uid_current';
$handler->display->display_options['filters']['uid_current']['relationship'] = 'uid';
$handler->display->display_options['filters']['uid_current']['value'] = '1';


/* Display: Personal upload view page */
$handler = $view->new_display('page', 'Personal upload view page', 'page');
$handler->display->display_options['display_description'] = 'View of all personal uploads';
$handler->display->display_options['path'] = 'workspace';


$views[$view->name] = $view;
