<?php

$view = new view;
$view->name = 'my_action_view';
$view->description = '';
$view->tag = 'default';
$view->base_table = 'node';
$view->human_name = 'my_action_view';
$view->core = 7;
$view->api_version = '3.0';
$view->disabled = FALSE; /* Edit this to true to make a default view disabled initially */

/* Display: Master */
$handler = $view->new_display('default', 'Master', 'default');
$handler->display->display_options['title'] = 'my_action_view';
$handler->display->display_options['access']['type'] = 'perm';
$handler->display->display_options['cache']['type'] = 'none';
$handler->display->display_options['query']['type'] = 'views_query';
$handler->display->display_options['query']['options']['query_comment'] = FALSE;
$handler->display->display_options['exposed_form']['type'] = 'basic';
$handler->display->display_options['pager']['type'] = 'full';
$handler->display->display_options['pager']['options']['items_per_page'] = '10';
$handler->display->display_options['style_plugin'] = 'table';
$handler->display->display_options['style_options']['columns'] = array(
    'title' => 'title',
);
$handler->display->display_options['style_options']['default'] = '-1';
$handler->display->display_options['style_options']['info'] = array(
    'title' => array(
        'sortable' => 0,
        'default_sort_order' => 'asc',
        'align' => '',
        'separator' => '',
        'empty_column' => 0,
    ),
);
$handler->display->display_options['style_options']['override'] = 1;
$handler->display->display_options['style_options']['sticky'] = 0;
$handler->display->display_options['style_options']['empty_table'] = 0;
/* Field: Content: Actions */
$handler->display->display_options['fields']['actions']['id'] = 'actions';
$handler->display->display_options['fields']['actions']['table'] = 'node';
$handler->display->display_options['fields']['actions']['field'] = 'actions';
$handler->display->display_options['fields']['actions']['alter']['alter_text'] = 0;
$handler->display->display_options['fields']['actions']['alter']['make_link'] = 0;
$handler->display->display_options['fields']['actions']['alter']['absolute'] = 0;
$handler->display->display_options['fields']['actions']['alter']['external'] = 0;
$handler->display->display_options['fields']['actions']['alter']['replace_spaces'] = 0;
$handler->display->display_options['fields']['actions']['alter']['trim_whitespace'] = 0;
$handler->display->display_options['fields']['actions']['alter']['nl2br'] = 0;
$handler->display->display_options['fields']['actions']['alter']['word_boundary'] = 1;
$handler->display->display_options['fields']['actions']['alter']['ellipsis'] = 1;
$handler->display->display_options['fields']['actions']['alter']['more_link'] = 0;
$handler->display->display_options['fields']['actions']['alter']['strip_tags'] = 0;
$handler->display->display_options['fields']['actions']['alter']['trim'] = 0;
$handler->display->display_options['fields']['actions']['alter']['html'] = 0;
$handler->display->display_options['fields']['actions']['element_label_colon'] = 1;
$handler->display->display_options['fields']['actions']['element_default_classes'] = 1;
$handler->display->display_options['fields']['actions']['hide_empty'] = 0;
$handler->display->display_options['fields']['actions']['empty_zero'] = 0;
$handler->display->display_options['fields']['actions']['hide_alter_empty'] = 1;
/* Field: Content: Title */
$handler->display->display_options['fields']['title']['id'] = 'title';
$handler->display->display_options['fields']['title']['table'] = 'node';
$handler->display->display_options['fields']['title']['field'] = 'title';
$handler->display->display_options['fields']['title']['label'] = '';
$handler->display->display_options['fields']['title']['alter']['alter_text'] = 0;
$handler->display->display_options['fields']['title']['alter']['make_link'] = 0;
$handler->display->display_options['fields']['title']['alter']['absolute'] = 0;
$handler->display->display_options['fields']['title']['alter']['word_boundary'] = 0;
$handler->display->display_options['fields']['title']['alter']['ellipsis'] = 0;
$handler->display->display_options['fields']['title']['alter']['strip_tags'] = 0;
$handler->display->display_options['fields']['title']['alter']['trim'] = 0;
$handler->display->display_options['fields']['title']['alter']['html'] = 0;
$handler->display->display_options['fields']['title']['hide_empty'] = 0;
$handler->display->display_options['fields']['title']['empty_zero'] = 0;
$handler->display->display_options['fields']['title']['link_to_node'] = 1;
/* Sort criterion: Content: Post date */
$handler->display->display_options['sorts']['created']['id'] = 'created';
$handler->display->display_options['sorts']['created']['table'] = 'node';
$handler->display->display_options['sorts']['created']['field'] = 'created';
$handler->display->display_options['sorts']['created']['order'] = 'DESC';
/* Contextual filter: Content: Author uid */
$handler->display->display_options['arguments']['uid']['id'] = 'uid';
$handler->display->display_options['arguments']['uid']['table'] = 'node';
$handler->display->display_options['arguments']['uid']['field'] = 'uid';
$handler->display->display_options['arguments']['uid']['default_argument_type'] = 'fixed';
$handler->display->display_options['arguments']['uid']['summary']['format'] = 'default_summary';
/* Filter criterion: Content: Type */
$handler->display->display_options['filters']['type']['id'] = 'type';
$handler->display->display_options['filters']['type']['table'] = 'node';
$handler->display->display_options['filters']['type']['field'] = 'type';
$handler->display->display_options['filters']['type']['value'] = array(
    'article' => 'article',
);

/* Display: Page */
$handler = $view->new_display('page', 'Page', 'page');
$handler->display->display_options['path'] = 'my-action-view';
$translatables['my_action_view'] = array(
    t('Master'),
    t('my_action_view'),
    t('more'),
    t('Apply'),
    t('Reset'),
    t('Sort by'),
    t('Asc'),
    t('Desc'),
    t('Items per page'),
    t('- All -'),
    t('Offset'),
    t('Actions'),
    t('All'),
    t('Page'),
);

$views[$view->name] = $view;
