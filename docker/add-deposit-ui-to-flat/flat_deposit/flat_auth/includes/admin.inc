<?php
/**
 * Flat Auth Admin Form
 *
 * @param array $form
 * @param array $form_state
 *
 * @return array
 */
function flat_auth_admin_form($form, &$form_state) {

    $form['general'] = [

      '#type'   => 'fieldset',
      '#title'  => t('Flat Auth'),
      'expiration_time' => [

          '#type'          => 'textfield',
          '#title'         => t('Expiration time in seconds'),
          '#default_value' => variable_get('flat_auth_expiration_time', FLAT_AUTH_THRESHOLD_EXPIRE),
          '#description'     => 'How long should a token be valid for',
      ],
      'deletion_time' => [

        '#type'          => 'textfield',
        '#title'         => t('Deletion time in seconds'),
        '#default_value' => variable_get('flat_auth_deletion_time', FLAT_AUTH_THRESHOLD_DELETE),
        '#description'     => 'How long until a token will be marked for deletion',
      ],
      'max_error_count' => [

        '#type'          => 'textfield',
        '#title'         => t('Max error count until token is invalidated'),
        '#default_value' => variable_get('flat_auth_max_error_count', FLAT_AUTH_MAX_ERROR_COUNT),
        '#description'     => 'How many times errors can occur until token is invalidated',
      ],
      'max_usage_count' => [

        '#type'          => 'textfield',
        '#title'         => t('Max usage count'),
        '#default_value' => variable_get('flat_auth_max_usage_count', FLAT_AUTH_MAX_USAGE_COUNT),
        '#description'     => 'How many times can a token be used',
      ],
    ];

    $form['actions'] = [

        '#type'  => 'actions',
        'submit' => [

            '#type'   => 'submit',
            '#value'  => t('Save Configuration'),
            '#weight' => 0,
            '#submit' => ['flat_auth_admin_form_submit'],
        ],
    ];

    return $form;
}

/**
 * Admin form submit handler
 *
 * @param array $form
 * @param array $form_state
 *
 * @return void
 */
function flat_auth_admin_form_submit($form, &$form_state) {

    if ($form_state['triggering_element']['#value'] === 'Save Configuration') {

        variable_set('flat_auth_expiration_time', $form_state['values']['expiration_time']);
        variable_set('flat_auth_deletion_time', $form_state['values']['deletion_time']);
        variable_set('flat_auth_max_error_count', $form_state['values']['max_error_count']);
        variable_set('flat_auth_max_usage_count', $form_state['values']['max_usage_count']);

        drupal_set_message(t('Flat Auth Admin configuration was successfully saved'));
    }
}
