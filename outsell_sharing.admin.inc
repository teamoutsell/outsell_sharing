<?php

/**
 * @file
 * Code for the admin part of outsell_sharing.
 */

/**
 * Drupal form for our sharing settings.
 */
function outsell_sharing_settings_form() {

  // Get the available content types we can share.
  $content_types = outsell_sharing_node_info();

  // Basic sharing config.
  $form['outsell_sharing_config'] = array(
    '#type' => 'fieldset',
    '#title' => t('Sharing configuration'),
    '#weight' => 0,
  );
  $form['outsell_sharing_config']['outsell_sharing_sender'] = array(
    '#type' => 'checkbox',
    '#title' => t('Allow this site to send content to other sites.'),
    '#description' => t('This will allow this site to send new content to other sites.'),
    '#default_value' => variable_get('outsell_sharing_sender', FALSE),
  );
  $form['outsell_sharing_config']['outsell_sharing_getter'] = array(
    '#type' => 'checkbox',
    '#title' => t('Allow this site to get content from other sites.'),
    '#description' => t('This will allow this site to receive new content from other sites.'),
    '#default_value' => variable_get('outsell_sharing_getter', FALSE),
  );

  // Options for sending.
  $form['outsell_sharing_sender_config'] = array(
    '#type' => 'fieldset',
    '#title' => t('Configure Sending'),
    '#states' => array(
      'visible' => array(
        ':input[name=\'outsell_sharing_sender\']' => array('checked' => TRUE),
      )
    )
  );
  $form['outsell_sharing_sender_config']['outsell_sharing_sites'] = array(
    '#type' => 'textarea',
    '#title' => t('Sites to send content and their api keys (one per line):'),
    '#cols' => 50,
    '#rows' => 5,
    '#default_value' => list_allowed_values_string(variable_get('outsell_sharing_sites', array())),
    '#description' => t('Enter sites here in the format url|apikey (one per line), e.g:
<br/>
<br/>https://editors.kbox.site|h39yfih9po3nop3u9fh3
<br/>https://outsellinc.com|09u3foin4fioh8hlkeshio
<br/>
<br/>NOTE: You must enable these sites to receive content. When you do you will get an API key to use above.
'),
  );

  // Loop through our sites and add options for each.
  foreach ($content_types as $type => $data) {
    $t_opts = array('!type' => '<strong><em>' . $type . '</em></strong>');
    $form['outsell_sharing_sender_config']['outsell_sharing_send_' . $type] = array(
      '#type' => 'checkbox',
      '#title' => t('Send !type.', $t_opts),
      '#description' => t('This will allow this site to receive !type content from other sites.', $t_opts),
      '#default_value' => variable_get('outsell_sharing_send_' . $type, FALSE),
    );
  }

  // Add our batch submit handler.
  $form['outsell_sharing_sender_config']['outsell_sharing_batch_description'] = array(
    '#type' => 'markup',
    '#markup' => t('<br><br>Click below to batch send content based on the above.<br>'),
  );
  $form['outsell_sharing_sender_config']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Batch send'),
    '#submit' => array('outsell_sharing_sender_batch_submit'),
    '#validate' => array('outsell_sharing_sender_batch_validate')
  );

  // Options for getting.
  $form['outsell_sharing_getter_config'] = array(
    '#type' => 'fieldset',
    '#title' => t('Configure Getting'),
    '#states' => array(
      'visible' => array(
        ':input[name=\'outsell_sharing_getter\']' => array('checked' => TRUE),
      )
    )
  );
  $form['outsell_sharing_getter_config']['outsell_sharing_apikey'] = array(
    '#type' => 'textfield',
    '#title' => t('API Key'),
    '#description' => t('Use this to send content from another site to this one.'),
    '#default_value' => variable_get('outsell_sharing_apikey', md5(drupal_random_bytes(128))),
    '#prefix' => '<div id="outsell-sharing-apikey-ajax">',
    '#suffix' => '</div>',
    '#size' => 60,
    '#maxlength' => 128
  );
  $form['outsell_sharing_getter_config']['outsell_sharing_apikey_refresh'] = array(
    '#type' => 'button',
    '#value' => t('Regenerate token'),
    '#ajax' => array(
      'callback' => 'outsell_sharing_regenerate_apikey_ajax',
      'wrapper' => 'outsell-sharing-apikey-ajax',
      'method' => 'replace',
      'effect' => 'fade'
    )
  );

  // Loop through our content types and add options for each type.
  foreach ($content_types as $type => $data) {
    $t_opts = array('!type' => '<strong><em>' . $type . '</em></strong>');
    $form['outsell_sharing_getter_config']['outsell_sharing_get_' . $type] = array(
      '#type' => 'checkbox',
      '#title' => t('Get !type.', $t_opts),
      '#description' => t('This will allow this site to receive !type content from other sites.', $t_opts),
      '#default_value' => variable_get('outsell_sharing_get_' . $type, FALSE),
    );
  }

  // Add another submit handler to handle unsetting things correctly.
  $form['#submit'][] = 'outsell_sharing_settings_form_submit';

  // Wrap and return.
  return system_settings_form($form);

}

/**
 * Helper function to regenerate the api key.
 */
function outsell_sharing_regenerate_apikey_ajax($form, $form_state) {
  $apikey = $form['outsell_sharing_getter_config']['outsell_sharing_apikey'];
  $apikey['#value'] = md5(drupal_random_bytes(128));
  return $apikey;
}

/**
 * Submit handler to unset any suboptions if needed.
 */
function outsell_sharing_settings_form_submit($form, &$form_state) {

  // Get the available content types we can share.
  $content_types = outsell_sharing_node_info();

  // Make sure we arrayify our list.
  $sites = $form_state['values']['outsell_sharing_sites'];
  $form_state['values']['outsell_sharing_sites'] = list_extract_allowed_values($sites, 'list_text', FALSE);

  // Unset getters if needed.
  if ($form_state['values']['outsell_sharing_getter'] === 0) {
    foreach ($content_types as $type => $data) {
      $form_state['values']['outsell_sharing_get_' . $type] = 0;
    }
  }

  // Unset senders if needed.
  if ($form_state['values']['outsell_sharing_sender'] === 0) {

    // Remove content senders.
    foreach ($content_types as $type => $data) {
      $form_state['values']['outsell_sharing_send_' . $type] = 0;
    }

    // Remove sites.
    $form_state['values']['outsell_sharing_sites'] = NULL;

  }

}

/**
 * Validate handler to check the batch.
 */
function outsell_sharing_sender_batch_validate($form, &$form_state) {

  // Get the available content types we can share.
  $allowed_content_types = outsell_sharing_node_info();

  // Collect errors.
  $content_type_responses = 0;

  // Collect responses.
  foreach ($allowed_content_types as $allowed_content_type => $data) {
    if ((bool) $form_state['values']['outsell_sharing_send_' . $allowed_content_type] === TRUE) {
      $content_type_responses++;
    }
  }

  // Throw errors if needed.
  if ($content_type_responses === 0) {
    form_set_error('Content Type', t('Need to select at least one type of content!'));
  }
  if (empty($form_state['values']['outsell_sharing_sites'])) {
    form_set_error('Site', t('Need to specify at least one site!'));
  }

}

/**
 * Submit handler to execute the batch.
 */
function outsell_sharing_sender_batch_submit($form, &$form_state) {

  // Get the available content types we can share.
  $allowed_content_types = outsell_sharing_node_info();

  // Get the sites we want to send to.
  $sites = list_extract_allowed_values($form_state['values']['outsell_sharing_sites'], 'list_text', FALSE);

  // Collectors for sites and content and node ids.
  $content_types = array();
  $nids = array();

  // What we sending?
  foreach ($allowed_content_types as $allowed_content_type => $data) {
    if ((bool) $form_state['values']['outsell_sharing_send_' . $allowed_content_type] === TRUE) {
      $content_types[] = $allowed_content_type;
    }
  }

  // Grab all our node ids.
  $sites_string = implode(', ', array_keys($sites));
  foreach ($content_types as $content_type) {

    // Get the nodes.
    $type_nids = db_query('SELECT nid FROM {node} WHERE type = :type LIMIT 10', array(':type' => $content_type))->fetchCol();
    $type_nids_count = count($type_nids);

    // Tell the user WTFIGO.
    $message = t(
      'Sending @num of %type to %sites',
      array(
        '@num' => $type_nids_count,
        '%type' => $content_type,
        '%sites' => $sites_string
      )
    );
    drupal_set_message($message);

    // Add to our nids.
    $nids += $type_nids;

  }

  // Collect some operations.
  $operations = array();

  // Iterate through all the ids add an op to each.
  foreach ($nids as $nid) {

    // Build the op args.
    $args = array($nid, $sites);

    // Add an operation.
    $operations[] = array('outsell_sharing_send_node', $args);

  }

  // Construct the batch.
  $batch = array(
    'operations' => $operations,
    'finished' => 'outsell_sharing_batch_finished',
    'file' => drupal_get_path('module', 'outsell_sharing') . '/outsell_sharing.send.inc'
  );

  // Set the batch.
  batch_set($batch);

}
