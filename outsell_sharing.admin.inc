<?php

/**
 * @file
 * Code for the admin part of outsell_sharing.
 */

/**
 * Return list of allowed sites to send to.
 */
function outsell_sharing_get_sites() {
  return array('community', 'editors', 'outsellinc');
}

/**
 * Drupal form for our sharing settings.
 */
function outsell_sharing_settings_form() {

  // Get the available content types we can share.
  $content_types = outsell_sharing_node_info();

  // Define the sites we can send to.
  $sites = outsell_sharing_get_sites();

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
  $form['outsell_sharing_sender_config']['description'] = array(
    '#type' => 'markup',
    '#markup' => t('Batch send the below kinds of content'),
  );

  // Loop through our content types and add options for each type.
  foreach ($content_types as $type => $data) {
    $t_opts = array('!type' => '<strong><em>' . $type . '</em></strong>');
    $form['outsell_sharing_sender_config']['outsell_sharing_send_' . $type] = array(
      '#type' => 'checkbox',
      '#title' => t('Send !type.', $t_opts),
      '#description' => t('This will allow this site to send !type content to other sites.', $t_opts)
    );
  }

  // Add other batch opts.
  $form['outsell_sharing_sender_config']['description2'] = array(
    '#type' => 'markup',
    '#markup' => t('To the following sites (remember these sites must allow receipt of content)'),
  );

  // Loop through our sites and add options for each.
  foreach ($sites as $site) {
    $t_opts = array('!site' => '<strong><em>' . $site . '</em></strong>');
    $form['outsell_sharing_sender_config']['outsell_sharing_send_' . $site] = array(
      '#type' => 'checkbox',
      '#title' => t('!site.', $t_opts),
      '#description' => t('This will allow this site to send !site content to other sites.', $t_opts)
    );
  }

  // Add our batch submit handler.
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
 * Submit handler to unset any suboptions if needed.
 */
function outsell_sharing_settings_form_submit($form, &$form_state) {

  // Get the available content types we can share.
  $content_types = outsell_sharing_node_info();

  // Unset getters if needed.
  if ($form_state['values']['outsell_sharing_getter'] === 0) {
    foreach ($content_types as $type => $data) {
      $form_state['values']['outsell_sharing_get_' . $type] = 0;
    }
  }

}

/**
 * Validate handler to check the batch.
 */
function outsell_sharing_sender_batch_validate($form, &$form_state) {

  // Get the available content types we can share.
  $allowed_content_types = outsell_sharing_node_info();

  // Define the sites we can send to.
  $allowed_sites = outsell_sharing_get_sites();

  // Collect errors.
  $content_type_responses = 0;
  $sites_responses = 0;

  // Collect responses.
  foreach ($allowed_content_types as $allowed_content_type => $data) {
    if ((bool) $form_state['values']['outsell_sharing_send_' . $allowed_content_type] === TRUE) {
      $content_type_responses++;
    }
  }
  foreach ($allowed_sites as $allowed_site) {
    if ((bool) $form_state['values']['outsell_sharing_send_' . $allowed_site] === TRUE) {
      $sites_responses++;
    }
  }

  // Throw errors if needed.
  if ($content_type_responses === 0) {
    form_set_error('Content Type', t('Need to select at least one type of content!'));
  }
  if ($sites_responses === 0) {
    form_set_error('Site', t('Need to select at least one site!'));
  }

}

/**
 * Submit handler to execute the batch.
 */
function outsell_sharing_sender_batch_submit($form, &$form_state) {

  // Get the available content types we can share.
  $allowed_content_types = outsell_sharing_node_info();

  // Define the sites we can send to.
  $allowed_sites = outsell_sharing_get_sites();

  // Collectors for sites and content and node ids.
  $sites = array();
  $content_types = array();
  $nids = array();

  // What we sending?
  foreach ($allowed_content_types as $allowed_content_type => $data) {
    if ((bool) $form_state['values']['outsell_sharing_send_' . $allowed_content_type] === TRUE) {
      $content_types[] = $allowed_content_type;
    }
  }

  // Where we sending it to?
  foreach ($allowed_sites as $allowed_site) {
    if ((bool) $form_state['values']['outsell_sharing_send_' . $allowed_site] === TRUE) {
      $sites[] = $allowed_site;
    }
  }

  // Grab all our node ids.
  $sites_string = implode(', ', $sites);
  foreach ($content_types as $content_type) {

    // Get the nodes.
    $type_nids = db_query('SELECT nid FROM {node} WHERE type =  :type LIMIT 10', array(':type' => $content_type))->fetchCol();
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
