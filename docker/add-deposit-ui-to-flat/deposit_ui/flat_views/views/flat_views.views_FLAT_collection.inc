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
$view->name = 'FLAT collection';
$view->description = 'The FLAT collection-member relationships';
$view->tag = 'default';
$view->base_table = 'flat_deposit_ui_collection';
$view->human_name = 'FLAT collection';
$view->core = 7;
$view->api_version = '3.0';
$view->disabled = FALSE; /* Edit this to true to make a default view disabled initially */

/* Display: Master */
$handler = $view->new_display('default', 'Master', 'default');
$handler->display->display_options['title'] = 'FLAT collections';
$handler->display->display_options['use_more'] = TRUE;
$handler->display->display_options['use_more_always'] = FALSE;
$handler->display->display_options['access']['type'] = 'none';
$handler->display->display_options['cache']['type'] = 'none';
$handler->display->display_options['query']['type'] = 'views_query';
$handler->display->display_options['exposed_form']['type'] = 'basic';
$handler->display->display_options['pager']['type'] = 'full';
$handler->display->display_options['pager']['options']['items_per_page'] = '10';
$handler->display->display_options['pager']['options']['offset'] = '0';
$handler->display->display_options['pager']['options']['id'] = '0';
$handler->display->display_options['pager']['options']['quantity'] = '9';
$handler->display->display_options['style_plugin'] = 'table';
$handler->display->display_options['style_options']['columns'] = array(
    'collection_id' => 'collection_id',
    'collection_name' => 'collection_name',
    'label' => 'label',
    'language' => 'language',
    'member' => 'member',
    'date_created' => 'date_created'
);
$handler->display->display_options['style_options']['default'] = 'collection_id';
$handler->display->display_options['style_options']['info'] = array(
    'collection_id' => array(
        'sortable' => 1,
        'default_sort_order' => 'asc',
        'align' => '',
        'separator' => '',
        'empty_column' => 0,
    ),
    'collection_name' => array(
        'sortable' => 1,
        'default_sort_order' => 'asc',
        'align' => '',
        'separator' => '',
        'empty_column' => 0,
    ),
    'label' => array(
        'sortable' => 1,
        'default_sort_order' => 'asc',
        'align' => '',
        'separator' => '',
        'empty_column' => 0,
    ),
    'language' => array(
        'sortable' => 0,
        'default_sort_order' => 'asc',
        'align' => '',
        'separator' => '',
        'empty_column' => 0,
    ),
    'member' => array(
        'sortable' => 1,
        'default_sort_order' => 'asc',
        'align' => '',
        'separator' => '',
        'empty_column' => 0,

    ),
    'date_created' => array(
        'sortable' => 1,
        'default_sort_order' => 'asc',
        'align' => '',
        'separator' => '',
        'empty_column' => 0,

    )
);
/* Field: Collection: Collection ID */
$handler->display->display_options['fields']['collection_id']['id'] = 'collection_id';
$handler->display->display_options['fields']['collection_id']['table'] = 'flat_deposit_ui_collection';
$handler->display->display_options['fields']['collection_id']['field'] = 'collection_id';
/* Field: Collection: Collection_name */
$handler->display->display_options['fields']['collection_name']['id'] = 'collection_name';
$handler->display->display_options['fields']['collection_name']['table'] = 'flat_deposit_ui_collection';
$handler->display->display_options['fields']['collection_name']['field'] = 'collection_name';
/* Field: Collection: Member */
$handler->display->display_options['fields']['member']['id'] = 'member';
$handler->display->display_options['fields']['member']['table'] = 'flat_deposit_ui_collection';
$handler->display->display_options['fields']['member']['field'] = 'member';
/* Field: Collection: Date created */
$handler->display->display_options['fields']['date_created']['id'] = 'date_created';
$handler->display->display_options['fields']['date_created']['table'] = 'flat_deposit_ui_collection';
$handler->display->display_options['fields']['date_created']['field'] = 'date_created';
/* Field: Collection: Action buttons */
$handler->display->display_options['fields']['actions']['id'] = 'actions';
$handler->display->display_options['fields']['actions']['table'] = 'flat_deposit_ui_collection';
$handler->display->display_options['fields']['actions']['field'] = 'actions';
/* Sort criterion: Collection: Collection_name */
$handler->display->display_options['sorts']['collection_name']['id'] = 'collection_name';
$handler->display->display_options['sorts']['collection_name']['table'] = 'flat_deposit_ui_collection';
$handler->display->display_options['sorts']['collection_name']['field'] = 'collection_name';
/* Sort criterion: Collection: date_created */
$handler->display->display_options['sorts']['date_created']['id'] = 'date_created';
$handler->display->display_options['sorts']['date_created']['table'] = 'flat_deposit_ui_collection';
$handler->display->display_options['sorts']['date_created']['field'] = 'date_created';
/* Sort criterion: Collection: Member */
$handler->display->display_options['sorts']['member']['id'] = 'member';
$handler->display->display_options['sorts']['member']['table'] = 'flat_deposit_ui_collection';
$handler->display->display_options['sorts']['member']['field'] = 'member';
/* Filter criterion: Collection: Collection_name */
$handler->display->display_options['filters']['collection_name']['id'] = 'collection_name';
$handler->display->display_options['filters']['collection_name']['table'] = 'flat_deposit_ui_collection';
$handler->display->display_options['filters']['collection_name']['field'] = 'collection_name';
$handler->display->display_options['filters']['collection_name']['exposed'] = TRUE;
$handler->display->display_options['filters']['collection_name']['expose']['operator_id'] = 'collection_name_op';
$handler->display->display_options['filters']['collection_name']['expose']['label'] = 'Collection_name';
$handler->display->display_options['filters']['collection_name']['expose']['use_operator'] = TRUE;
$handler->display->display_options['filters']['collection_name']['expose']['operator'] = 'collection_name_op';
$handler->display->display_options['filters']['collection_name']['expose']['identifier'] = 'collection_name';
$handler->display->display_options['filters']['collection_name']['expose']['remember_roles'] = array(
    2 => '2',
    1 => 0,
    3 => 0,
);
$handler->display->display_options['filters']['collection_name']['group_info']['label'] = 'Collection_name';
$handler->display->display_options['filters']['collection_name']['group_info']['identifier'] = 'collection_name';
$handler->display->display_options['filters']['collection_name']['group_info']['remember'] = FALSE;
$handler->display->display_options['filters']['collection_name']['group_info']['group_items'] = array(
    1 => array(),
    2 => array(),
    3 => array(),
);
/* Filter criterion: Collection: Member */
$handler->display->display_options['filters']['member']['id'] = 'member';
$handler->display->display_options['filters']['member']['table'] = 'flat_deposit_ui_collection';
$handler->display->display_options['filters']['member']['field'] = 'member';
$handler->display->display_options['filters']['member']['exposed'] = TRUE;
$handler->display->display_options['filters']['member']['expose']['operator_id'] = 'member_op';
$handler->display->display_options['filters']['member']['expose']['label'] = 'Member';
$handler->display->display_options['filters']['member']['expose']['use_operator'] = TRUE;
$handler->display->display_options['filters']['member']['expose']['operator'] = 'member_op';
$handler->display->display_options['filters']['member']['expose']['identifier'] = 'member';
$handler->display->display_options['filters']['member']['expose']['remember_roles'] = array(
    2 => '2',
    1 => 0,
    3 => 0,
);

/* Display: Page */
$handler = $view->new_display('page', 'Page', 'page');
$handler->display->display_options['path'] = 'managecollections';
$handler->display->display_options['menu']['type'] = 'tab';
$handler->display->display_options['menu']['title'] = 'Manage FLAT collection - member relationships';
$handler->display->display_options['menu']['weight'] = '0';
$handler->display->display_options['menu']['context'] = 0;
$handler->display->display_options['menu']['context_only_inline'] = 0;


$views[$view->name] = $view;